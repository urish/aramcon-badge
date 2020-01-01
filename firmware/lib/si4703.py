# SI4703 RDS FM Receiver driver for CircuitPython
# Released under the MIT License

# Note:
# Some of the code was inspired by https://github.com/achilikin/RdSpi, for which the license
# is unclear - some of the source files specify GPLv2+, while others specify BSD.
#
# Copyright (c) 2019 Elad Luz and Uri Shaked

from time import sleep
from struct import pack, unpack
import adafruit_bus_device.i2c_device as i2c_device

DEBUG = False

SI4703_ADDR = 0x10
SI4703_MAGIC = 0x1242

# POWERCFG (0x2)
POWERCFG_DSMUTE      = 1 << 15
POWERCFG_DMUTE       = 1 << 14
POWERCFG_MONO        = 1 << 13
POWERCFG_RDSM        = 1 << 11
POWERCFG_SKMODE      = 1 << 10
POWERCFG_SEEKUP      = 1 << 9
POWERCFG_SEEK        = 1 << 8
POWERCFG_PWR_DISABLE = 1 << 6
POWERCFG_PWR_ENABLE  = 1 << 0

# CHANNEL (0x3)
CHANNEL_TUNE = 1 << 15
CHANNEL_CHAN = 0x003FF

# SYSCONF1 (0x4)
SYSCONF1_RDSIEN = 1 << 15
SYSCONF1_STCIEN = 1 << 14
SYSCONF1_RDS    = 1 << 12
SYSCONF1_DE     = 1 << 11
SYSCONF1_AGCD   = 1 << 10
SYSCONF1_BLNDADJ= 0x00C0
SYSCONF1_GPIO3  = 0x0030
SYSCONF1_GPIO2  = 0x000C
SYSCONF1_GPIO1  = 0x0003

# SYSCONF2 (0x05)
SYSCONF2_SEEKTH   = 0xFF00
SYSCONF2_BAND     = 0x00C0
SYSCONF2_SPACING  = 0x0030
SYSCONF2_VOLUME   = 0x000F
SYSCONF2_BAND0    = 0x0000 # 87.5 - 107 MHz (Europe, USA)
SYSCONF2_BAND1    = 0x0040 # 76-108 MHz (Japan wide band)
SYSCONF2_BAND2    = 0x0080 # 76-90 MHz (Japan)
SYSCONF2_SPACE50  = 0x0020 #  50 kHz spacing
SYSCONF2_SPACE100 = 0x0010 # 100 kHz (Europe, Japan)
SYSCONF2_SPACE200 = 0x0000 # 200 kHz (USA, Australia)

# SYSCONF3 (0x06)
SYSCONF3_SMUTER = 0xC000
SYSCONF3_SMUTEA = 0x3000
SYSCONF3_RDSPRF = 1 << 9
SYSCONF3_VOLEXT = 1 << 8
SYSCONF3_SKSNR =  0x00F0
SYSCONF3_SKCNT =  0x000F

# TEST1 (0x07)
TEST1_XOSCEN = 1 << 15
TEST1_AHIZEN = 1 << 14

# STATUSRSSI (0x0A)
STATUSRSSI_RDSR   = 1 << 15
STATUSRSSI_STC    = 1 << 14
STATUSRSSI_SFBL   = 1 << 13
STATUSRSSI_AFCRL  = 1 << 12
STATUSRSSI_RDSS   = 1 << 11
STATUSRSSI_BLERA  = 0x0600
STATUSRSSI_STEREO = 1 << 8
STATUSRSSI_RSSI   = 0x00FF

# READCHAN (0x0B)
READCHAN_BLERB = 0xC000
READCHAN_BLERC = 0x3000
READCHAN_BLERD = 0x0C00
READCHAN_RCHAN = 0x03FF

SI_BAND = [[8750, 10800], [7600, 10800], [7600, 9000]]
SI_SPACE = [20, 10, 5]

