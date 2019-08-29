# Magic Blue BLE bulb driver for CircuitPython
# Copyright (C) 2019, Uri Shaked

import bleio

def parse_color(color):
    if isinstance(color, int):
        return (color >> 16, (color >> 8) & 0xff, color & 0xff)
    elif len(color) == 3:
        return color
    else:
        raise ValueError("Invalid color: " + str(color))

class MagicblueBulb:
    """
    Connect and control a Magicblue light bulb.

    .. code-block:: python
      from magicblue import MagicblueBulb
      import time
      bulb = MagicblueBulb()
      bulb.connect()
      
      # Cross-fade between red and blue
      for i in range(0, 256, 10):
        bulb.color = [255-i, 0, 255]
        time.sleep(0.2)
    """
    
    def __init__(self):
        self._central = bleio.Central()
        self._characteristic = None
        self._color = None
        self._white = None
        self._power_on = True
        self._effect = None
        self._effect_speed = 5

    def connect(self):
        """
        Find a bulb and connect to it
        """
        scanner = bleio.Scanner()
        entries = scanner.scan(1)
        for entry in entries:
            if b"\x07\x03\xf0\xff\xe5\xff\xe0\xff" in entry.advertisement_bytes:
                self._central.connect(entry.address, 5)
                self._find_services()
                return self.connected
        return False
        
    def _find_services(self):
        services = self._central.discover_remote_services()
        for service in services:
            if service.uuid == bleio.UUID(0xffe5):
                self._characteristic = service.characteristics[0]
                return
        # No matching services found
        self._central.disconnect()
    
    @property 
    def connected(self):
        return self._central.connected
        
    @property
    def color(self):
        """ The color of the bulb, as a 3-tuple: (r, g, b). Values in the range of 0-255 """
        return self._color
    
    @color.setter
    def color(self, value):
        self._color = parse_color(value)
        self._characteristic.value = bytes([0x56, value[0], value[1], value[2], 0x00, 0xf0, 0xaa])
    
    @property
    def white(self):
        """ Warm white level in the range of 0-255 """
        return self._white
    
    @white.setter
    def white(self, value):
        self._white = value
        self._characteristic.value = bytes([0x56, 0, 0, 0, value, 0x0f, 0xaa])
        
    @property
    def effect(self):
        """ Special effect index, valid values are between 0x25 and 0x38 """
        return self._effect
    
    @effect.setter
    def effect(self, value):
        self._effect = value
        self._characteristic.value = bytes([0xbb, value, self._effect_speed, 0x44])
        
    @property
    def effect_speed(self):
        """ The speed of the effect. Each unit of speed roughly equals 200ms. """
        return self._effect_speed
    
    @effect_speed.setter
    def effect_speed(self, value):
        self._effect_speed = value
        if self._effect:
            self.effect = self._effect

    @property
    def power_on(self):
        """ ``True`` when the light is on. ``False`` if not. """
        return self._power_on
    
    @power_on.setter
    def power_on(self, value):
        self._power_on = value
        self._characteristic.value = bytes([0xcc, 0x23 if value else 0x24, 0x33])
