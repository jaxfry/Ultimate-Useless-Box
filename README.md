<a id="readme-top"></a>


[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![project_license][license-shield]][license-url]



<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/jaxfry/Ultimate-Useless-Box">
    <img src="assets/images/renders/Useless_Box_Master_Assembly_2026-Apr-23_04-58-31AM-000_CustomizedView32119260769_png.png" alt="Logo" width="600">
  </a>

<h3 align="center">Ultimate Useless Box</h3>

  <p align="center">
    The Ultimate Useless Box, a box that does nothing, with style.
    <br />
    <br />
    <a href="#gallery--renders">View Renders</a>
    &middot;
    <a href="#custom-pcb">View PCB</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#gallery--renders">Gallery & Renders</a>
    </li>
    <li>
      <a href="#custom-pcb">Custom PCB</a>
    </li>
    <li><a href="#bill-of-materials">Bill of Materials</a></li>
    <li><a href="#license">License</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

While most useless boxes just flip a switch, this one has personality. Powered by an **ESP32-S3**, this box uses a Time-of-Flight sensor to detect your hand before you even push the swtich, allowing the box to react by driving away, with the OLED screen, or countless other ways!
This project is heavily inspired by LuuMa EV3.
When I saw that video many years ago, I realized I wanted my own useless box. Then I realized that Lego Mindstorm kits are prohibitively expensive. Many years later and I decided to make my own using standard electronic components with a few unique twists of my own :)

**Key Features:**
* **OLED Display** for animated faces.
* **Fun Built in Power Management:** Mainly for learning and fun but we charge over usb-C!
* **Wheels** for driving away.
* **TOF** sensor for detecting hands.
* **A Retractable Switch**
<p align="right">(<a href="#readme-top">back to top</a>)</p>



### Built With

*   ![ESP32](https://img.shields.io/badge/Hardware-ESP32--S3-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
*   ![C++](https://img.shields.io/badge/Firmware-C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
*   ![KiCad](https://img.shields.io/badge/PCB-KiCad-31409E?style=for-the-badge&logo=kicad&logoColor=white)
*   ![Fusion 360](https://img.shields.io/badge/CAD-Fusion_360-EA4335?style=for-the-badge&logo=autodesk&logoColor=white)

<p align="right">(<a href="#readme-top">back to top</a>)</p>


### Custom PCB
<div align="center">
  <img src="assets/images/photos/pcb.png" width="45%" alt="PCB Front" />
  <img src="assets/images/photos/pcb3d.png" width="45%" alt="PCB 3D View" />
    <img src="assets/images/photos/schematic.png" width="45%" alt="Schematic Diagram" />
</div>

<!-- GALLERY -->
<h2 id="gallery--renders">Gallery & Renders</h2>

<div align="center">
  <img src="assets/images/photos/cutSide.png" width="45%" alt="Cut Side Photo" />
  <img src="assets/images/renders/Useless_Box_Master_Assembly_2026-Apr-23_04-14-33AM-000_CustomizedView27976062760_png.png" width="45%" alt="Render View 1" />
  <img src="assets/images/renders/Useless_Box_Master_Assembly_2026-Apr-23_04-11-53AM-000_CustomizedView12176126467_mp4.gif" width="45%" alt="Animated Render 1" />
  <img src="assets/images/renders/Useless_Box_Master_Assembly_2026-Apr-23_05-17-29AM-000_CustomizedView3936447915_png.png" width="45%" alt="Render View 3" />
  <img src="assets/images/renders/Useless_Box_Master_Assembly_2026-Apr-23_05-10-10AM-000_CustomizedView32119260769_mp4.gif" width="45%" alt="Animated Render 2" />
</div>

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Project Goals
- Build a reliable useless box with some personality
- Use accessible parts and open files
- Learn more about PCB design



## How to build
It should be pretty easy, the PCB has labeled connectors for what you need to plug in where. Basically, plug the servos into the headers labeled servos, the motors into the ports labeled motor right and motor left, the TOF sensor into TOF and switch into Switch, and you should be good to go.



<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[stars-shield]: https://img.shields.io/github/stars/jaxfry/Ultimate-Useless-Box.svg?style=for-the-badge
[stars-url]: https://github.com/jaxfry/Ultimate-Useless-Box/stargazers
[issues-shield]: https://img.shields.io/github/issues/jaxfry/Ultimate-Useless-Box.svg?style=for-the-badge
[issues-url]: https://github.com/jaxfry/Ultimate-Useless-Box/issues
[license-shield]: https://img.shields.io/github/license/jaxfry/Ultimate-Useless-Box.svg?style=for-the-badge
[license-url]: https://github.com/jaxfry/Ultimate-Useless-Box/blob/main/LICENSE.txt


