# I2C EEPROM VFS Adapter for CircuitPython
# Released under The MIT License (MIT)
#
# Copyright (c) 2019 Uri Shaked
import storage

class EEPROMVFS:
    SECTOR_SIZE = 512

    def __init__(self, eeprom):
        self._eeprom = eeprom

    def readblocks(self, n, buf):
        self._eeprom.readinto(n * self.SECTOR_SIZE, buf)
        return 0

    def writeblocks(self, n, buf):
        self._eeprom.writebuf(n * self.SECTOR_SIZE, buf)
        return 0

    def ioctl(self, op, arg):
        if op == 1:  # BP_IOCTL_INIT
            return 0 # Success
        if op == 4:  # BP_IOCTL_SEC_COUNT
            return self._eeprom.size // self.SECTOR_SIZE
        if op == 5:  # BP_IOCTL_SEC_SIZE
            return self.SECTOR_SIZE

def mount_eeprom(eeprom, path):
    eepromvfs = EEPROMVFS(eeprom)
    try:
        fs = storage.VfsFat(eepromvfs)
        storage.mount(fs, path)
        return fs
    except OSError:
        # The diskette is probably not initialized, format it first
        storage.VfsFat.mkfs(eepromvfs)
        fs = storage.VfsFat(eepromvfs)
        storage.mount(fs, path)
        return fs
