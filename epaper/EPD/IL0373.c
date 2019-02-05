/* IL0373 Driver, Copyright (C) 2019, Uri Shaked */
#include "IL0373.h"

#define EPD_SendLut(cmd, lut) \
  EPD_SendCommand(cmd);       \
  EPD_SendDataBuffer(lut, sizeof(lut));

void EPD_Reset(void) {
  DEV_Digital_Write(EPD_RST_PIN, 0);
  DEV_Delay_ms(20);
  DEV_Digital_Write(EPD_RST_PIN, 1);
  DEV_Delay_ms(200);
}

static void EPD_SendCommand(UBYTE commandRegister) {
  DEV_Digital_Write(EPD_DC_PIN, 0);
  DEV_Digital_Write(EPD_CS_PIN, 0);
  DEV_SPI_WriteByte(commandRegister);
  DEV_Digital_Write(EPD_CS_PIN, 1);
}

static void EPD_SendData(UBYTE data) {
  DEV_Digital_Write(EPD_DC_PIN, 1);
  DEV_Digital_Write(EPD_CS_PIN, 0);
  DEV_SPI_WriteByte(data);
  DEV_Digital_Write(EPD_CS_PIN, 1);
}

static void EPD_SendLutFull(UBYTE lutId, UBYTE b0, UBYTE b6, UBYTE b18) {
  UBYTE lutData[] = {
      b0, 0x08, 0x00, 0x00, 0x00, 0x02, b6, 0x28, 0x28, 0x00, 0x00, 0x01,
      b0, 0x14, 0x00, 0x00, 0x00, 0x01, b18, 0x12, 0x12, 0x00, 0x00, 0x01};
  UBYTE padding = (lutId == 0x20) ? 20 : 18;
  UBYTE i = 0;

  EPD_SendCommand(lutId);
  for (i = 0; i < sizeof(lutData); i++) {
    EPD_SendData(lutData[i]);
  }
  for (i = 0; i < padding; i++) {
    EPD_SendData(0);
  }
}

static void EPD_SendLutPartial(UBYTE lutId, UBYTE b0) {
  UBYTE padding = (lutId == 0x20) ? 38 : 36;
  EPD_SendCommand(lutId);
  EPD_SendData(b0);
  EPD_SendData(0x20);
  EPD_SendData(0x01);
  EPD_SendData(0x00);
  EPD_SendData(0x00);
  EPD_SendData(0x01);
  for (UBYTE i = 0; i < padding; i++) {
    EPD_SendData(0);
  }
}

static void EPD_SendDataBuffer(const UBYTE *data, UBYTE length) {
  DEV_Digital_Write(EPD_DC_PIN, 1);
  DEV_Digital_Write(EPD_CS_PIN, 0);
  for (UBYTE i = 0; i < length; i++) {
    DEV_SPI_WriteByte(data[i]);
  }
  DEV_Digital_Write(EPD_CS_PIN, 1);
}

void EPD_WaitUntilIdle(void) {
  while (DEV_Digital_Read(EPD_BUSY_PIN) == 1) { //LOW: idle, HIGH: busy
    DEV_Delay_ms(1);
  }
}

void EPD_PowerOn() {
  EPD_SendCommand(0x04);
  EPD_WaitUntilIdle();
}

void EPD_UpdateDisplay() {
  EPD_SendCommand(0x12);
  DEV_Delay_ms(50);
  EPD_WaitUntilIdle();
}

void EPD_Init(bool partial) {
  EPD_SendCommand(0x01); // Power setting
  EPD_SendData(0x03);
  EPD_SendData(0x00);
  EPD_SendData(0x2b);
  EPD_SendData(0x2b);
  EPD_SendData(0x03);
  EPD_SendCommand(0x06); // Boost soft start
  EPD_SendData(0x17);
  EPD_SendData(0x17);
  EPD_SendData(0x17);
  EPD_SendCommand(0x00); // Panel setting
  EPD_SendData(0xbf);
  EPD_SendData(0x0d);    // VCOM to 0V fast
  EPD_SendCommand(0x30); // PLL setting
  EPD_SendData(0x3a);    // 100HZ
  EPD_SendCommand(0x61); // Resolution
  EPD_SendData(EPD_WIDTH);
  EPD_SendData(EPD_HEIGHT >> 8);
  EPD_SendData(EPD_HEIGHT & 0xFF);
  EPD_SendCommand(0x82); // vcom_DC setting
  EPD_SendData(0x08);

  EPD_SendCommand(0X50); // VCOM and Data Interval
  EPD_SendData(partial ? 0x17 : 0x97);

  if (partial) {
    EPD_SendLutPartial(0x20, 0x00);
    EPD_SendLutPartial(0x21, 0x00);
    EPD_SendLutPartial(0x22, 0x80);
    EPD_SendLutPartial(0x23, 0x40);
    EPD_SendLutPartial(0x24, 0x00);
  } else {
    EPD_SendLutFull(0x20, 0x00, 0x60, 0x00);
    EPD_SendLutFull(0x21, 0x40, 0x90, 0xa0);
    EPD_SendLutFull(0x22, 0x40, 0x90, 0xa0);
    EPD_SendLutFull(0x23, 0x80, 0x90, 0x50);
    EPD_SendLutFull(0x24, 0x80, 0x90, 0x50);
  }

  EPD_SendCommand(0x4);
  EPD_WaitUntilIdle();
}

void EPD_Clear(void) {
  EPD_SendCommand(0x10);
  for (uint32_t i = 0; i < (uint32_t)EPD_WIDTH * (uint32_t)EPD_HEIGHT / 8; i++) {
    EPD_SendData(0xFF);
  }
  EPD_SendCommand(0x13);
  for (uint32_t i = 0; i < (uint32_t)EPD_WIDTH * (uint32_t)EPD_HEIGHT / 8; i++) {
    EPD_SendData(0xFF);
  }
  EPD_UpdateDisplay();
}

static void EPD_SetWindow(UWORD x, UWORD y, UWORD endX, UWORD endY) {
  EPD_SendCommand(0x90);
  EPD_SendData(x & 0xFF);
  EPD_SendData(((endX - 1) | 0x7) & 0xFF);
  EPD_SendData((y >> 8) & 0xFF);
  EPD_SendData(y & 0xFF);
  EPD_SendData(((endY - 1) >> 8) & 0xFF);
  EPD_SendData((endY - 1) & 0xFF);
  EPD_SendData(0x01);
}

void EPD_Display(const UBYTE *image) {
  EPD_SendCommand(0x13);
  for (uint32_t i = 0; i < (uint32_t)EPD_WIDTH * (uint32_t)EPD_HEIGHT / 8; i++) {
    EPD_SendData(image[i]);
  }
  EPD_UpdateDisplay();
}

void EPD_Sleep(void) {
  EPD_SendCommand(0x02);
  EPD_WaitUntilIdle();
}