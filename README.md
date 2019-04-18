## Smart Badge Project

nRF52840-Based Smart Badge with Bluetooth, Thread Mesh Network, 2.9" ePaper Display, Neopixels, built-in MP3 Sound decoder and more!

## Flashing The Badge

### Prerequisites
1. Install [nrfutil](https://github.com/NordicSemiconductor/pc-nrfutil) using `pip` (on all platforms) or [download windows binary](https://github.com/NordicSemiconductor/pc-nrfutil/releases)
2. Build a firmware hex file (see [firmware/nametags](firmware/nametags) for instructions) 

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
