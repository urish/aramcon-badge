#define BUTTON0_PORT SW0_GPIO_CONTROLLER
#define BUTTON0 SW0_GPIO_PIN
#define BUTTON1_PORT SW0_GPIO_CONTROLLER
#define BUTTON1 SW1_GPIO_PIN
#define BUTTON2_PORT SW0_GPIO_CONTROLLER
#define BUTTON2 SW2_GPIO_PIN

#include "buttons.h"
#include <device.h>
#include <gpio.h>

static struct device *gpio_bindings[BUTTON_COUNT]  = {};
static const u32_t button_pin_map[BUTTON_COUNT] = {BUTTON0, BUTTON1, BUTTON2};
static const const char *button_port_map[BUTTON_COUNT] = {BUTTON0_PORT, BUTTON0_PORT, BUTTON0_PORT};
void init_buttons()
{
    for (u32_t i = 0; i < BUTTON_COUNT; ++i)
    {
        gpio_bindings[i] = device_get_binding(button_port_map[i]);
        gpio_pin_configure(gpio_bindings[i], button_pin_map[i],	GPIO_DIR_IN |  SW0_GPIO_FLAGS);
    }
}

void button_read(u32_t button_number, u32_t *value)
{
    if (button_number < 3) {
        gpio_pin_read(
            gpio_bindings[button_number],
            button_pin_map[button_number],
            value);
    }
}