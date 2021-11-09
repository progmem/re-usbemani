# USBemani

USBemani is a firmware for the ATmega32u4 microcontroller intended to replace the original control board for many of Konami's home rhythm game controllers. Originally the project was developed to target the Konami Original Controller (KOC) and Arcade Style Controller (ASC) for the game **beatmania IIDX**, but the project has grown to support many more controllers besides that. Along with providing support for the original consoles these controllers were designed for, the project offers the following improvements:

* Native USB support, allowing for low- to zero-latency usage for software simulators such as **Lunatic Rave 2**.
* Improved rotary encoder handling, including tunable relative-based (direction) tracking as well as open-loop absolute position tracking.
* Lighting, including both host-controlled (HID) lighting in software that supports it, as well as input-based fallback lighting. This includes the support of both direct-driven lighting as well as WS2812 RGB LED support.

## Doesn't this project already exist?

USBemani is a fairly old project, and has been in need of a rewrite for some time. This version of the codebase aims to make use of the experience I've gained since then. Personal goals for this rewrite are:

* Support significantly more development boards. This is done by completely rewriting how inputs and outputs are handled, allowing for the full use of ports `B`, `D`, and `F`.
  * The new input engine currently allows for up to 16 buttons and 5 rotary encoders across all available pins.
  * Rotary encoders are no longer tied to specific port/pin combinations. A rotary encoder can make use of a pin on port B and a different pin on port D without any issue.
* Improved PS1 and PS2 support. This is done with a new approach that makes liberal use of variable and function pointers, along with improved timings collected from real hardware.
  * Inputs are handled through a list processor, retrieving data from variable pointers and checking against masks.
  * A single input can map to one or more buttons. Multiple inputs are also able to be mapped to the same button.
  * "Identity quirks" are now handled via an "always on" input.
* RGB support. This is done with a custom-written RGB processor, built to push data out without consuming a large chunk of system time.
  * Up to 144 LEDs have been tested, at 30FPS.
  * Writes are handled via single bit pushes that leverage known properties of the WS2812 protocol. This allows for individual bit pushes to occur without affecting the execution of other interrupts.
* RGB effect support. This is done with a list processor and functions to combine an effect handler and a color provider into effects that draw one or more colors in a variety of ways.
  * [Components of FastLED](https://github.com/FastLED/FastLED) are incorporated here, such as the use of Rainbow HSV. This allows for the use of many colors versus a predefined palette.
  * Color providers handle the creation of RGB colors. Providers can take in fixed or variable data, and can provide the same or different colors based one each call.
  * Effect handlers dictate how these colors are drawn. An effect handler can choose to make a single or multiple color provider calls, allowing for one or more colors to be drawn to a given strip.
  * Effect handlers can spawn additional effect handlers, allowing for complex effects like splashes.
* Configuration. USBemani previously handled everything via a stored configuration and a configuration tool, and I want to handle it the same way here. This is a work-in-progress, but once complete, it will allow for:
  * Input configuration, allowing for the mapping of buttons and encoders to pins.
  * Button mapping for PS1/PS2, including the configuration of an "identity quirk" for games that require it as well as the mapping of a single button to one or more PS1/PS2 buttons.
  * RGB effects, allowing for effects to be built by dictating what controls the effect, what inputs are involved in that effect, and where that effect should be drawn. The plan is to provide a number of available effects and color providers built-in, with the configuration tool offering both common usecases as well as full customization.
