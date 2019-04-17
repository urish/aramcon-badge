## Smart Badge Project

nRF52840-Based Smart Badge with Bluetooth, Thread Mesh Network, 2.9" ePaper Display, Neopixels, built-in MP3 Sound decoder and more!

## Directory Structure

### Badge firmware

* [zephyr/nametags](zephyr/nametags) - Name tags app. Use [Zephyr](https://zephyrproject.org) to build it.

### Other stuff

* [pcb/badge](pcb/badge) - Badge PCB, designed with KiCad
* [pcb/tiny-sao](pcb/tiny-sao) - PCB for ATTiny85 Shitty Add-On 
* [pcb/svg2mod](pcb/svg2mod) - [svg2mod](https://github.com/mtl/svg2mod) with some patches
* [zephyr/hardware-test](zephyr/hardware-test) - Hardware test app
* [zephyr/boards/arm/badge](zephyr/boards/arm/badge) - Board definition for ZephyrProject
* [epaper](epaper) - Preliminary code to interface nRF52840 with various EPDs
* [graphics](graphics) - Some demo files with graphics for 2.7" and 2.9" EPDs
* [tools](tools) - Various python scripts, e.g. convert images to ePaper bitmap
