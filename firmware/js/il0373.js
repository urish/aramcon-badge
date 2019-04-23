/**
 * Espruino 2.9" E-Paper IL0373 Driver
 * 
 * Copyright (C) 2019, Uri Shaked
 * 
 * Released under the MIT License.
 */

const C = {
  CMD: 0,
  DATA: 1
};

const CMD = {
  PANEL_SETTING: 0x00,
  POWER_SETTING: 0x01,
  POWER_OFF: 0x02,
  POWER_OFF_SEQUENCE: 0x03,
  POWER_ON: 0x04,
  POWER_ON_MEASURE: 0x05,
  BOOSTER_SOFT_START: 0x06,
  DEEP_SLEEP: 0x07,
  DTM1: 0x10,
  DATA_STOP: 0x11,
  DISPLAY_REFRESH: 0x12,
  DTM2: 0x13,
  PDTM1: 0x14,
  PDTM2: 0x15,
  PDRF: 0x16,
  LUT1: 0x20,
  LUTWW: 0x21,
  LUTBW: 0x22,
  LUTWB: 0x23,
  LUTBB: 0x24,
  PLL: 0x30,
  CDI: 0x50,
  RESOLUTION: 0x61,
  VCM_DC_SETTING: 0x82,
  PARTIAL_WINDOW: 0x90,
  PARTIAL_IN: 0x91,
  PARTIAL_OUT: 0x92
};

class IL0373 {
  constructor(spi, cs, dc, rst, busy) {
    this.spi = spi;
    this.cs = cs;
    this.dc = dc;
    this.rst = rst;
    this.busy = busy;
    this.width = 128;
    this.height = 296;
    pinMode(cs, 'output');
    pinMode(dc, 'output');
    pinMode(rst, 'output');
    pinMode(busy, 'input');
  }

  _send(data, isData) {
    digitalWrite(this.dc, isData ? C.DATA : C.CMD);
    this.spi.write(data, this.cs);
  }

  reset() {
    return new Promise(resolve => {
      digitalWrite(this.rst, LOW);
      setTimeout(() => {
        digitalWrite(this.rst, HIGH);
        setTimeout(resolve, 20);
      }, 20);
    });
  }

  sendCommand(code, data) {
    this._send([code]);
    if (data != null) {
      this._send(data, true);
    }
  }

  wait() {
    return new Promise(resolve => {
      if (digitalRead(this.busy)) {
        resolve();
      }
      setWatch(resolve, this.busy, { debounce: 0, edge: 'rising' });
    });
  }

  suspend() {
    this.sendCommand(CMD.POWER_ON);
  }

  resume() {
    this.sendCommand(CMD.POWER_OFF);
  }

  _sendLut(lutId, b0, b6, b18) {
    const lut = new Uint8Array(lutId === CMD.LUT1 ? 44 : 42);
    lut.set([b0, 0x08, 0x00, 0x00, 0x00, 0x02], 0);
    lut.set([b6, 0x28, 0x28, 0x00, 0x00, 0x01], 6);
    lut.set([b0, 0x14, 0x00, 0x00, 0x00, 0x01], 12);
    lut.set([b18, 0x12, 0x12, 0x00, 0x00, 0x01], 18);
    this.sendCommand(lutId, lut);
  }

  _sendPartialLut(lutId, b0) {
    const lut = new Uint8Array(lutId === CMD.LUT1 ? 44 : 42);
    lut.set([b0, 0x20, 0x01, 0x00, 0x00, 0x01], 0);
    this.sendCommand(lutId, lut);
  }

  _powerOn() {
    this.sendCommand(CMD.POWER_ON);
    return this.wait();
  }

  init(isFull) {
    return this.reset().then(() => {
      this.sendCommand(CMD.POWER_SETTING, [0x03, 0x00, 0x2b, 0x2b, 0x03]);
      this.sendCommand(CMD.BOOSTER_SOFT_START, [0x17, 0x17, 0x17]);
      this.sendCommand(CMD.PANEL_SETTING, [0xbf, 0x0d]); // VCOM to 0V fast
      this.sendCommand(CMD.PLL, [0x3a]); // 100HZ
      this.sendCommand(CMD.RESOLUTION, [
        this.width,
        this.height >> 8,
        this.height & 0xff
      ]);
      this.sendCommand(CMD.VCM_DC_SETTING, [0x08]);
      this.sendCommand(CMD.CDI, [isFull ? 0x97 : 0x17]);

      if (isFull) {
        this._sendLut(CMD.LUT1, 0x00, 0x60, 0x00);
        this._sendLut(CMD.LUTWW, 0x40, 0x90, 0xa0);
        this._sendLut(CMD.LUTBW, 0x40, 0x90, 0xa0);
        this._sendLut(CMD.LUTWB, 0x80, 0x90, 0x50);
        this._sendLut(CMD.LUTBB, 0x80, 0x90, 0x50);
      } else {
        this._sendPartialLut(CMD.LUT1, 0x00);
        this._sendPartialLut(CMD.LUTWW, 0x00);
        this._sendPartialLut(CMD.LUTBW, 0x80);
        this._sendPartialLut(CMD.LUTWB, 0x40);
        this._sendPartialLut(CMD.LUTBB, 0x00);
      }

      return this._powerOn();
    });
  }

  clear() {
    const buf = new Uint8Array(this.width);
    const repeats = this.height / 8;
    buf.fill(0xff, 0, buf.length);

    this.sendCommand(CMD.DTM1);
    for (let i = 0; i < repeats; i++) {
      this._send(buf, true);
    }

    this.sendCommand(CMD.DTM2);
    for (let i = 0; i < repeats; i++) {
      this._send(buf, true);
    }

    return this.updateDisplay();
  }

  updateDisplay() {
    this.sendCommand(CMD.DISPLAY_REFRESH);
    return this.wait();
  }

  setPartialArea(x, y, w, h) {
    const x_end = x + w - 1;
    const y_end = y + h - 1;
    x &= 0xfff8;
    this.sendCommand(
      CMD.PARTIAL_WINDOW,
      new Uint8Array([
        x % 256,
        x_end % 256,
        y / 256,
        y % 256,
        y_end / 256,
        y_end % 256,
        0x01
      ])
    );
  }

  write(buf) {
    this.sendCommand(CMD.PARTIAL_IN);
    this.setPartialArea(0, 0, this.width, this.height);
    this.sendCommand(CMD.DTM2, buf);
    this.sendCommand(CMD.PARTIAL_OUT);
    return this.updateDisplay();
  }
}

/*

SPI1.setup({ mosi: 42, sck: D1, baud: 2000000 });
display = new IL0373(SPI1, D7, D12, D6, D26);

display.init(true).then(() => {
  const g = Graphics.createArrayBuffer(display.width, display.height, 1, {
    msb: true
  });
  g.setRotation(3); // -90 degrees
  g.setColor(1);
  g.setFontVector(32);
  g.drawString('Hello, World!', 16, 16);
  display.write(g.buffer);
});

*/
