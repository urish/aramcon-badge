# Deprecation notice
This bootloader is deprecated. Please use the [UF2 bootloader](https://github.com/urish/aramcon-badge/releases/tag/Bootloader-27-6-2019) instead. ([Source code for the new bootloader](https://github.com/bmeisels/Adafruit_nRF52_Bootloader/tree/aramcon_badge))


# Bootloader for AramCon Badge (Zephyr App)
Instructions:
1. flash bootloader_combined.hex using jtag
2. compile app
3. create application package using "nrfutil pkg generate --hw-version 52 --sd-req=0x00 --application zephyr.hex --application-version 1 pkg.zip"
4. reboot the board while holding down the left button (pin 2)
5. the board should enter dfu mode
5. flash using "nrfutil dfu usb_serial -pkg pkg.zip -p /dev/ttyACM0"
