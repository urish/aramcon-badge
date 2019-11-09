# Badge Add-On Identification Driver
# Released under The MIT License (MIT)
#
# Copyright (c) 2019 Uri Shaked

"""
The Addon Descriptor is a stored inside a read-only EEPROM
on address 0x50 of the I2C bus. 

There are two variants, a binary and a JSON.

The binary variant can be starts with the following header:

Offset Size     Description 
------ ----     -----------
0      byte[4]  Magic string: 'LIFE'
4      byte     Length of device name
5      byte     Length of driver name
6      byte     Length of driver data buffer
7      byte     Reserved, must be 0

The header is followed by device name, driver name, and optionally
a driver data buffer. The length of each of these elements is 
specified in the header. The device name string may be UTF-8 encoded.
The driver name string must be ASCII encoded.

The JSON variant has a 5-byte header, followed by JSON data:

Offset Size     Description
------ ----     -----------
0      byte[4]  Magic string: 'JSON'
4      byte     Length of JSON Data
5      byte[n]  JSON-encoded object data, encoded in UTF-8

The JSON object must contain the following fields:

Field   Description
-----   -----------
name    The device name
device  The driver name

It may contain additional fields that will be processed by the driver.

For example: { "name": "My amazing addon", "driver": "amazing.py" }
"""

from eeprom import EEPROM
from badgeio import badge
import json

ADDON_MAGIC = b'LIFE'
JSON_MAGIC = b'JSON'

def read_addon_descriptor(eeprom):
    try:
        magic = eeprom.read(0, 4)
        if magic == JSON_MAGIC:
            size, = eeprom.read(4, 1)
            return json.loads(bytes(eeprom.read(5, size)).decode('utf-8'))
        if magic == ADDON_MAGIC:
            offset = len(magic)
            header = eeprom.read(offset, 4)
            offset += 4
            name = bytes(eeprom.read(offset, header[0])).decode('utf-8')
            offset += header[0]
            driver = bytes(eeprom.read(offset, header[1])).decode('ascii')
            offset += header[1]
            data = bytes(eeprom.read(offset, header[2])) if header[2] else b''
            return {
                "name": name,
                "driver": driver,
                "data": data,
            }
    except:
        return None

def write_addon_descriptor(eeprom, descriptor):
    name = descriptor.get('name').encode('utf-8')
    driver = descriptor.get('driver').encode('ascii')
    data = descriptor.get('data', b'')
    header = ADDON_MAGIC + bytearray([len(name), len(driver), len(data), 0])
    eeprom.writebuf(0, header)
    offset = len(header)
    eeprom.writebuf(offset, name)
    offset += len(name)
    eeprom.writebuf(offset, driver)
    offset += len(driver)
    eeprom.writebuf(offset, data)
    return offset + len(data)

def write_addon_json(eeprom, descriptor):
    payload = json.dumps(descriptor).encode('utf-8')
    if len(payload) > 250:
        raise Exception("Payload too big to fit in EEPROM")
    header = JSON_MAGIC + bytearray([len(payload)])
    eeprom.writebuf(0, header)
    eeprom.writebuf(len(header), payload)
    return len(header) + len(payload)
