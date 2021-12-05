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

Firmware and EEPROM files can be found in the `build` folder.

## Configuration

Out of the box, USBemani won't do anything until the board is configured. This is planned to be done through the Configuration Tool once it's built, but until then you will need to build and flash your own EEPROM file. The easiest way to build an EEPROM file is to modify `eeprom.h`. This file is broken up into three chunks: a `Header`, the `Device` configuration, and the `User` configuration.

### Header

The header must contain a `.string` containing the literal text `USBMNIv3`. This area also contains two CRC16s that we will need to calculate; a helper utility has been written to help facilitate this.

### Device Configuration

Device configuration is intended to reflect the capabilities of the hardware and connections to the microcontroller. These are intended to be rarely or infrequently updated.

#### PS1/PS2 Support `PS2`

Enabling PS1/PS2 support requires the following:

* A `.pin`, set to either `PS2_C6` or `PS2_C7`. This is the pin to be used as the Acknowledge line for the PS2.
* An `.invert`, set to either `PS2_DIRECT` or `PS2_TRANSISTOR`. This determines is the MISO line needs a transistor/MOSFET to act as a buffer. It is **highly advised** to use `PS2_TRANSISTOR` along with a 2N7000-series MOSFET, due to the varying quality of cables out there.

If you don't need PS1/PS2 support, set `.pin` to `PS2_NC`. If you are using a Pro Micro, keep in mind the following:

* At this time, you can only have PS2 _or_ RGB enabled. Pins on ports B, D and F are reserved for generalized input/output purposes, and E6 is used for diagnostic purposes, leaving only C6 and C7.
* PS2 requires four SPI pins, which cannot be moved: B0 (CS, used by the RXLED), B1 (SCLK, bottom-right), B2 (MOSI, bottom-right), and B3 (MISO, bottom-right). You will need to solder to the RXLED resistor pad closest to the edge of the board.

#### RGB Support `RGB`

Enabling RGB requires the following:

* A `.pin`, set to either `RGB_C6` or `RGB_C7`. This is the pin to be used as the WS2812B RGB data line. It is suggested to place a 300-500 ohm resistor between the data line and the pin on the microcontroller.
* A `.quantity`. Set this to the total number of LEDs connected to your controller.

If you don't need RGB support, set `.pin` to `RGB_NC`. If you are using a Pro Micro, keep in mind the following:

* At this time, you can only have PS2 _or_ RGB enabled. Pins on ports B, D and F are reserved for generalized input/output purposes, and E6 is used for diagnostic purposes, leaving only C6 and C7.

#### Digital Inputs `Input`

Inputs are stored in a single `.pin` array, and requires the following:

* One or more `PIN_XY` declarations. Replace `XY` with the AVR port and pin numbers, e.g. `PIN_B7`. Any pins on ports B, D, and F are available for use.
* Up to 16 entries are allowed.
* If fewer than 16 entries are used, add an additional `PIN_NC` entry. This tells the configuration parser to stop parsing.

#### Digital Outputs `Output`

Outputs are stored in a single `.pin` array. Configuration of this array depends on if you're using **direct output** or **latched output**.

Direct output requires the following:

* One or more `PIN_XY` declarations. Replace `XY` with the AVR port and pin numbers, e.g. `PIN_B7`. Any pins on ports B, D, and F are available for use. If you need to invert the output, `OR` the pin with `OUT_INV`, e.g. `PIN_B7 | OUT_INV`.
* Up to 16 entries are allowed. These entries must be unique, and cannot be shared across other similar blocks e.g. the same pin used on both `Input` and `Output` blocks.
* If fewer than 16 entries are used, add an additional `PIN_NC` entry. This tells the configuration parser to stop parsing.

It is highly advised to use a transistor or MOSFET to drive each of your outputs. The AVR microcontrollers can only source or sink a certain quantity of current; too much can cause damage to the microcontroller.

Latched output requires the following:

* One or more latch chips, such as the `SN74HCT573NSR`. These chips can handle 8 inputs and outputs; you can use one for every 8 I/O. Wire each input pin directly to each digital pin on the AVR. Wire `/OE` to ground.
* One 10k resistor per digital input. This goes between the digital input source (e.g. button) and the pin on the AVR.

