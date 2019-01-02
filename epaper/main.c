#include <nrf.h>
#include <stdio.h>

#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_log.h"

#include "DEV_Config.h"
#include "EPD_1in54.h"
#include "GUI_Paint.h"
#include "ImageData.h"

#define SCK_PIN NRF_GPIO_PIN_MAP(0, 1)
#define MOSI_PIN NRF_GPIO_PIN_MAP(1, 10)

#define LED_PIN NRF_GPIO_PIN_MAP(1, 11)

const nrf_drv_spi_t m_spi_master_0 = NRF_DRV_SPI_INSTANCE(0);

uint32_t epaper_init() {
  uint32_t err_code;
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.frequency = NRF_SPI_FREQ_250K;
  spi_config.mosi_pin = MOSI_PIN;
  spi_config.sck_pin = SCK_PIN;

  err_code = nrf_drv_spi_init(&m_spi_master_0, &spi_config, NULL, NULL);
  VERIFY_SUCCESS(err_code);
  nrf_gpio_cfg_output(EPD_CS_PIN);
  nrf_gpio_cfg_output(EPD_RST_PIN);
  nrf_gpio_cfg_output(EPD_DC_PIN);
  nrf_gpio_cfg_input(EPD_BUSY_PIN, GPIO_PIN_CNF_PULL_Disabled);
}

void main(void) {
  nrf_gpio_cfg_output(LED_PIN);

  epaper_init();

  EPD_Init(lut_full_update);
  EPD_Clear();

  UBYTE image[((EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1)) * EPD_HEIGHT];

  Paint_NewImage(image, EPD_WIDTH, EPD_HEIGHT, 270, WHITE);
  Paint_SelectImage(image);
  Paint_Clear(WHITE);

  Paint_DrawString_EN(5, 45, "   Hello", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(5, 85, "  AramCon!", &Font24, WHITE, BLACK);
  EPD_Display(image);
  DEV_Delay_ms(2000);

  EPD_Display(logo_image);

  do {
    nrf_gpio_pin_write(LED_PIN, 0);
    nrf_delay_ms(250);
    nrf_gpio_pin_write(LED_PIN, 1);
    nrf_delay_ms(250);
  } while (1);
}