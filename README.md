```
MMP""MM""YMM
P'   MM   `7",
     MM  .gP"Ya  ,pP"Ybd ,pP"Ybd  .gP"Ya `7Mb,od8 ,6"Yb.
     MM ,M'   Yb 8I   `" 8I   `" ,M'   Yb  MM' "'8)   MM
     MM 8M"""""" `YMMMa. `YMMMa. 8M""""""  MM     ,pm9MM
     MM YM.    , L.   I8 L.   I8 YM.    ,  MM    8M   MM
   .JMML.`Mbmmd' M9mmmP' M9mmmP'  `Mbmmd'.JMML.  `Moo9^Yo
    
                                                     v0.1
```

# 🧱 Tessera

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)![macOS](https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=macos&logoColor=F0F0F0)

> *Tessera has been tested on MacOS, support for other platforms is currently unknown*

## 🌟 Highlights

- Customizable
- Modular
- Lightweight

## 📋 Overview

Tessera is a single pixel camera (SPC) control software. Connect any sensor to your machine using serial and use a monitor, projector, or coded aperture to illuminate it. Tessera handles patterning and reconstruction accellerated by OpenCL.

## 📸 Usage

```
$ > ./tessera

  Welcome to                                                   \__\  \__\  \__\
                                                             \__\  \__\  \__\  \
  MMP""MM""YMM                                               _\  \__\  \__\  \__
  P'   MM   `7                                                 \__\  \__\  \__\
       MM  .gP"Ya  ,pP"Ybd ,pP"Ybd  .gP"Ya `7Mb,od8 ,6"Yb.   \__\  \__\  \__\  \
       MM ,M'   Yb 8I   `" 8I   `" ,M'   Yb  MM' "'8)   MM   _\  \__\  \__\  \__
       MM 8M"""""" `YMMMa. `YMMMa. 8M""""""  MM     ,pm9MM     \__\  \__\  \__\
       MM YM.    , L.   I8 L.   I8 YM.    ,  MM    8M   MM   \__\  \__\  \__\  \
     .JMML.`Mbmmd' M9mmmP' M9mmmP'  `Mbmmd'.JMML.  `Moo9^Yo  _\  \__\  \__\  \__
                                                               \__\  \__\  \__\
                                                       v0.1  \__\  \__\  \__\  \
                                                             _\  \__\  \__\  \__
     Resolution:     [             64 x 64            ]        \__\  \__\  \__\
                                                             \__\  \__\  \__\  \
     Sample Rate:    [                4               ]      _\  \__\  \__\  \__
                                                               \__\  \__\  \__\
     Sensor Port:    [        cu.debug-console        ]      \__\  \__\  \__\  \
                                                             _\  \__\  \__\  \__
                     [              Start             ]        \__\  \__\  \__\
                                                             \__\  \__\  \__\  \
                                                             _\  \__\  \__\  \__
                                                               \__\  \__\  \__\
                                                             \__\  \__\  \__\  \
                                                             _\  \__\  \__\  \__
```
1. Navigate using the arrow and enter keys.\
2. Configure the settings to match your setup requirements:
   - Resolution: Enter the desired output resolution. Ensure the resolution is smaller than that of the coded aperture
   - Sample Rate: Ensure this is equal to or slower than the sensor's maximum sample rate.
   - Sensor Port: Select the sensor's serial connection port.
3. Finally, press start and sampling will begin. A periodic preview will be saved to ./out/preview.pgm

## 🖼️ Results
> All example captures were made using a generic photoresistor and an aperture value of f/11 on the front NIKKOR lens.

![Tulip64_100_EDIT](https://github.com/user-attachments/assets/0870eacd-9758-4750-9b4b-4807e1917d00)
![result](https://github.com/user-attachments/assets/96460ad1-76ba-426c-a5a1-62021b672367)

## 🧪 Experimental Setup

To conduct tests on real hardware, I used an apparatus consisting of: 
- A 50mm NIKKOR lens
- An object placed at the NIKKOR lens's focal point
- A 50mm lens directly behind the object
- A 10mm lens direclty behind the 50mm lens
- A photoresistor placed so the aperture of the NIKKOR lens is sharp.
- An arduino microcontroller for analog measurements over serial
     
![IMG_3078](https://github.com/user-attachments/assets/c4435195-0d12-4fed-bef9-6f72ceceeaf3)

In later trials I covered the apparatus with more cloths to block ambient light, though Tessera should eliminate ambient light through averaging.

![IMG_3086](https://github.com/user-attachments/assets/af2d7e79-7e46-410f-8866-d4186a5c3f45)
![IMG_3072 (1)](https://github.com/user-attachments/assets/9176eca6-94ca-48cf-b739-4f50d4c12b21)