* One 22k resistor. This goes between the `LE` pin and `VCC`. If you are using multiple chips, wire this between any one of the `LE` pins and `VCC`.
* A single `PIN_XY` declaration for your latch pin. Replace `XY` with the AVR port and pin numbers, e.g. `PIN_B7`. Any pins on ports B, D, and F are available for use. `OR` this with `OUT_LATCH`, e.g. `PIN_B7 | OUT_LATCH`. If you need to invert the output of all latched, `OR` the pin with `OUT_INV`, e.g. `PIN_B7 | OUT_INV`. Wire this pin to the `LE` pin on the latch chip. If you are using multiple chips, wire this to all `LE` pins.
* A second `PIN_NC` declaration immediately after.

In latched output mode, each digital input acts as a digital output as well. Digital output data is sent periodically to the latch chip, which then latches the data to allow for persistent output. The 10k resistors between digital inputs are to prevent shorts during mode switches. It is highly advised to use a transistor or MOSFET to drive each of your outputs.

#### Rotary Encoders `Encoder`

Up to 5 encoders are supported at a single time, each represented as its own object. Configuration is as follows:

* For each encoder, declare a `.pin_a` and a `.pin_b` using the `PIN_XY` declaration. Replace `XY` with the AVR port and pin numbers, e.g. `PIN_B7`. Any pins on ports B, D, and F are available for use.
* Declare the `.ppr` of the encoder. Use the PPR explicitly declared by the part. If you are using an encoder wheel, indicate the number of *teeth* here. USBemani automatically calculates the number of states from this, so a number that is too high  or too low will cause the encoder to move faster or slower than expected.
* If fewer than 5 encoders are being used, add an additional entry. Mark `.pin_a` with `PIN_NC` and leave the other two entries unpopulated.

USBemani round-robins through these encoders at a polling rate of 8kHz. The more encoders, the longer of a gap between reads and the increased likelihood of dropped pulses (for 2 encoders, the effective rate is 4kHz; for 5 encodes, the effective rate is 1.6kHz). For games using all 5 encoders, use lower-resolution encoders for the best results.

It is _highly advised_ not to go overboard on encoder PPR. High-resolution encoders do exist (e.g. 600PPR), but offer zero improvement to gameplay while also allowing for a greater chance of dropped inputs. A similar encoder at e.g. 150PPR should be suitable for gameplay.

#### Analog Inputs `Analog`

Up to 12 analog inputs are supported. These provide an 8-bit output along with a digital representation of the output. These are stored in a single `.pin` array, and requires the following:

* One or more declarations from the following list: `ANALOG_F0, ANALOG_F1, ANALOG_F4 ANALOG_F5, ANALOG_F6, ANALOG_F7, ANALOG_D4 ANALOG_D6, ANALOG_D7, ANALOG_B4, ANALOG_B5, ANALOG_B6`
  * Only these 12 pins are supported, as the microcontroller only connects these pins to the analog subsystems at a hardware level.
* If fewer than 12 entries are used, add an additional `ANALOG_NC` entry. This tells the configuration parser to stop parsing.

USBemani round-robins through analog inputs at approximately 5kHz. The more analog inputs, the more time before an analog input is read and updated. At 12 analog inputs, the effective polling rate is around 400Hz.

### User Configuration

User configuration handles anything that could be considered a "preference" by a given user. A single model of game controller and control board may share a common Device configuration, but varying User configurations depending on the needs of these users.

#### Datasource Declarations

This portion of the config makes heavy use of `CONFIG_DATASOURCE` identifiers. These identifiers break a single byte into two nibbles:

* The high nibble contains the *source* that should be referenced.
* The low nibble contains the index of that source.

Datasources should be `OR`'d with a given index to form a complete datasource identifier. The following datasources are available for use:

