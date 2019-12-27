# Simple I2C EEPROM Driver for CircuitPython
# Released under The MIT License (MIT)
#
# Copyright (c) 2019 Uri Shaked

import time

class EEPROM:
    """
    .. code-block:: python
        from eeprom import EEPROM

        e = EEPROM(i2c)
        e.write(0, 100)          # write the value 100 to memory address 0
        print(e.read(0, 1)[0])   # will print 100
    """

    def __init__(self, i2c, eeprom_addr = 0x50, eeprom_size = 256, page_size = 16):
        self._i2c = i2c
        self._addr = eeprom_addr
        self._eeprom_size = eeprom_size
        self._page_size = 16

    def _encode_addr(self, mem_addr):
        if mem_addr >= self._eeprom_size:
            raise Exception("Invalid address: {}".format(mem_addr))
        if self._eeprom_size <= 256:
            return [mem_addr]
        else:
            return [mem_addr >> 8, mem_addr & 0xff]

    @property
    def size(self):
        """The size of the EEPROM, in bytes"""
        return self._eeprom_size

    def readinto(self, mem_addr, buf):
        mem_addr = self._encode_addr(mem_addr)
        while not self._i2c.try_lock():
            pass
        try:
            self._i2c.writeto(self._addr, bytearray(mem_addr), stop=False)
            self._i2c.readfrom_into(self._addr, buf)
        finally:
            self._i2c.unlock()
    
    def read(self, mem_addr, size):
        buf = bytearray(size)
        self.readinto(mem_addr, buf)
        return buf

    def _write(self, mem_addr, buf):
        """ must write to a single page """
        mem_addr = self._encode_addr(mem_addr)
        while not self._i2c.try_lock():
            pass
        try:
            self._i2c.writeto(self._addr, bytearray(bytes(mem_addr) + buf))

            # Wait for the write to complete (polling)
            start = time.monotonic()
            buf = bytearray(0)
            while time.monotonic() - start < 0.01:
                try:
                    self._i2c.readfrom_into(self._addr, buf)
                    return
                except:
                    pass
        finally:
            self._i2c.unlock()

    def write(self, mem_addr, value):
        if value < 0 or value > 0xff:
            raise Exception("Invalid value: {}".format(value))
        self._write(mem_addr, bytes([value]))

    def writebuf(self, mem_addr, buf):
        page = self._page_size - (mem_addr % self._page_size)
        while len(buf) > page:
            self._write(mem_addr, bytes(buf[:page]))
            mem_addr += page
            buf = buf[page:]
            page = 16
        self._write(mem_addr, bytes(buf))
