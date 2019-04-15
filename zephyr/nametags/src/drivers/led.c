#include "led.h"
#include <device.h>
#include <gpio.h>

static struct device *gpio = NULL;

void init_led()
{
  gpio = device_get_binding(LED0_GPIO_CONTROLLER);
  gpio_pin_configure(gpio, LED0_GPIO_PIN, GPIO_DIR_OUT);
}

void write_led(u32_t value)
{
  gpio_pin_write(gpio, LED0_GPIO_PIN, value);
}