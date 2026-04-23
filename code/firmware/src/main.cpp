#include <Arduino.h>
#include <Wire.h>
#include <atomic>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_VL53L0X.h>
#include <ESP32Servo.h>

#include "pins.h"
#include "sounds.h"

namespace {
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;

constexpr TickType_t INPUT_POLL_INTERVAL = pdMS_TO_TICKS(10);
constexpr uint32_t TOF_POLL_PERIOD_MS = 60;
constexpr uint32_t VBATT_POLL_PERIOD_MS = 1000;
constexpr TickType_t BEHAVIOR_LOOP_INTERVAL = pdMS_TO_TICKS(10);
constexpr TickType_t DISPLAY_FRAME_INTERVAL = pdMS_TO_TICKS(33);
constexpr int16_t HAND_DETECTED_MM = 180;
constexpr uint32_t HAND_ACTION_COOLDOWN_MS = 1500;
constexpr uint8_t HAND_REACTION_CHANCE_PERCENT = 35;
constexpr uint8_t SWITCH_HIDE_SERVO_HOME_DEG = 90;
constexpr uint8_t SWITCH_HIDE_SERVO_PULL_IN_DEG = 20;  // Try 160 if your linkage is reversed.
constexpr float VBATT_ADC_DIVIDER_RATIO = 2.0f;  // Adjust if your divider uses a different ratio.
constexpr float VBATT_LOW_THRESHOLD_V = 3.55f;
constexpr float VBATT_CRITICAL_THRESHOLD_V = 3.40f;

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_VL53L0X tofSensor;
Servo mainServo;
Servo switchHideServo;

TaskHandle_t sensorTaskHandle = nullptr;
TaskHandle_t behaviorTaskHandle = nullptr;
SemaphoreHandle_t i2cMutex = nullptr;

enum BoxEmotion {
  EMOTION_SLEEPY,
  EMOTION_IDLE,
  EMOTION_ANGRY,
  EMOTION_SURPRISED
};

std::atomic<BoxEmotion> currentEmotion{EMOTION_SLEEPY};
TaskHandle_t displayTaskHandle = nullptr;

// Shared sensor snapshot for future behavior/state tasks.
// Using std::atomic for thread-safe access from multiple cores.
std::atomic<bool> gMainSwitchActive{false};
std::atomic<int16_t> gHandDistanceMm{-1};
std::atomic<bool> gTofSensorPresent{false};
std::atomic<float> gBatteryVoltageV{-1.0f};
std::atomic<bool> gBatteryLow{false};
std::atomic<bool> gBatteryCritical{false};
std::atomic<uint32_t> gEmotionHoldUntilMs{0};
std::atomic<bool> gHasCompletedAnyAction{false};
}  // namespace

void SensorTask(void* pvParameters) {
  (void)pvParameters;

  uint32_t lastTofReadMs = 0;
  uint32_t lastBatteryReadMs = 0;

  for (;;) {
    gMainSwitchActive.store(digitalRead(PIN_MAIN_SWITCH) == LOW);

    uint32_t nowMs = millis();
    if (!gTofSensorPresent.load()) {
      gHandDistanceMm.store(-1);
    } else if (nowMs - lastTofReadMs >= TOF_POLL_PERIOD_MS) {
      VL53L0X_RangingMeasurementData_t measure;
      // Lock I2C mutex to prevent contention with DisplayTask
      if (xSemaphoreTake(i2cMutex, portMAX_DELAY)) {
        tofSensor.rangingTest(&measure, false);
        xSemaphoreGive(i2cMutex);
      }
      gHandDistanceMm.store((measure.RangeStatus != 4) ? measure.RangeMilliMeter : -1);
      lastTofReadMs = nowMs;
    }

    if (nowMs - lastBatteryReadMs >= VBATT_POLL_PERIOD_MS) {
      uint32_t batteryMv = analogReadMilliVolts(PIN_VBATT_SENSE);
      float batteryV = (batteryMv / 1000.0f) * VBATT_ADC_DIVIDER_RATIO;
      gBatteryVoltageV.store(batteryV);
      gBatteryLow.store(batteryV > 0.0f && batteryV <= VBATT_LOW_THRESHOLD_V);
      gBatteryCritical.store(batteryV > 0.0f && batteryV <= VBATT_CRITICAL_THRESHOLD_V);
      lastBatteryReadMs = nowMs;
    }

    vTaskDelay(INPUT_POLL_INTERVAL);
  }
}