* `CONFIG_DATASOURCE_ALWAYS` indicates that something should always happen. This doesn't require an index.
* `CONFIG_DATASOURCE_DIGITAL` refers to digital inputs. `OR` this with a value between 0 and 15, with 0 referring to your first button.
* `CONFIG_DATASOURCE_ANALOG` refers to analog inputs and the analog representation of this data. `OR` this with a value between 0 and 11.
* `CONFIG_DATASOURCE_ANALOG_DIGI` refers to the digital representation of an analog input. `OR` this with a value between 0 and 11.
* `CONFIG_DATASOURCE_ENCODER` refers to an encoder's position. `OR` this with a value between 0 and 4.
* `CONFIG_DATASOURCE_ENCODER_CCW` and `CONFIG_DATASOURCE_ENCODER_CW` refer to an encoder moving in the indicated direction. `OR` this with a value between 0 and 4.
  * If you need to trigger on encoder movement, regardless of the direction, use `CONFIG_DATASOURCE_ENCODER_DIR`.
* `CONFIG_DATASOURCE_OUTPUT` refers to the 16 available output channels. These are triggered either on HID lighting data or on 'fallback' data declarations. This is commonly used for triggering RGB lighting effects, but also provides niche cases, like triggering PS2 inputs by feeding in HID lighting data. `OR` this with a value between 0 and 15.
* `CONFIG_DATASOURCE_NC` should be used to populate or terminate configurables when the configurable is not needed.

#### USB Mapping `USBMap`

Along with the 5 fixed rotary encoder axes, USBemani supports customizable mapping for the following USB gamepad controls:

* Left and right analog sticks, each treated as a digital axis from -100 to 100.
* 16 buttons.

This section should be declared as follows:

* A Datasource Declaration for each of the following axes: `.map_lx_neg, .map_lx_pos, .map_ly_neg, .map_ly_pos, .map_rx_neg, .map_rx_pos, .map_ry_neg, .map_ry_pos`. Declare `CONFIG_DATASOURCE_NC` if a given axis isn't being used.
* An `.map_button` array, containing one or more Datasource Declarations. Up to 16 declarations are allowed. If fewer than 16 are used, terminate this array with `CONFIG_DATASOURCE_NC`.

#### PS2 Mapping `PS2Map`

USBemani supports a separate mapping for PS1 and PS2. This is presented as an array of objects, with each object containing the following:

* A `.source`, which is a Datasource Declaration.
* An `.output`, containing one or more `PS2_` button masks. If you need a single source to trigger multiple buttons, `OR` these masks together, e.g. `PS2_LEFT | PS2_UP | PS2_DOWN`.
  * For certain games requiring an "identity quirk", use `CONFIG_DATASOURCE_ALWAYS` as the datasource.

Up to 16 objects are available. If fewer than 16 are used, terminate this array of objects by adding an object with a `.source` of `CONFIG_DATASOURCE_NC`.

#### Output Mapping `Out`

USBemani supports up to 16 output channels. These can be controlled either:

* Via HID lighting packets, or
* A fallback mapping when HID lighting data is not available.

This configuration allows for customizing the following:

* A `.hid_timeout`, declaring how long to wait for the next HID lighting packet before switching back to the fallback mapping. This is declared in increments of 1/60 seconds; set this to 60 to wait up to 1 second between HID lighting packets. This is an 8-bit number, allowing for a maximum wait of 4.25 seconds (255).
* An array of `.channels`, containing one or more Datasource Declarations. These datasources are used to determine when an output channel should be active whenever HID lighting data is not available. Up to 16 channels are available. If fewer than 16 are used, terminate this array with `CONFIG_DATASOURCE_NC`.

#### Rotary Encoders `Encoder`

USBemani supports customization of the encoder behavior, presented as an array of objects. Each object contains the following:

* A `.hold_time`, declaring how long an encoder's direction should be held. This is declared in increments of 1/8000 seconds (the polling rate).
* A `.target_max` and `.target_rot`. These allow for a logical mapping of a given encoder's position to the position expected by a given game.
  * Set `.target_max` to the maximum value handled internally by the game. For certain implementations of IIDX, this would be `256` (0-255 internally, +1). For certain implementations of Sound Voltex, this would be `1024` (0-1023 internally, +1).
  * Set `.target_rot` to the value you wish to increment up to after one full rotation fo the encoder. USBemani will try to match this value as close as possible. For certain implementations of IIDX, this would be either `72` or `144`.

