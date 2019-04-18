#include <logging/log.h>
LOG_MODULE_REGISTER(buttons);

#include <device.h>
#include <gpio.h>

#include "buttons.h"

#define KEYBOARD_QUEUE_SIZE (16)
#define BUTTON_DEBOUNCE_MS (10)

static struct device *gpio_bindings[BUTTON_COUNT] = {};
static const u32_t button_pin_map[BUTTON_COUNT] = {SW0_GPIO_PIN, SW1_GPIO_PIN, SW2_GPIO_PIN};
static const char *button_port_map[BUTTON_COUNT] = {SW0_GPIO_CONTROLLER, SW1_GPIO_CONTROLLER, SW2_GPIO_CONTROLLER};
static struct gpio_callback button_cb[BUTTON_COUNT];
static u32_t button_last_event[BUTTON_COUNT] = {0};

struct k_msgq keyboard_event_queue;
static char __aligned(4) keyboard_event_queue_buffer[KEYBOARD_QUEUE_SIZE * sizeof(struct keyboard_event)];

void button_pressed(struct device *dev, struct gpio_callback *cb, u32_t pins)
{
  struct keyboard_event event;

  for (u8_t i = 0; i < BUTTON_COUNT; ++i)
  {
    if ((dev == gpio_bindings[i]) && (pins & BIT(button_pin_map[i])))
    {
      // debounce
      s32_t delta = k_uptime_get_32() - button_last_event[i];
      if (delta >= 0 && delta < BUTTON_DEBOUNCE_MS) {
        LOG_INF("Button debounce: %d", i);
        continue;
      }
      button_last_event[i] = k_uptime_get_32();

      event.button = i;
      event.pressed = button_read(i);
      LOG_INF("Dispatch button event, button=%d, state=%d", event.button, event.pressed);
      while (k_msgq_put(&keyboard_event_queue, &event, K_NO_WAIT) != 0) {
        /* message queue is full: purge old data & try again */
        k_msgq_purge(&keyboard_event_queue);
      }
    }
  }
}

void init_buttons()
{
  k_msgq_init(&keyboard_event_queue, keyboard_event_queue_buffer, sizeof(struct keyboard_event), KEYBOARD_QUEUE_SIZE);

  for (u8_t i = 0; i < BUTTON_COUNT; ++i)
  {
    gpio_bindings[i] = device_get_binding(button_port_map[i]);
    gpio_pin_configure(gpio_bindings[i], button_pin_map[i],
                       GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE | GPIO_INT_DOUBLE_EDGE 
                       | GPIO_INT_ACTIVE_LOW | SW0_GPIO_FLAGS);

    gpio_init_callback(&button_cb[i], button_pressed, BIT(button_pin_map[i]));
    gpio_add_callback(gpio_bindings[i], &button_cb[i]);
    gpio_pin_enable_callback(gpio_bindings[i], button_pin_map[i]);
  }
}

bool button_read(u8_t button_number)
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