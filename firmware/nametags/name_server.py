# Nametags app for Aramcon Badge
# Copyright (C) 2019, Uri Shaked

from bleio import Characteristic, CharacteristicBuffer, Peripheral, Service, UUID, adapter
from adafruit_ble.advertising import ServerAdvertisement
from badgeio import badge
from led_spinner import LedSpinner
from bitmapsave import bitmap_save
import displayio
import storage

BT_UUID_DISP = UUID(0xfeef)
BT_UUID_DISP_DATA = UUID(0xfeee)
BT_UUID_BATTERY = UUID(0xfeed)

# Protocol:
# struct packet {
#   int has_offset: 1; // MSB
#   int size: 7;
#   uint16_t offset; // little-endian. Optional, only if has_offset is 1
#   uint8_t bytes[size];
# }

class NameServer:
    def __init__(self):
        addr = adapter.address.address_bytes
        self.device_name = "BADGE-{:02X}{:02X}{:02X}".format(addr[3], addr[4], addr[5])
        self._disp_char = Characteristic(BT_UUID_DISP_DATA, properties=Characteristic.WRITE)
        self._battery_char = Characteristic(BT_UUID_BATTERY, properties=Characteristic.READ | Characteristic.NOTIFY)
        self._disp_buffer = CharacteristicBuffer(self._disp_char, buffer_size=256)
        self._disp_service = Service(BT_UUID_DISP, (self._disp_char, self._battery_char))
        self._periph = Peripheral((self._disp_service,), name=self.device_name)
        self._advertisement = ServerAdvertisement(self._periph)
        self._bitmap = displayio.Bitmap(badge.display.width, badge.display.height, 2)
        self._spinner = LedSpinner(badge.pixels)
        self._palette = displayio.Palette(2)
        self._palette[0] = 0xffffff
        self._palette[1] = 0x000000
        self._offset = 0
        self._bufsize = 0
        self._dirty = False
        self._advertising = False
        self._connected = False
    
    def start_advertising(self):
        self._periph.start_advertising(self._advertisement.advertising_data_bytes,
                                       scan_response=self._advertisement.scan_response_bytes,
                                       interval=0.1)
        self._advertising = True
    
    def stop_advertising(self):
        self._periph.stop_advertising()
        self._advertising = False

    @property
    def advertising(self):
        return self._advertising and not self._periph.connected
    
    @property
    def connected(self):
        return self._periph.connected
    
    def update(self):
        if self._connected and not self._periph.connected and self._advertising:
            # Disconnect event - we need to restart advertising
            self.start_advertising()
        self._connected = self._periph.connected
        while self._disp_buffer.in_waiting > 0:
            if self._bufsize == 0:
                value = int.from_bytes(self._disp_buffer.read(1), 'little')
                if value == 0:
                    self._finish_update()
                    continue
                self._bufsize = value & 0x7f
                if value & 0x80:
                    self._offset = None
            if self._offset is None and self._disp_buffer.in_waiting >= 2:
                self._offset = int.from_bytes(self._disp_buffer.read(2), 'little')
            if self._bufsize > 0 and self._offset is not None:
                data = self._disp_buffer.read(min(self._bufsize, self._disp_buffer.in_waiting))
                self._bufsize -= len(data)
                for i in range(len(data)):
                    for bit in range(8):
                        self._bitmap[self._offset*8+bit] = 1 if data[i] & (1 << bit) else 0
                    self._offset += 1
                self._spinner.next()
        if self._dirty and badge.display.time_to_refresh == 0:
            badge.display.refresh()
            self._dirty = False
        #if self.connected:
        #    self._battery_char.value = '{:.2f}'.format(badge.battery_voltage).encode('utf-8')

    def _store_bitmap(self):
        try:
            storage.remount('/', False)
        except:
            pass
        try:
            bitmap_save('/nametag.bmp', self._bitmap)
        except Exception as err:
            print("Couldn't save file: {}".format(err))
        try:
            storage.remount('/', True)
        except:
            pass
    
    def _finish_update(self):
        print("Update done!")
        self._offset = 0
        self._bufsize = 0
        badge.pixels.fill(0)
        frame = displayio.Group()
        frame.append(displayio.TileGrid(self._bitmap, pixel_shader=self._palette))
        badge.display.show(frame)
        self._dirty = True
        self._store_bitmap()
