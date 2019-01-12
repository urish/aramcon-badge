/*
 * Copyright (c) 2017 Linaro Limited
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr.h>
#include <device.h>
#include <gpio.h>
#include <led_strip.h>
#include <spi.h>
#include <misc/util.h>

#define LED_PORT LED0_GPIO_CONTROLLER
#define LED	LED0_GPIO_PIN

#define STRIP_NUM_LEDS 2
#define STRIP_DEV_NAME CONFIG_WS2812_STRIP_NAME

#define SLEEP_TIME 	500

static const struct led_rgb red = { .r = 0x20, .g = 0x00, .b = 0x00 };
static const struct led_rgb green = { .r = 0x0, .g = 0x20, .b = 0x0 };

struct led_rgb strip_colors[STRIP_NUM_LEDS];

void main(void) {
	struct device *strip;
	struct device *gpio;
	uint16_t counter = 0;

	gpio = device_get_binding(LED_PORT);
	gpio_pin_configure(gpio, LED, GPIO_DIR_OUT);


    strip = device_get_binding(STRIP_DEV_NAME);
    if (!strip) {
		LOG_ERR("LED strip device %s not found", STRIP_DEV_NAME);
		return;
	}

	while (1) {
		printk("Blink %d!\n", counter++);

		gpio_pin_write(gpio, LED, 0);
		strip_colors[0] = red;
		strip_colors[1] = green;
		led_strip_update_rgb(strip, strip_colors, STRIP_NUM_LEDS);
		k_sleep(SLEEP_TIME);

		gpio_pin_write(gpio, LED, 1);
		strip_colors[0] = green;
		strip_colors[1] = red;
		led_strip_update_rgb(strip, strip_colors, STRIP_NUM_LEDS);
		k_sleep(SLEEP_TIME);
	}
}