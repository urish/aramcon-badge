#include "led.h"
#include <device.h>
#include <gpio.h>
#include <pwm.h>

static struct device *gpio = NULL;
struct device *pwm_dev = NULL;

#define BREATHE_STEPS (32)
#define PERIOD (USEC_PER_SEC / 50U)

void init_led()
{
  gpio = device_get_binding(LED0_GPIO_CONTROLLER);
  gpio_pin_configure(gpio, LED0_GPIO_PIN, GPIO_DIR_OUT);

  pwm_dev = device_get_binding(DT_NORDIC_NRF_PWM_PWM_0_LABEL);
}

void write_led(u32_t value)
{
  gpio_pin_write(gpio, LED0_GPIO_PIN, value);
}

static void breathe_timer_handler(struct k_timer *dummy)
{
  static u16_t led_level = 0;
  static s8_t direction = 1;

  pwm_pin_set_usec(pwm_dev, 32 + LED0_GPIO_PIN, PERIOD, PERIOD / BREATHE_STEPS * led_level);

  led_level += direction;
  if (led_level == BREATHE_STEPS || led_level == 0) {
    direction *= -1;
  }
}

K_TIMER_DEFINE(breathe_timer, breathe_timer_handler, NULL);

void breathe_led(u32_t interval_ms)
{
  k_timer_start(&breathe_timer, K_MSEC(interval_ms / BREATHE_STEPS), K_MSEC(interval_ms / BREATHE_STEPS));
}