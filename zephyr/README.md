# Zephyr Project Firmware for AramCon Badge

Instructions:

1. Follow the [Zephyr Project Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/getting_started.html) to set up your environment
2. git-apply the patch '0001-WIP-IL0373-display-controller-driver-for-zephyr-base.patch' in the zephyr repository inside zephyr sdk.
3. Run `app/build.sh` to build the firmware
4. Write the built firmware from `app/build/zephyr/zephyr.hex` to your device
