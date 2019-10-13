# Simple I2C EEPROM Driver for CircuitPython
# Released under The MIT License (MIT)
#
# Copyright (c) 2019 Uri Shaked

class EEPROM:
    """
    .. code-block:: python
        from eeprom import EEPROM

        e = EEPROM(i2c)
        e.write(0, 100)          # write the value 100 to memory address 0
        print(e.read(0, 1)[0])   # will print 100
    """

    def __init__(self, i2c, eeprom_addr = 0x50, eeprom_size = 256):
        self._i2c = i2c
        self._addr = eeprom_addr
        self._eeprom_size = eeprom_size

    def _encode_addr(self, mem_addr):
        if mem_addr >= self._eeprom_size:
            raise Exception("Invalid address: {}".format(mem_addr))
        if self._eeprom_size <= 256:
            return [mem_addr]
        else:
            return [mem_addr >> 8, mem_addr & 0xff]
    
    def read(self, mem_addr, size):
        mem_addr = self._encode_addr(mem_addr)
        while not self._i2c.try_lock():
            pass
        try:
            buf = bytearray(size)
            self._i2c.writeto(self._addr, bytearray(mem_addr))
            self._i2c.readfrom_into(self._addr, buf)
            return buf
        finally:
            self._i2c.unlock()

    def write(self, mem_addr, value):
        mem_addr = self._encode_addr(mem_addr)
        if value < 0 or value > 0xff:
            raise Exception("Invalid value: {}".format(value))
        while not self._i2c.try_lock():
            pass
        try:
            self._i2c.writeto(self._addr, bytearray(mem_addr + [value]))
        finally:
            self._i2c.unlock()