One object must be presented for each encoder. If you wish for a 1-to-1 mapping (one full rotation of the encoder = one full cycle through the values), set both `.target_max` and `.target_rot` to a value of `-1`.

Unlike other arrays, you do not need to terminate this list in any way. USBemani will only read up to the number of active encoders.

#### Analog Inputs `Analog`

USBemani supports the customization of thresholds for each of the analog inputs. This is presented as an array of objects containing the following:

* A `.trigger`, indicating the value at which the digital representation of the analog input should be considered "pressed".
* A `.release`, indicating the value at which the digital representation of the analog input should be considered "released".

One object must be presented for each analog input. Unlike other arrays, you do not need to terminate this list in any way. USBemani will only read up to the number of active analog inputs.

#### RGB Global Configuration `RGB`

USBemani supports the customization of the following for all RGB effects:

* A `.fade_process`, containing one of the following:
  * `CONFIG_RGB_FRAMEPROCESS_CLEAR` clears any inactive effect without any fade.
  * `CONFIG_RGB_FRAMEPROCESS_FADE` fades out any inactive LEDs based on the `.fade_rate`.
  * `CONFIG_RGB_FRAMEPROCESS_FADERANDOM` fades out each inactive LED based on a random rate, influenced by the `.fade_rate`.
* A `.fade_rate`. Internally, color channels are faded at a rate of `1 / (2^fade_rate)`.
  * A `.fade_rate` of `0` is effectively the same as `CONFIG_RGB_FRAMEPROCESS_CLEAR`.
  * A `.fade_rate` of `1` will fade a given LED by half of it's value on each frame.
  * A `.fade_rate` of `2` will fade a given LED by a quarter of it's value on each frame.
  * A max value of `8` is allowed.

The following customizations affect the "Splash" RGB effect:

* A `.splash_fade_rate`. This is used to determine the rate at which the "splashes" decay. This follows a similar pattern to `.fade_rate`. A max value of 8 is allowed.
* A `.splash_bounds_start` and `.splash_bounds_end` define the bounding box that the splash effect is forced to be contained within. These refer to the 0-indexed position of each LED. Set these to the first and last LED that the effect is allowed to draw to, respectively.
  * For example, if you only want the effect to draw on the first 16 LEDs, set the values to `0` and `15`.

#### RGB Effect Declarations

USBemani provides a library of RGB draw routines, broken out into two separate mechanisms:

* A **color provider**, which provides one or more colors for a given effect.
* The **effect** itself, which determines how often to pull a color from the color provider for drawing.

The following **color providers** are available:

* `CONFIG_COLOR_PROVIDER_HUE` provides a single, fixed hue.
* `CONFIG_COLOR_PROVIDER_HUECYCLE` cycles the hue each time the effect requests a color.
* `CONFIG_COLOR_PROVIDER_HUERANDOM` cycles to a random hue each time the effect requests a color.
* `CONFIG_COLOR_PROVIDER_RAINBOW` provides up to a declared quantity of hues, using an input value to determine the first hue that should be provided.

The following **effects** determine how these colors are pulled:

* `CONFIG_EFFECT_SINGLECOLOR` pulls a single color for a block of RGB LEDs being drawn. If drawing 24 LEDs, one color will be pulled and used for all 24 LEDs. When a given datasource is active, colors are pulled on each frame.
* `CONFIG_EFFECT_MULTICOLOR` pulls a single color for each RGB LED being drawn. If drawing 24 LEDs, 24 colors are pulled and used in order from left to right. When a given datasource is active, colors are pulled on each frame.
* `CONFIG_EFFECT_CYCLEONPRESS` pulls a single color for a block of RGB LEDs being drawn. If drawing 24 LEDs, one color will be pulled and used for all 24 LEDs. The color is only pulled at the time the datasource becomes active, and the same color is used until the datasource becomes inactive.
* `CONFIG_EFFECT_ONLYONPRESS` pulls a single color for a block of RGB LEDs being drawn. If drawing 24 LEDs, one color will be pulled and used for all 24 LEDs. The LEDs are only drawn **once** at the time the datasource becomes active, and will not draw again until the datasource becomes inactive.
* `CONFIG_EFFECT_SPLASH` performs the same effect that `CONFIG_EFFECT_CYCLEONPRESS`. In addition to this effect, additional child effects are spawned that "splash" out to the left and right using the color that has been drawn. These splash effects decay over time based on the `.splash_fade_rate`.

