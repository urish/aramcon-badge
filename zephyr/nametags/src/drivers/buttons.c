#include "buttons.h"
#include <device.h>
#include <gpio.h>

static struct device *gpio_bindings[BUTTON_COUNT] = {};
static const u32_t button_pin_map[BUTTON_COUNT] = {SW0_GPIO_PIN, SW1_GPIO_PIN, SW2_GPIO_PIN};
static const char *button_port_map[BUTTON_COUNT] = {SW0_GPIO_CONTROLLER, SW1_GPIO_CONTROLLER, SW2_GPIO_CONTROLLER};

void init_buttons()
{
  for (u32_t i = 0; i < BUTTON_COUNT; ++i)
  {
    gpio_bindings[i] = device_get_binding(button_port_map[i]);
    gpio_pin_configure(gpio_bindings[i], button_pin_map[i], GPIO_DIR_IN | SW0_GPIO_FLAGS);
  }
}

bool button_read(u32_t button_number)
{
  if (button_number < BUTTON_COUNT)
  {
    u32_t value;
    gpio_pin_read(
        gpio_bindings[button_number],
        button_pin_map[button_number],
        &value);
    return !value;
  }

  return false;
}