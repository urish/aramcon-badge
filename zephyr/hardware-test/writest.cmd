C:\dev\openocd\bin\openocd.exe -d2 -f interface/stlink.cfg -f target/nrf52.cfg -c "init ; halt; nrf5 mass_erase; program build/zephyr/zephyr.hex; reset; exit"
