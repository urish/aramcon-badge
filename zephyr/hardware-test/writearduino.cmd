cd build\zephyr
c:\dev\nrfutil pkg generate --hw-version 52 --sd-req=0x00  --application zephyr.hex --application-version 1 pkg.zip
c:\dev\nrfutil dfu usb_serial -pkg pkg.zip -p com27