#### RGB Effects `Effect`

RGB effects are declared as an array of objects, containing a number of attributes:

* A `.trigger`, containing a Datasource Declarations. This determines when the effect is considered "active".
* An `.effect`, containing a `CONFIG_COLORPROVIDER` and a `CONFIG_EFFECT`. `OR` the two of these together, e.g. `CONFIG_COLOR_PROVIDER_RAINBOW | CONFIG_EFFECT_MULTICOLOR`.
* The `.start`, indicating the first LED to draw.
* The `.size`, indicating how many LEDs to draw.

Additionally, the following attributes must be declared for certain color providers:

* For `CONFIG_COLOR_PROVIDER_RAINBOW`, declare the following:
  * A `.source`, containing a Datasource Declarations. This determines where to pull a value from for a given effect. Common sources would be `CONFIG_DATASOURCE_ENCODER` or `CONFIG_DATASOURCE_ANALOG`.
  * A `.quantity`, determining the number of colors to provide. For `CONFIG_EFFECT_MULTICOLOR`, try starting with the same number as your `.size`. Increasing or decreasing this may cause the effect to appear to automatically "rotate". For all others, start with 1. A value greater than `0` must be provided.
* For `CONFIG_COLOR_PROVIDER_HUE` and `CONFIG_COLOR_PROVIDER_HUECYCLE`, declare the following:
  * A `.hue`. This will be a fixed hue to draw, or the starting hue to use.
  * For `HUECYCLE`, a `.hue_delta`. This will indicate how far to advance the hue with each effect call.

With these all of these declarations, you can build some interesting effects. Some examples of effects are as follows:

* Combine `CONFIG_COLOR_PROVIDER_RAINBOW` with `CONFIG_EFFECT_MULTICOLOR` for a "rainbow ring". Combine that with a rotary encoder datasource for a rainbow ring around e.g. a IIDX turntable.
* Combine `CONFIG_COLOR_PROVIDER_RAINBOW` with `CONFIG_EFFECT_SINGLECOLOR` for a shifting hue based on the turntable position instead.
* Combine `CONFIG_COLOR_PROVIDER_HUECYCLE` with `CONFIG_EFFECT_SINGLECOLOR` to have buttons shift through hues when held.
* Combine `CONFIG_COLOR_PROVIDER_HUECYCLE` or `CONFIG_EFFECT_HUERANDOM` with `CONFIG_EFFECT_CYCLEONPRESS` to have buttons cycle through colors when pressed.
* Combine `CONFIG_COLOR_PROVIDER_HUECYCLE` with `CONFIG_EFFECT_SPLASH` to generate splash effects that cycle through hues each time the button is pressed.

* **Note: You probably don't want to combine `CONFIG_EFFECT_MULTICOLOR` or `CONFIG_EFFECT_SINGLECOLOR` with `CONFIG_COLOR_PROVIDER_HUERANDOM`. This will draw a random hue for every frame, which could potentially cause a seizure.**

### Flashing the EEPROM

Once your EEPROM configuration has been built, you will need to do the following in order to upload it:

* Run `make clean all`. This will build both the firmware and `eeprom.h`. Your EEPROM file can be found in the `build` folder, as `usbemani.eep`.
* Convert the `.eep` into a binary format, placing the file in the `utils` folder with the following command: `avr-objcopy -I ihex build/usbemani.eep -O binary utils/input.eep`
* Navigate to the `utils` folder and build `calc_crc16`: `gcc -o calc_crc16 calc_crc16.c`
* Run `calc_crc16`. This will read in `input.eep` and generate an `output.eep` containing the calculated CRC16 values. USBemani uses these to determine if the data in EEPROM is valid.
* Finally, upload the EEPROM. This can be done with a tool like `avrdudess`, or using the command line `avrdude` and a command similar to the following: `avrdude -P /dev/tty.usbmodem* -c avr109 -p m32u4 -U eeprom:r:output.eep:r`.

