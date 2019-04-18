# NameTags firmware for Badge

## Build Instructions

1. Install the [Zephyr Project Prerequisites](https://docs.zephyrproject.org/latest/getting_started/index.html#set-up-a-development-system), including the [GNU ARM Embedded toolchain](https://docs.zephyrproject.org/latest/getting_started/toolchain_3rd_party_x_compilers.html#gnu-arm-embedded)
2. Clone the [Zephyr good-display Branch](https://github.com/urish/zephyr/tree/good-display) from our fork
3. (For windows) edit [build.cmd](build.cmd) and change the paths there:
  - `ZEPHYR_BASE` should point to the Zephyr repository you clone in step 2
  - `GNUARMEMB_TOOLCHAIN_PATH` should point to where you installed the [GNU Arm Embedded toolchain](https://docs.zephyrproject.org/latest/getting_started/toolchain_3rd_party_x_compilers.html#gnu-arm-embedded) from step 1
4. Run the build script - `build.cmd` on windows, `./build.sh` on Linux / MAC

At the end of the process you will find the result in `build/zephyr/zephyr.hex`. Enjoy!