// Actions

bool emotionHoldActive() {
  return static_cast<int32_t>(gEmotionHoldUntilMs.load() - millis()) > 0;
}

void holdEmotion(BoxEmotion emotion, uint32_t durationMs) {
  currentEmotion.store(emotion);
  gEmotionHoldUntilMs.store(millis() + durationMs);
}

void actionStandard() {
  // Standard push and return

  holdEmotion(EMOTION_ANGRY, 900);
  soundChirp();

  // Flick switch
  mainServo.write(20);
  vTaskDelay(pdMS_TO_TICKS(320));

  // Home
  mainServo.write(90);
  soundHappy();
  vTaskDelay(pdMS_TO_TICKS(180));
}

void actionHesitant() {
  // Comes out a little bit, goes back, then flicks it.

  holdEmotion(EMOTION_SURPRISED, 1200);
  soundSurprise();
  mainServo.write(65);
  vTaskDelay(pdMS_TO_TICKS(380));

  mainServo.write(90);
  vTaskDelay(pdMS_TO_TICKS(650));

  holdEmotion(EMOTION_ANGRY, 1000);
  soundSass();
  mainServo.write(20);
  vTaskDelay(pdMS_TO_TICKS(320));

  mainServo.write(90);
  soundHappy();
  vTaskDelay(pdMS_TO_TICKS(180));
}

void motorsStop() {
  analogWrite(PIN_MOT_AIN1, 0);
  analogWrite(PIN_MOT_AIN2, 0);
  analogWrite(PIN_MOT_BIN1, 0);
  analogWrite(PIN_MOT_BIN2, 0);
}

void motorsDrive() {
  analogWrite(PIN_MOT_AIN1, 200);
  analogWrite(PIN_MOT_AIN2, 0);
  analogWrite(PIN_MOT_BIN1, 200);
  analogWrite(PIN_MOT_BIN2, 0);
}

void actionDriveAway() {
  holdEmotion(EMOTION_ANGRY, 1000);
  soundGrump();
  motorsDrive();
  vTaskDelay(pdMS_TO_TICKS(400));
  motorsStop();
  vTaskDelay(pdMS_TO_TICKS(200));
}

void actionHideSwitchInside() {
  holdEmotion(EMOTION_SURPRISED, 1100);
  soundWhistle();

  switchHideServo.write(SWITCH_HIDE_SERVO_PULL_IN_DEG);
  vTaskDelay(pdMS_TO_TICKS(550));

  switchHideServo.write(SWITCH_HIDE_SERVO_HOME_DEG);
  soundHappy();
  vTaskDelay(pdMS_TO_TICKS(220));
}

void runRandomAction(bool allowDriveAway) {
  if (allowDriveAway) {
    // Weighted chance: 10% drive away, 45% standard, 45% hesitant.
    int slot = random(0, 10);
    if (slot == 0) {
      actionDriveAway();
    } else if (slot <= 4) {
      actionStandard();
    } else {
      actionHesitant();
    }
  } else {
    // First action after boot: never drive away.
    if (random(0, 2) == 0) {
      actionStandard();
    } else {
      actionHesitant();
    }
  }

  gHasCompletedAnyAction.store(true);
}

void runRandomTofAction() {
  // TOF-specific pool: drive away or pull the switch mechanism inside the box.
  if (!gBatteryLow.load() && random(0, 2) == 0) {
    actionDriveAway();
  } else {
    actionHideSwitchInside();
  }

  gHasCompletedAnyAction.store(true);
}

