# USBemani

USBemani is a firmware for the ATmega32u4 microcontroller intended to replace the original control board for many of Konami's home rhythm game controllers. Originally the project was developed to target the Konami Original Controller (KOC) and Arcade Style Controller (ASC) for the game **beatmania IIDX**, but the project has grown to support many more controllers besides that. Along with providing support for the original consoles these controllers were designed for, the project offers the following improvements:

* Native USB support, allowing for low- to zero-latency usage for software simulators such as **Lunatic Rave 2**.
* Improved rotary encoder handling, including tunable relative-based (direction) tracking as well as open-loop absolute position tracking.
* Lighting, including both host-controlled (HID) lighting in software that supports it, as well as input-based fallback lighting. This includes the support of both direct-driven lighting as well as WS2812 RGB LED support.

## Building USBemani

First, make sure you have an up-to-date version of `avr-gcc`. This documentation does not cover the installation of `avr-gcc` as this can differ between Windows, Linux, and macOS platforms. Once installed, clone this repository, making sure to include submodules:

```
git clone --recursive https://github.com/progmem/re-usbemani.git
```

Navigate into the folder this code was cloned to, then run the following to build:

```
make clean all
```

By default, this will build the firmware for use on the **USBemani v3 Reference Hardware**, and should not be used out of the box. Read through `usbemani.c`, specifically the `SetupHardware` function, for more information on tailoring to your particular needs.
