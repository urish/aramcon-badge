cd build\zephyr
C:\Python27\Scripts\nrfutil.exe pkg generate --hw-version 52 --sd-req=0x00  --application zephyr.hex --application-version 1 pkg.zip
C:\Python27\Scripts\nrfutil.exe dfu usb-serial -pkg pkg.zip -p com25
cd ..\..


