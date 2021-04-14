# Raspberry Pi FDD
Make your Raspberry Pi an intelligent type FDD unit like the NEC PC-8031-2W/PC-80S31.
It supports 2D FDD images in d88 format. 2 drives.  
Note: The CPU and ROM on the FDD unit side are not supported.

## Hardware requirements
- Raspberry Pi 4 (It may work with other Raspberry Pi's, but I haven't tried it.)
- A cable that connects the FDD connector to GPIO pins on the Raspberry Pi.

## Pin assignments
```
FDD Port(5V)           Raspberry Pi(3.3V)
Pin 1- 8 (PB0-PB7) --> GPIO4-11 (*)Convert to 3.3V.
Pin19-26 (PA0-PA7) <-- GPIO12-19
Pin27-30 (PC4-7)   --> GPIO20-23 (*)Convert to 3.3V.
Pin31-33 (PC0-2)   <-- GPIO24-26
   Pin35 (RESET#)  --> GPIO27 (*)Convert to 3.3V.
   Pin36 (GND)     --- GND

In addition, connect pin 9 and pin 10 of the FDD connector. If you don't connect these pins, it won't be recognized as an FDD.
FDD Port         FDD Port
Pin9 (EXTON) --- Pin10 (GND)
```
**Note**: Since the voltage on the PC side is 5V and the voltage on the Raspberry Pi side is 3.3V, at least the above (\*) signals input to the Raspberry Pi side should be converted from 5V to 3.3V by level shifters or voltage divider with resistors.
To divide the voltage with resistors, connect as follows (12 pins in total).
```
Singal from PC -+
                |
              10kΩ
               |
               +-- To a GPIO pin of the Raspberry Pi
               |
              20kΩ
               |
              GND
```

## How to compile
```
$ git clone https://github.com/MinatsuT/RPi_PC-80S31.git
$ cd RPi_PC-80S31
$ make
```

## How to use
Prepare a system disk image file in d88 format and run with the following command.
You can specify up to two disk image files.
```
$ sudo ./pc80s31 system.d88
```

If you specify an empty (0 byte) image file, it will be inserted as an unformatted disk.
In this case, please format it from the connected PC.
```
$ touch blank.d88
$ sudo ./pc80s31 system.d88 blank.d88
```

## Disclaimer
Please note that I am not responsible for any damages incurred based on this information.