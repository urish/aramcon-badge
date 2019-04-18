/*
 * Badge Sound Driver (VS1003B/VS1053B)
 * Author: Uri Shaked
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sound.h"
#include <device.h>
#include <gpio.h>
#include <spi.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(sound);

#define SOUND_XRESET_PORT "GPIO_0"
#define SOUND_XRESET_PIN 0
#define SOUND_DREQ_PORT "GPIO_0"
#define SOUND_DREQ_PIN 13
#define SOUND_xDCS_PORT "GPIO_0"
#define SOUND_xDCS_PIN 24
#define SOUND_xCS_PORT "GPIO_1"
#define SOUND_xCS_PIN 13

#define SOUND_SPI_DEVICE "SPI_0"

#define VS_WRITE_COMMAND (0x02)
#define VS_READ_COMMAND (0x03)

#define SPI_MODE (0x0)
#define SPI_STATUS (0x1)
#define SPI_BASS (0x2)
#define SPI_CLOCKF (0x3)
#define SPI_DECODE_TIME (0x4)
#define SPI_AUDATA (0x5)
#define SPI_WRAM (0x6)
#define SPI_WRAMADDR (0x7)
#define SPI_HDAT0 (0x8)
#define SPI_HDAT1 (0x9)
#define SPI_AIADDR (0xa)
#define SPI_VOL (0xb)
#define SPI_AICTRL0 (0xc)
#define SPI_AICTRL1 (0xd)
#define SPI_AICTRL2 (0xe)
#define SPI_AICTRL3 (0xf)

#define SM_DIFF (0x01)
#define SM_JUMP (0x02)
#define SM_RESET (0x04)
#define SM_OUTOFWAV (0x08)
#define SM_PDOWN (0x10)
#define SM_TESTS (0x20)
#define SM_STREAM (0x40)
#define SM_PLUSV (0x80)
#define SM_DACT (0x100)
#define SM_SDIORD (0x200)
#define SM_SDISHARE (0x400)
#define SM_SDINEW (0x800)
#define SM_ADPCM (0x1000)
#define SM_ADPCM_HP (0x2000)

#define SOUND_READY_TIMEOUT (500) // ms

struct device *reset_gpio = NULL;
struct device *command_data_gpio = NULL;
struct device *dreq_gpio = NULL;
static struct device *spi = NULL;

static struct spi_cs_control cs_control = {
    .gpio_dev = NULL,
    .gpio_pin = SOUND_xCS_PIN,
    .delay = 0,
};
static const struct spi_config config = {
    .frequency = 250000,
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE,
    .slave = 0,
    .cs = &cs_control,
};

static uint16_t vs1053_read(uint8_t addr)
{
  u8_t cmd[2] = {VS_READ_COMMAND, addr};
  u8_t data[2] = {0, 0};
  struct spi_buf buf[2] = {
      {
          .buf = cmd,
          .len = sizeof(cmd),
      },
      {
          .buf = data,
          .len = sizeof(data),
      },
  };
  const struct spi_buf_set tx = {.buffers = buf, .count = 1};
  const struct spi_buf_set rx = {.buffers = buf, .count = 2};

  int ret = spi_transceive(spi, &config, &tx, &rx);
  if (!ret)
  {
    LOG_ERR("SPI transfer failed: %d", ret);
  }

  return (data[0] << 8) | data[1];
}

static void vs1053_write(uint8_t addr, uint16_t data)
{
  uint8_t buf[4] = {VS_WRITE_COMMAND, addr, data >> 8, data & 0xFF};
  const struct spi_buf write_buf = {
      .buf = &buf,
      .len = sizeof(buf),
  };
  const struct spi_buf_set tx = {
      .buffers = &write_buf,
      .count = 1,
  };

  int ret = spi_write(spi, &config, &tx);
  if (!ret)
  {
    LOG_ERR("SPI write failed: %d", ret);
  }
}

void vs1053_send_data(u8_t *buf, u8_t len)
{
  struct spi_buf data_buf = {
      .buf = buf,
      .len = len,
  };
  const struct spi_buf_set tx = {
      .buffers = &data_buf,
      .count = 1,
  };

  gpio_pin_write(command_data_gpio, SOUND_xDCS_PIN, 0);
  for (uint16_t i = 0; i < len; i += 32)
  {
    data_buf.buf = &buf[i];
    data_buf.len = MIN(32, len - i);

    int ret = spi_write(spi, &config, &tx);
    if (!ret)
    {
      LOG_ERR("SPI write failed: %d", ret);
    }
  }
  gpio_pin_write(command_data_gpio, SOUND_xDCS_PIN, 1);
}

static bool vs1053_is_ready()
{
  u32_t ready = 0;
  gpio_pin_read(dreq_gpio, SOUND_DREQ_PIN, &ready);
  return ready;
}

static bool vs1053_wait_for_ready()
{
  for (u32_t i = 0; i < SOUND_READY_TIMEOUT; i++)
  {
    if (vs1053_is_ready())
    {
      return true;
    }
    k_sleep(1);
  }

  return false;
}

void vs1053_soft_reset(void)
{
  vs1053_write(SPI_MODE, SM_SDINEW | SM_RESET);
  k_sleep(2);
  vs1053_wait_for_ready();
  vs1053_write(SPI_HDAT0, 0xABAD);
  vs1053_write(SPI_HDAT1, 0x1DEA);
  k_sleep(100);
  bool sanity_pass = vs1053_read(SPI_HDAT0) == 0xABAD && vs1053_read(SPI_HDAT1) == 0x1DEA;
  if (!sanity_pass)
  {
    LOG_WRN("VS1053 Sanity failed!");
  }

  vs1053_write(SPI_CLOCKF, 0xC000);
  vs1053_write(SPI_AUDATA, 0xBB81);
  vs1053_write(SPI_BASS, 0x0055);
  vs1053_write(SPI_VOL, 0x4040);
  vs1053_wait_for_ready();
}

void vs1053_set_volume(uint8_t left, uint8_t right)
{
  uint16_t v = (left << 8) | right;
  vs1053_write(SPI_VOL, v);
}

void vs1053_reset()
{
  gpio_pin_write(reset_gpio, SOUND_XRESET_PIN, 0);
  k_sleep(100);
  gpio_pin_write(reset_gpio, SOUND_XRESET_PIN, 1);
  k_sleep(100);

  vs1053_soft_reset();
  k_sleep(100);

  vs1053_write(SPI_CLOCKF, 0x6000);

  vs1053_set_volume(0, 0);
}

void init_sound()
{
  reset_gpio = device_get_binding(SOUND_XRESET_PORT);
  dreq_gpio = device_get_binding(SOUND_DREQ_PORT);
  command_data_gpio = device_get_binding(SOUND_xDCS_PORT);
  cs_control.gpio_dev = device_get_binding(SOUND_xCS_PORT);
  gpio_pin_configure(reset_gpio, SOUND_XRESET_PIN, GPIO_DIR_OUT);
  gpio_pin_configure(dreq_gpio, SOUND_DREQ_PIN, GPIO_DIR_IN);
  gpio_pin_configure(command_data_gpio, SOUND_xDCS_PIN, GPIO_DIR_OUT);
  gpio_pin_configure(cs_control.gpio_dev, SOUND_xCS_PIN, GPIO_DIR_OUT);

  spi = device_get_binding(SOUND_SPI_DEVICE);
  if (spi == NULL)
  {
    LOG_ERR("SPI device not found");
    return;
  }

  gpio_pin_write(command_data_gpio, SOUND_xDCS_PIN, 1);
  gpio_pin_write(cs_control.gpio_dev, SOUND_xCS_PIN, 1);

  // Reset the chip
  gpio_pin_write(reset_gpio, SOUND_XRESET_PIN, 0);
  k_sleep(100);
  gpio_pin_write(reset_gpio, SOUND_XRESET_PIN, 1);
  k_sleep(100);
}

bool sound_sanity(u32_t value)
{
  LOG_INF("Sound sanity running...");
  vs1053_write(SPI_MODE, SM_SDINEW | SM_RESET);
  k_sleep(2);
  vs1053_wait_for_ready();
  vs1053_write(SPI_HDAT0, 0xABAD);
  vs1053_write(SPI_HDAT1, 0x1DEA);
  k_sleep(100);
  bool pass = vs1053_read(SPI_HDAT0) == 0xABAD && vs1053_read(SPI_HDAT1) == 0x1DEA;
  LOG_INF("Sound sanity %s: %04x, %04x...", pass ? "PASS" : "FAIL", vs1053_read(SPI_HDAT0), vs1053_read(SPI_HDAT1));
  return pass;
}

void vs1053_test(uint8_t n, uint16_t ms)
{
  vs1053_reset();
  uint16_t mode = vs1053_read(SPI_MODE);
  mode |= 0x0020;
  vs1053_write(SPI_MODE, mode);

  vs1053_wait_for_ready();

  char buf1[] = {0x53, 0xEF, 0x6E, n, 0x00, 0x00, 0x00, 0x00};
  vs1053_send_data(buf1, sizeof(buf1));

  k_sleep(ms);

  char buf2[] = {0x45, 0x78, 0x69, 0x74, 0x00, 0x00, 0x00, 0x00};
  vs1053_send_data(buf2, sizeof(buf2));
}
