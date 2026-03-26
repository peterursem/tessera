![Banner](https://github.com/user-attachments/assets/d576c8ea-f542-4519-b9b9-4da561f456d3)


# 🧱 Tessera
A single pixel camera using Hadamard basis compressed sensing.

**Languages and Frameworks**\
[![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)](#) [![OpenGl](https://img.shields.io/badge/OpenGl-5487A6?style=for-the-badge&logo=OpenGl&logoColor=fff)](#) [![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)](#)\
**OS Support**\
[![macOS](https://img.shields.io/badge/macOS-000000?style=for-the-badge&logo=apple&logoColor=F0F0F0)](#) [![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)](#)

**My website**\
[![Static Badge](https://img.shields.io/badge/Peter_Ursem-peter?style=for-the-badge&color=%2366CDAA)](https://ursem.ca/)


## 📋 Overview

### Hardware:
The apparatus used to capture test images is a very low-cost visible light detection setup.\
It consists of:

        **1. Light:** An LCD monitor projects Hadamard patterns. The objective lens focuses the light patterns onto the object.\
        **2. Object:** The object masks light from the pattern, blocking some from the detector.\
        **3. Collection:** The field lens and relay lens condense the light and focus the objective lens’ aperture onto the detector.\
        **4. Detection:** The photoresistor converts the resulting light intensity into a measurable voltage to be read by an Arduino.

Samples were taken of all Hadamard patterns to allow for easy reordering in post.

```mermaid
flowchart LR;

    %% --- Physical Optical Subsystem ---
    subgraph Optical Path;
        direction LR;
        DS[LCD Monitor];
        DS -->L1[50mm 
        NIKKOR Lens];
        L1 -->|Reduces and 
        Focuses Pattern| O(((Object)));
        O --> L2[50mm
        Convex Lens];
        L2 -->L3[10mm 
        Convex Lens];
        L3 -->|Focuses NIKKOR
        Aperture| PR((Photoresistor));
    end

    %% --- Connecting the sensor ---
    PR -->|Voltage Probe| MCU[Arduino 
    Microcontroller];

    %% --- Styling ---
    style DS fill:#555,stroke:#fff,stroke-width:1px,color:#fff,stroke-dasharray: 5 5;
    style MCU fill:#00979D,stroke:#fff,stroke-width:2px,color:#fff;

    style L1 fill:#00599C,stroke:#fff,stroke-width:2px,color:#fff;
    style L2 fill:#00599C,stroke:#fff,stroke-width:2px,color:#fff;
    style L3 fill:#00599C,stroke:#fff,stroke-width:2px,color:#fff;
    style O fill:#222,stroke:#fff,stroke-width:2px,color:#fff;
    style PR fill:#eebb00,stroke:#333,stroke-width:2px,color:#000;

    %% Thick arrow for light entering the system
    linkStyle 0,1,2,3,4 stroke-width:3px,fill:none,stroke:yellow;
```

![IMG_3171](https://github.com/user-attachments/assets/5befb335-5526-4eb6-82e5-1918508d3675)

The elements of the optical train are held together with 3D printed mounts that make it easy to focus the NIKKOR lens and photoresistor.

### Software:
The software is comprised of three elements:

* **Patterning:**
    * Generate Hadamard patterns
    * Order patterns by sequency (# of sign changes)
    * Display patterns
 
* **Sampling:**
    * Capture and store brightness values from the photoresistor
 
* **Reconstruction:**
    * The final image is reconstructed using a fast Walsh-Hadamard transform


## 📸 Usage
|Main Menu|Scan Progress|
|--|--|
|![Menu](https://github.com/user-attachments/assets/6a5f3e5c-1c60-4f06-91c6-301bfd218ce6)|![Progress](https://github.com/user-attachments/assets/ce2d61e0-8d04-4305-90f9-cac722aaf002)|

1. Navigate using the arrow and enter keys.
2. Configure the settings to match your setup requirements:
   - Resolution: Enter the desired output resolution. Ensure the resolution is smaller than that of the coded aperture
   - Sample Rate: Ensure this is equal to or slower than the sensor's maximum sample rate.
   - Sensor Port: Select the sensor's serial connection port.

3. Finally, press start and sampling will begin. A periodic preview will be saved to ./out/preview.pgm

## 🖼️ Results
> All example captures were made using a generic photoresistor and an aperture value of f/11 on the front NIKKOR lens.

|Mountain (64px)|Balloon (256px)|Tulip (64px)|Arrow (32px)|
|--|--|--|--|
|<img src="https://github.com/user-attachments/assets/bc0d448d-4200-4989-bf53-8001df84f01a" height=64>|<img src="https://github.com/user-attachments/assets/fc385244-cbad-496b-9b6e-b29156d46635" height=64>|<img src="https://github.com/user-attachments/assets/59b304f5-117c-4c9a-a9ab-b6be07fe3c9f" height=64>|<img src="https://github.com/user-attachments/assets/f16cb3f9-c181-4c5c-94de-c11f4260e196" height=64>|


## 🧪 Experimental Setup

![IMG_3230](https://github.com/user-attachments/assets/bcc44b71-9850-4c77-a3cb-0c7451eb0805)

## 👨‍🎓 Research Poster
### [Download](https://github.com/user-attachments/files/26288451/UrsemP_Poster_ResearchDay26.pdf)
![UrsemP_Poster_ResearchDay26](https://github.com/user-attachments/assets/be81ce91-490e-427e-b016-6bb2b67da17d)
