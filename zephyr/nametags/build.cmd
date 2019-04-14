@echo off

set ZEPHYR_BASE=c:\plow\zephyr
set ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
set GNUARMEMB_TOOLCHAIN_PATH=C:\dev\gcc-arm-none-eabi

IF NOT EXIST build (
  MKDIR build
  cd build
  cmake -v -GNinja -DBOARD_ROOT=$PS0/../boards -DBOARD=badge -DBOARD_DIR=../../boards/arm/badge .. && call ninja
  cd ..
) ELSE (
  cd build
  ninja
  cd ..
)
