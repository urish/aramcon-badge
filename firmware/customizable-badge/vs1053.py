# VS1053 driver for CircuitPython
# Copyright (C) 2018, Uri Shaked

import digitalio
import time
import board

SPI_BAUDRATE = 2000000

VS_WRITE_COMMAND = 0x02
VS_READ_COMMAND = 0x03

SPI_MODE    = 0x0
SPI_STATUS  = 0x1
SPI_BASS    = 0x2
SPI_CLOCKF  = 0x3
SPI_DECODE_TIME  = 0x4
SPI_AUDATA  = 0x5
SPI_WRAM    = 0x6
SPI_WRAMADDR= 0x7
SPI_HDAT0   = 0x8
SPI_HDAT1   = 0x9
SPI_AIADDR  = 0xa
SPI_VOL     = 0xb
SPI_AICTRL0 = 0xc
SPI_AICTRL1 = 0xd
SPI_AICTRL2 = 0xe
SPI_AICTRL3 = 0xf

SM_DIFF     = 0x01
SM_JUMP     = 0x02
SM_RESET    = 0x04
SM_OUTOFWAV = 0x08
SM_PDOWN    = 0x10
SM_TESTS    = 0x20
SM_STREAM   = 0x40
SM_PLUSV    = 0x80
SM_DACT     = 0x100
SM_SDIORD   = 0x200
SM_SDISHARE = 0x400
SM_SDINEW   = 0x800
SM_ADPCM    = 0x1000
SM_ADPCM_HP = 0x2000

class Player:
    def __init__(self, spi, xResetPin, dReqPin, xDCSPin, xCSPin, CSPin = None):
        self.xReset = digitalio.DigitalInOut(xResetPin)
        self.xReset.direction = digitalio.Direction.OUTPUT
        self.dReq = digitalio.DigitalInOut(dReqPin)
        self.dReq.direction = digitalio.Direction.INPUT
        self.xDCS = digitalio.DigitalInOut(xDCSPin)
        self.xDCS.direction = digitalio.Direction.OUTPUT
        self.xCS = digitalio.DigitalInOut(xCSPin)
        self.xCS.direction = digitalio.Direction.OUTPUT
        
        if CSPin:
            self.CS = digitalio.DigitalInOut(CSPin)
            self.CS.direction = digitalio.Direction.OUTPUT
            self.CS.value = True
        
        self.xDCS.value = True
        self.xCS.value = True
        self.spi = spi
        self.reset()
    
    def reset(self):
        self.xReset.value = False
        time.sleep(0.002); # It is a must, 2ms
        self.xCS.value = True
        self.xDCS.value = True
        self.xReset.value = True
        self.softReset()
        
    def waitForDREQ(self):
        while not self.dReq.value:
            pass
            
    def writeRegister(self, addressByte, value):
        while not self.spi.try_lock():
            pass
        try:
            self.xDCS.value = True
            self.waitForDREQ()
            self.xCS.value = False
            self.spi.configure(baudrate=SPI_BAUDRATE)
            self.spi.write(bytes([VS_WRITE_COMMAND, addressByte, value >> 8, value & 0xFF]))
            self.xCS.value = True
        finally:
            self.spi.unlock()

    def readRegister(self, addressByte):
        while not self.spi.try_lock():
            pass
        try:
            self.xDCS.value = True
            self.waitForDREQ()
            self.xCS.value = False
            self.spi.configure(baudrate=SPI_BAUDRATE)
            result = bytearray(4)
            self.spi.write_readinto(bytes([VS_READ_COMMAND, addressByte, 0xff, 0xff]), result)
            self.xCS.value = True
            return (result[2] << 8) | result[3]
        finally:
            self.spi.unlock()
    
    def writeData(self, buf):
        self.waitForDREQ()
        self.xDCS.value = False
        while not self.spi.try_lock():
            pass
        try:
            self.spi.write(buf)
        finally:
            self.spi.unlock()
        self.xDCS.value = True
        
    def setVolume(self, volume):
        """ Sets the volume to the given value (the range is 0 to 1.0). 
        Volume is not linear, so values below 0.75 are likely to be too quite to hear.
        
        Setting the volume to 0 will power down the analog part of the chip, 
        thus saving power.
        """
        self.setChannelVolume(volume, volume)
    
    def setChannelVolume(self, left, right):
        """ Sets the volume for each of the channels (left, right). The range is 0 to 1.0 """
        left = max(min(1.0, left), 0)
        right = max(min(1.0, right), 0)
        leftVal = round((1 - left) * 255)
        rightVal = round((1 - right) * 255)
        self.writeRegister(SPI_VOL, leftVal << 8 | rightVal)
    
    def softReset(self):
        """ Soft Reset of VS10xx """
        self.writeRegister(SPI_MODE, 0x0804) # Newmode, Reset, No L1-2
        time.sleep(0.002)
        self.waitForDREQ()
        self.writeRegister(SPI_HDAT0, 0xABAD)
        self.writeRegister(SPI_HDAT1, 0x1DEA)
        time.sleep(0.1)
        # Sanity check
        if self.readRegister(SPI_HDAT0) != 0xABAD or self.readRegister(SPI_HDAT1) != 0x1DEA:
            pass# raise RuntimeError("VS10xx Reset failed!")
  
        self.writeRegister(SPI_CLOCKF,0xC000)   # Set the clock
        self.writeRegister(SPI_AUDATA,0xBB81)   # Sample rate 48k, stereo
        self.writeRegister(SPI_BASS, 0x0055)    # Set accent
        self.setVolume(1)
  
        self.waitForDREQ()