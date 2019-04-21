## Smart Badge Project

nRF52840-Based Smart Badge with Bluetooth, Thread Mesh Network, 2.9" ePaper Display, Neopixels, built-in MP3 Sound decoder and more!

## Flashing The Badge

### Prerequisites
1. Install [nrfutil](https://github.com/NordicSemiconductor/pc-nrfutil) using `pip` (on all platforms) or [download windows binary](https://github.com/NordicSemiconductor/pc-nrfutil/releases)
2. Grab a .HEX file from the [releases](https://github.com/urish/aramcon-badge/releases) page, or follow the [project build instructions](firmware/nametags). 

### Instructions
1. Remove the battery from your badge
2. Connect your badge to the PC using a MicroUSB Cable
3. Move the power switch to "BAT" position
4. Press the left key while switching to "USB" position
5. The badge should appear as a Serial port in your computer
6. Using nrfutil, create an application package from your HEX file (e.g. `zephyr.hex`):

    `nrfutil pkg generate --hw-version 52 --sd-req=0x00 --application zephyr.hex --application-version 1 pkg.zip`
    
7. Use nrfutil to flash the board:

    `nrfutil dfu usb-serial -pkg pkg.zip -p COM11`
    
    Change `COM11` to the name of the serial port on your platform (e.g. `/dev/ttyUSB0`)

## Directory Structure

### Badge firmware

* [firmware/nametags](firmware/nametags) - Name tags app. Use [Zephyr](https://zephyrproject.org) to build it.

### Other stuff

* [pcb/badge](pcb/badge) - Badge PCB, designed with KiCad
* [pcb/tiny-sao](pcb/tiny-sao) - PCB for ATTiny85 Shitty Add-On 
* [pcb/svg2mod](pcb/svg2mod) - [svg2mod](https://github.com/mtl/svg2mod) with some patches
* [firmware/bootloader](firmware/bootloader) - nRF52 USB Bootloader .HEX build
* [firmware/hardware-test](firmware/hardware-test) - Hardware test app
* [firmware/boards/arm/badge](firmware/boards/arm/badge) - Board definition for ZephyrProject
* [tools](tools) - Various python scripts, e.g. convert images to ePaper bitmap

## Hardware Information
* [nRF52840](https://infocenter.nordicsemi.com/pdf/nRF52840_OPS_v0.5.pdf) 64MHz ARM Cortex-M4F64 CPU with 1MB flash, 256KB ram, USB, Bluetooth Low Energy 5, Thread, and Zigbee
* [GDEW029T5](http://www.e-paper-display.com/products_detail/productId=347.html) 2.9 inch e-paper glass display
* [VS1003](http://www.vlsi.fi/fileadmin/datasheets/vs1003.pdf) MP3/WMA Audio Codec
* [LIS2DH12](https://www.st.com/resource/en/datasheet/lis2dh12.pdf) I2C Accelerometer
* [GD25Q16C](http://www.elm-tech.com/en/products/spi-flash-memory/gd25q16/gd25q16.pdf) 16MBit Serial Flash
* 4 [WS2812B](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf) "NeoPixel" Addressable RGB LEDs
* 3 CHERRY MX Keyboard Switches
* [Shitty Add-On (SAO) Connector](https://hackaday.com/2018/06/21/this-is-the-year-conference-badges-get-their-own-badges/shitty-add-on-standard/)
* Vibration motor
* Red LED

### Pinout
| Pin   | Component          | Function |
|-------|--------------------|----------|
| P0.00 | VS1003 MP3 Codec   | RESET    |
| P0.01 | E-Paper, MP3 Codec | SCLK     |
| P0.02 | Keyboard Switch    | Left     |
| P0.03 | Accelerometer, SAO | SDA      |
| P0.06 | E-Paper            | RESET    |
| P0.07 | E-Paper            | CS       |
| P0.08 | WS2812 Neopixels   | DATA     |
| P0.12 | E-Paper            | D/C      |
| P0.13 | VS1003 MP3 Codec   | DREQ     |
| P0.17 | Vibrator           | Vibrate  |
| P0.20 | Serial Flash       | SO/IO1   |
| P0.22 | Serial Flash       | SI/IO0   |
| P0.24 | VS1003 MP3 Codec   | xDCS     |
| P0.26 | E-Paper            | BUSY     |
| P0.28 | Accelerometer, SAO | SCL      |
| P0.29 | Keyboard Switch    | Middle   |
| P0.30 | Battery Voltage    | Analog   |
| P0.31 | Keyboard Switch    | Right    |
| P1.00 | Serial Flash       | SCK      |
| P1.02 | Serial Flash       | CS       |
| P1.04 | Serial Flash       | WP/IO2   |
| P1.06 | Serial Flash       | HOLD/IO3 |
| P1.10 | E-Paper, MP3 Codec | MISO     |
| P1.10 | E-Paper, MP3 Codec | MOSI     |
| P1.11 | Red LED            | Cathode  |
| P1.13 | VS1003 MP3 Codec   | CS       |