# Registers
REGS = [
    'DEVICEID', 
    'CHIPID', 
    'POWERCFG', 
    'CHANNEL', 
    'SYSCONF1', 
    'SYSCONF2', 
    'SYSCONF3', 
    'TEST1', 
    'TEST2',
    'BOOTCONF', 
    'STATUSRSSI', 
    'READCHAN', 
    'RDSA', 
    'RDSB', 
    'RDSC', 
    'RDSD'
]

def pretty_print_regs(registers, title=None):
    if type(registers) is not dict or not DEBUG:
        return False

    if title:
        print("[Si4703] {}:".format(title))

    for index, reg_name in enumerate(REGS):
        print('\t{:X} 0x{:04X}: {}'.format(index, registers[reg_name], reg_name))

class SI4703:
    """SI4703 RDS FM receiver driver. Parameters:
     - i2c: The I2C bus connected to the board.
     - reset: A DigitalInOut instance connected to the board's reset line
     - address (optional): The I2C address if it has been changed.
     - channel (optional): Channel to tune to initially. Set to the desired FM frequency, e.g. 91.8
    """

    def __init__(self, i2c, resetpin, address=SI4703_ADDR, channel=95):
        self._resetpin = resetpin
        self._channel = channel
        self._resetpin.switch_to_output(True)
        self._device = i2c_device.I2CDevice(i2c, address)

    def read_registers(self):
        registers = {}
        fm_registers_unsorted = bytearray(32)
        with self._device as i2c:
            i2c.readinto(fm_registers_unsorted)        
        
        for i in range(0, 32, 2):
            # Each register is a short, and output starts from 0x0A, wraps around at 0x10, don't ask.
            register_index = int(0x0A + (i / 2))
            if register_index >= 0x10:
                register_index -= 0x10
            
            registers[REGS[register_index]] = unpack('>H', fm_registers_unsorted[i:i+2])[0]
        
        return registers

    def write_registers(self, registers, count = None):
        if type(registers) is not dict:
            return False

        if not set(REGS).issubset(set(registers.keys())):
            return False
        
        # Only these are allowed to be set and at this specific order
        registers_batch = pack('>HHHHHH', registers['POWERCFG'], 
                                        registers['CHANNEL'],
                                        registers['SYSCONF1'],
                                        registers['SYSCONF2'],
                                        registers['SYSCONF3'],
                                        registers['TEST1'])
        if count:
            registers_batch = registers_batch[:count * 2]
        if DEBUG:
            from binascii import hexlify
            print('[Si4703] Write: {}'.format(hexlify(registers_batch)))

        with self._device as i2c:
            i2c.write(registers_batch, stop=True)

        return True

    def power_up(self):
        registers = self.read_registers()
        
        if registers['DEVICEID'] != SI4703_MAGIC:
            raise RuntimeError('Invalid device ID for SI4703: {:%04X}'.format(register['DEVICEID']))

        # set only ENABLE bit, leaving device muted
        registers['POWERCFG'] = POWERCFG_PWR_ENABLE
        # by default we allow wrap by not setting SKMODE
        # registers['POWERCFG'] |= POWERCFG_SKMODE
        registers['SYSCONF1'] |= SYSCONF1_RDS # enable RDS

        # Set mono/stereo blend adjust to default 0 or 31-49 RSSI
        registers['SYSCONF1'] &= ~SYSCONF1_BLNDADJ
        # set different BLDADJ if needed
        # x00=31-49, x40=37-55, x80=19-37,xC0=25-43 RSSI
        #	registers['SYSCONF1'] |= 0x0080
        # enable RDS High-Performance Mode
        registers['SYSCONF3'] |= SYSCONF3_RDSPRF
        registers['SYSCONF1'] |= SYSCONF1_DE # set 50us De-Emphasis for Europe, skip for USA
        # select general Europe/USA 87.5 - 108 MHz band
        registers['SYSCONF2'] = SYSCONF2_BAND0 | SYSCONF2_SPACE100 # 100kHz channel spacing for Europe

        # apply recommended seek settings for "most stations"
        registers['SYSCONF2'] &= ~SYSCONF2_SEEKTH
        registers['SYSCONF2'] |= 0x0C00 # SEEKTH 12
        registers['SYSCONF3'] &= 0xFF00
        registers['SYSCONF3'] |= 0x004F # SKSNR 4, SKCNT 15

        self.write_registers(registers)
        
        # Recommended powerup time
        sleep(0.110)

        return True

    def reset(self):
        self._resetpin.value = False
        sleep(0.01) # 10ms
        self._resetpin.value = True
        sleep(0.001) # 1ms

        registers = self.read_registers()
        pretty_print_regs(registers, "After reset")

        registers['TEST1'] |= TEST1_XOSCEN
        self.write_registers(registers)

        sleep(0.5) # Recommended delay

        registers = self.read_registers()
        pretty_print_regs(registers, "Oscillator enabled")

        # The only way to reliable start the device is to powerdown and powerup
        # Just powering up does not work for me after cold start
        registers['POWERCFG'] = POWERCFG_PWR_DISABLE | POWERCFG_PWR_ENABLE
        self.write_registers(registers, 1)
        sleep(0.110)

        self.power_up()

        registers = self.read_registers()
        pretty_print_regs(registers, "Power up")

        # default channel
        self.channel = self._channel

    @property
    def channel(self):
        return self._channel
    
    @channel.setter
    def channel(self, value):
        if type(value) not in [int, float]:
            return False

        freq = round(value * 100)

        registers = self.read_registers()

        pretty_print_regs(registers, "Before setting a channel")

        band = (registers['SYSCONF2'] >> 6) & 0x03
        space = (registers['SYSCONF2'] >> 4) & 0x03

        if freq < SI_BAND[band][0]:
            freq = SI_BAND[band][0]
        if freq > SI_BAND[band][1]:
            freq = SI_BAND[band][1]

        nchan = (freq - SI_BAND[band][0]) // SI_SPACE[space];

        registers['CHANNEL'] &= ~CHANNEL_CHAN;
        registers['CHANNEL'] |= nchan & CHANNEL_CHAN;
        registers['CHANNEL'] |= CHANNEL_TUNE;

        self.write_registers(registers)

        sleep(0.01)
        pretty_print_regs(registers, "After setting a channel")

        # Poll to see if STC is set
        while (self.read_registers()['STATUSRSSI'] & STATUSRSSI_STC) == 0:
            sleep(0.01)

        registers = self.read_registers()
        registers['CHANNEL'] &= ~CHANNEL_TUNE # Clear the tune after a tune has completed
        self.write_registers(registers)

        # Wait for the si4703 to clear the STC as well
        # Poll to see if STC is set
        while (self.read_registers()['STATUSRSSI'] & STATUSRSSI_STC) != 0:
            sleep(0.01)

        self._channel = value

    @property
    def volume(self):
        registers = self.read_registers()
        volume = registers['SYSCONF2'] & 0xF
        if registers['SYSCONF3'] & SYSCONF3_VOLEXT:
            volume += 15
        return volume

    @volume.setter
    def volume(self, volume):
        registers = self.read_registers()
        
        registers['SYSCONF3'] &= ~SYSCONF3_VOLEXT
        if volume > 15:
            registers['SYSCONF3'] |= SYSCONF3_VOLEXT
            volume -= 15;

        if volume:
            registers['POWERCFG'] |= POWERCFG_DSMUTE | POWERCFG_DMUTE # unmute
        else:
            registers['POWERCFG'] &= ~(POWERCFG_DSMUTE | POWERCFG_DMUTE) # mute

        # Clear volume bits
        registers['SYSCONF2'] &= 0xFFF0
        # Set new volume
        registers['SYSCONF2'] |= volume & 0xF
        self.write_registers(registers)