void BehaviorTask(void* pvParameters) {
  (void)pvParameters;

  bool lastSwitchState = false;
  bool lastHandNear = false;
  uint32_t nextHandActionAllowedMs = 0;

  for (;;) {
    bool currentSwitchState = gMainSwitchActive.load();

    bool handNear = (gHandDistanceMm.load() > 0 && gHandDistanceMm.load() < HAND_DETECTED_MM);
    uint32_t nowMs = millis();
    bool batteryCritical = gBatteryCritical.load();

    if (currentSwitchState == true && lastSwitchState == false) {
      Serial.println(batteryCritical ? "Switch flicked, battery is critical" :
                                       "OMG someone flicked the switch!");
      runRandomAction(gHasCompletedAnyAction.load() && !batteryCritical);
    }

    // Hand-detected trigger: rising edge + cooldown + chance.
    else if (handNear == true && lastHandNear == false &&
             nowMs >= nextHandActionAllowedMs) {
      nextHandActionAllowedMs = nowMs + HAND_ACTION_COOLDOWN_MS;

      if (batteryCritical) {
        Serial.println("Hand detected: skipping action because battery is critical");
      } else if (random(0, 100) < HAND_REACTION_CHANCE_PERCENT) {
        if (gHasCompletedAnyAction.load()) {
          Serial.println("Hand detected: TOF action triggered");
          runRandomTofAction();
        } else {
          Serial.println("Hand detected: TOF actions locked until first action");
        }
      } else {
        Serial.println("Hand detected: ignoring this time");
      }
    }

    lastSwitchState = currentSwitchState;
    lastHandNear = handNear;

    if (!emotionHoldActive()) {
      if (handNear) {
        currentEmotion.store(EMOTION_IDLE);
      } else {
        currentEmotion.store(EMOTION_SLEEPY);
      }
    }

    vTaskDelay(BEHAVIOR_LOOP_INTERVAL);
  }
}

void drawSleepyFace(uint32_t nowMs) {
  const int leftX = 40;
  const int rightX = 88;
  const int eyeY = 30;

  display.drawLine(leftX - 11, eyeY, leftX + 11, eyeY, SSD1306_WHITE);
  display.drawLine(leftX - 11, eyeY + 1, leftX + 11, eyeY + 1, SSD1306_WHITE);
  display.drawLine(rightX - 11, eyeY, rightX + 11, eyeY, SSD1306_WHITE);
  display.drawLine(rightX - 11, eyeY + 1, rightX + 11, eyeY + 1, SSD1306_WHITE);

  uint8_t zPhase = (nowMs / 500) % 6;
  if (zPhase < 5) {
    display.setTextSize(1);
    display.setCursor(8 + (zPhase * 8), 10 - (zPhase / 2));
    display.print("z");
  }
}

void drawIdleFace(uint32_t nowMs) {
  const int leftX = 40;
  const int rightX = 88;
  const int eyeY = 30;
  const int eyeW = 22;

  int eyeHalfH = 8;
  uint32_t cycle = nowMs % 3200;
  if (cycle >= 2800 && cycle < 2910) {
    eyeHalfH = map(cycle, 2800, 2910, 8, 1);
  } else if (cycle >= 2910 && cycle < 3020) {
    eyeHalfH = map(cycle, 2910, 3020, 1, 8);
  }

  display.fillRoundRect(leftX - (eyeW / 2), eyeY - eyeHalfH, eyeW, eyeHalfH * 2, 4,
                        SSD1306_WHITE);
  display.fillRoundRect(rightX - (eyeW / 2), eyeY - eyeHalfH, eyeW, eyeHalfH * 2, 4,
                        SSD1306_WHITE);

  if (eyeHalfH > 3) {
    int lookOffset = (static_cast<int>((nowMs / 250) % 5) - 2) * 2;
    display.fillCircle(leftX + lookOffset, eyeY, 2, SSD1306_BLACK);
    display.fillCircle(rightX + lookOffset, eyeY, 2, SSD1306_BLACK);
  }
}

void drawAngryFace() {
  const int leftX = 40;
  const int rightX = 88;
  const int eyeY = 31;

  display.fillRoundRect(leftX - 12, eyeY - 7, 24, 14, 4, SSD1306_WHITE);
  display.fillRoundRect(rightX - 12, eyeY - 7, 24, 14, 4, SSD1306_WHITE);

  // Carve the top edge to create a stronger, cleaner angry slant.
  display.fillTriangle(leftX - 12, eyeY - 7, leftX + 12, eyeY - 7, leftX + 12, eyeY - 1,
                       SSD1306_BLACK);
  display.fillTriangle(rightX - 12, eyeY - 7, rightX + 12, eyeY - 7, rightX - 12,
                       eyeY - 1, SSD1306_BLACK);

  display.drawLine(leftX - 14, eyeY - 13, leftX + 12, eyeY - 7, SSD1306_WHITE);
  display.drawLine(leftX - 14, eyeY - 12, leftX + 12, eyeY - 6, SSD1306_WHITE);
  display.drawLine(rightX + 14, eyeY - 13, rightX - 12, eyeY - 7, SSD1306_WHITE);
  display.drawLine(rightX + 14, eyeY - 12, rightX - 12, eyeY - 6, SSD1306_WHITE);
}

