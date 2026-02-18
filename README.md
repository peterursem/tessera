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

# Tessera

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)![macOS](https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=macos&logoColor=F0F0F0)

> *Tessera has been tested on MacOS, support for other platforms is currently unknown*

## 🌟 Highlights

- Customizable
- Modular
- Lightweight


## ℹ️ Overview

Tessera is a single pixel camera (SPC) control software. Connect any sensor to your machine using serial and use a monitor, projector, or coded aperture to illuminate it. Tessera handles patterning and reconstruction accellerated by OpenCL.

## 🚀 Usage

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

Navigate using the arrow and enter keys.

Configure the settings to match your setup requirements:
- Resolution: Enter the desired output resolution. Ensure the resolution is smaller than that of the coded aperture
- Sample Rate: Ensure this is equal to or slower than the sensor's maximum sample rate.
- Sensor Port: Select the sensor's serial connection port.


Finally, press start and sampling will begin. A periodic preview will be saved to ./out/preview.pgm

![Tulip64_100_EDIT](https://github.com/user-attachments/assets/0870eacd-9758-4750-9b4b-4807e1917d00)

