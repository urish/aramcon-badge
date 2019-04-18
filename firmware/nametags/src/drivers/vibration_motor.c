#include "vibration_motor.h"
#include <device.h>
#include <gpio.h>

#define VIBRATION_MOTOR_PORT "GPIO_0"
#define VIBRATION_MOTOR_PIN 17

static struct device *gpio = NULL;

void init_vibration_motor()
{
	gpio = device_get_binding(VIBRATION_MOTOR_PORT);
	gpio_pin_configure(gpio, VIBRATION_MOTOR_PIN, GPIO_DIR_OUT);
}

void write_vibration_motor(u32_t value)
{
	gpio_pin_write(gpio, VIBRATION_MOTOR_PIN, value);
}

static void pulse_timer_handler(struct k_timer *dummy)
{
	write_vibration_motor(0);
}

K_TIMER_DEFINE(pulse_timer, pulse_timer_handler, NULL);

void pulse_vibration_motor(u32_t ms)
{
	write_vibration_motor(1);
	k_timer_start(&pulse_timer, ms, 0);
}