void drawSurprisedFace(uint32_t nowMs) {
  const int leftX = 40;
  const int rightX = 88;
  const int eyeY = 29;

  int radius = 8 + ((nowMs / 240) % 2);
  display.drawCircle(leftX, eyeY, radius, SSD1306_WHITE);
  display.drawCircle(rightX, eyeY, radius, SSD1306_WHITE);
  display.fillCircle(leftX, eyeY, 2, SSD1306_WHITE);
  display.fillCircle(rightX, eyeY, 2, SSD1306_WHITE);

  int mouthR = 5 + ((nowMs / 180) % 2);
  display.drawCircle(64, 49, mouthR, SSD1306_WHITE);
}

void DisplayTask(void* pvParamters) {
  (void)pvParamters;

  for (;;) {
    uint32_t nowMs = millis();
    display.clearDisplay();

    switch (currentEmotion.load()) {
      case EMOTION_SLEEPY:
        drawSleepyFace(nowMs);
        break;
      case EMOTION_IDLE:
        drawIdleFace(nowMs);
        break;
      case EMOTION_ANGRY:
        drawAngryFace();
        break;
      case EMOTION_SURPRISED:
        drawSurprisedFace(nowMs);
        break;
    }

    // Lock I2C mutex before sending data to OLED
    if (xSemaphoreTake(i2cMutex, portMAX_DELAY)) {
      display.display();
      xSemaphoreGive(i2cMutex);
    }
    vTaskDelay(DISPLAY_FRAME_INTERVAL);
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(micros());

  pinMode(PIN_MAIN_SWITCH, INPUT_PULLUP);
  pinMode(PIN_VBATT_SENSE, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  analogSetPinAttenuation(PIN_VBATT_SENSE, ADC_11db);

  pinMode(PIN_MOT_NSLEEP, OUTPUT);
  digitalWrite(PIN_MOT_NSLEEP, HIGH);

  analogWrite(PIN_MOT_AIN1, 0);
  analogWrite(PIN_MOT_AIN2, 0);
  analogWrite(PIN_MOT_BIN1, 0);
  analogWrite(PIN_MOT_BIN2, 0);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  mainServo.setPeriodHertz(50);
  mainServo.attach(PIN_SERVO_1, 500, 2400);
  mainServo.write(90);

  switchHideServo.setPeriodHertz(50);
  switchHideServo.attach(PIN_SERVO_2, 500, 2400);
  switchHideServo.write(SWITCH_HIDE_SERVO_HOME_DEG);

  delay(250);

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  pinMode(PIN_TOF_SHUT, OUTPUT);
  digitalWrite(PIN_TOF_SHUT, HIGH);
  delay(10);

  if (!tofSensor.begin()) {
    Serial.println(F("VL53L0X init failed"));
    gTofSensorPresent.store(false);
  } else {
    gTofSensorPresent.store(true);
    // Note: Adafruit_VL53L0X doesn't expose setTimeout(), but I2C mutex protects against bus hangs
  }

  bool displayPresent = false;
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 init failed"));
  } else {
    displayPresent = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("Useless Box Booting..."));
    display.display();
    soundHappy();
  }

  Serial.println(F("Battery monitor enabled on VBATT sense"));

  // Create I2C mutex for thread-safe Wire access
  i2cMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
      SensorTask, "Sensors", 4096, nullptr, 2, &sensorTaskHandle, 0);
  xTaskCreatePinnedToCore(
    BehaviorTask, "Behavior", 4096, nullptr, 2, &behaviorTaskHandle, 1);
  if (displayPresent) {
    xTaskCreatePinnedToCore(
      DisplayTask, "Display", 4096, nullptr, 2, &displayTaskHandle, 1);
  }
}

void loop() {
  vTaskDelete(nullptr);
}
