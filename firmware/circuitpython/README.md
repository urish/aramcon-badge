# Setup Instructions

1. Download `firmware.uf2` from the [release page](https://github.com/urish/aramcon-badge/releases)
2. Connect the board to your computer using a USB cable and switch it off
3. Press the left key and switch the board on. This will start the UF2 bootloader and a new drive will show up on your computer.
4. Copy `firmware.uf2` to the ARAMBOOT drive
5. After boot, a new usb drive will appear, copy a 'main.py' to the drive, and it will run, however, don't forget to sync the drive.
   It is possible on windows using sync64.exe <drive> (https://docs.microsoft.com/en-us/sysinternals/downloads/sync)
