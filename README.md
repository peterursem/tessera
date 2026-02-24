![Banner](https://github.com/user-attachments/assets/d576c8ea-f542-4519-b9b9-4da561f456d3)


# 🧱 Tessera

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![macOS](https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=macos&logoColor=F0F0F0)

> *Tessera has been tested on MacOS, support for other platforms is currently unknown*

## 📋 Overview

### A Single Pixel Camera
Tessera is a hardware and software implementation of compressed sensing with a single photodetector.

### Hardware
The current implementation of the sensing hardware is an apparatus consisting of: 
- A 50mm NIKKOR lens
- An object placed at the NIKKOR lens's focal point
- A 50mm lens directly behind the object
- A 10mm lens direclty behind the 50mm lens
- A photoresistor placed so the aperture of the NIKKOR lens is sharp.
- An arduino microcontroller for analog measurements over serial
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
![IMG_3078-1](https://github.com/user-attachments/assets/12293841-1156-4e92-b28f-fc2a6edb173a)

The elements of the optical train are held together with 3D printed mounts that make it easy to focus the NIKKOR lens and photoresistor. The photoresistor is monitored by an arduino that passes serial data to the main computer.

### Software
The software is composed of two elements:
* **Patterning:**
    * Generating Hadamard patterns
    * Ordering patterns for Walsh sequency
    * Display patterns
 
* **Reconstruction:**
    * Get the weight of each pattern and sum them to obtain a result
    * The weight of each pattern is determined by the light intensity measured by the sensor.
    * A simplified final expression is $`x = \sum H_ny_n `$ where x is the result, $`H_n`$ is the Hadamard pattern being sampled and $`y_n`$ is the light intensity measured.


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

|Tulip (64x64)|Arrow (32x32)|
|--|--|
|![Tulip64_100](https://github.com/user-attachments/assets/59b304f5-117c-4c9a-a9ab-b6be07fe3c9f)|![Arrow32_100 copy](https://github.com/user-attachments/assets/f16cb3f9-c181-4c5c-94de-c11f4260e196)|



## 🧪 Experimental Setup
     
In later trials I covered the apparatus with more cloths to block ambient light, though Tessera should eliminate ambient light through averaging.

![IMG_3086](https://github.com/user-attachments/assets/af2d7e79-7e46-410f-8866-d4186a5c3f45)
![IMG_3072 (1)](https://github.com/user-attachments/assets/9176eca6-94ca-48cf-b739-4f50d4c12b21)
