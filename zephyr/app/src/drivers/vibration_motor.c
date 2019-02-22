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