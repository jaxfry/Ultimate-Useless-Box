#pragma once

#include <Arduino.h>
#include "pins.h"

// Frequencies
#define NOTE_G3  196
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_D6  1175
#define NOTE_E6  1319
#define NOTE_G6  1568

inline void soundChirp() {
    tone(PIN_BUZZER, NOTE_E6, 50);
    delay(60);
    tone(PIN_BUZZER, NOTE_G6, 50);
}

inline void soundHappy() {
    tone(PIN_BUZZER, NOTE_E5, 100);
    delay(110);
    tone(PIN_BUZZER, NOTE_G5, 100);
    delay(110);
    tone(PIN_BUZZER, NOTE_C6, 200);
}

inline void soundGrump() {
    tone(PIN_BUZZER, NOTE_C4, 200);
    delay(220);
    tone(PIN_BUZZER, NOTE_G3, 300);
}

inline void soundSurprise() {
    tone(PIN_BUZZER, NOTE_C6, 50);
    delay(60);
    tone(PIN_BUZZER, NOTE_C6, 50);
    delay(60);
}

inline void soundSass() {
    for (int i = 0; i < 3; i++) {
        tone(PIN_BUZZER, NOTE_G5, 50);
        delay(70);
        tone(PIN_BUZZER, NOTE_F5, 50);
        delay(70);
    }
}

inline void soundWhistle() {
    for (int freq = NOTE_C5; freq < NOTE_C6; freq += 20) {
        tone(PIN_BUZZER, freq, 10);
        delay(10);
    }
}
