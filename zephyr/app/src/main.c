/*
 * Badge Hardware Test App
 * Author: Uri Shaked
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
#include <display/cfb.h>
#include <misc/util.h>

#define LED_PORT LED0_GPIO_CONTROLLER
#define LED	LED0_GPIO_PIN

#define STRIP_NUM_LEDS 4
#define STRIP_DEV_NAME DT_WORLDSEMI_WS2812_0_LABEL

#define SLEEP_TIME 	500

#define DISPLAY_DRIVER		"SSD1673"

static const struct led_rgb red = { .r = 0x20, .g = 0x00, .b = 0x00 };
static const struct led_rgb green = { .r = 0x0, .g = 0x20, .b = 0x0 };
static const struct led_rgb blue = { .r = 0x0, .g = 0x0, .b = 0x20 };
static const struct led_rgb purple = { .r = 0x10, .g = 0x0, .b = 0x20 };

struct led_rgb strip_colors[STRIP_NUM_LEDS];

struct device *gpio;
struct device *gpio0;

void main(void) {
	struct device *strip;
	uint16_t counter = 0;

	gpio = device_get_binding(LED_PORT);
	gpio0 = device_get_binding("GPIO_0");
	gpio_pin_configure(gpio, LED, GPIO_DIR_OUT);

	strip = device_get_binding(STRIP_DEV_NAME);
	if (!strip) {
		LOG_ERR("LED strip device %s not found", STRIP_DEV_NAME);
		return;
	}

	strip_colors[0] = purple;
	strip_colors[1] = purple;
	strip_colors[2] = purple;
	strip_colors[3] = purple;
	led_strip_update_rgb(strip, strip_colors, STRIP_NUM_LEDS);

	printk("Start !\n");
	
	struct device *display = device_get_binding(DISPLAY_DRIVER);
	if (display == NULL) {
		printk("Device not found\n");
		return;
	}
	printk("Found!\n");

	if (display_set_pixel_format(display, PIXEL_FORMAT_MONO10) != 0) {
		printk("Failed to set required pixel format\n");
		return;
	}

	if (cfb_framebuffer_init(display)) {
		printk("Framebuffer initialization failed!\n");
		return;
	}

	cfb_framebuffer_clear(display, true);

	while (1) {
		cfb_print(display,
				      "Hello!",
				      0, 0);
		cfb_framebuffer_invert(display);
		cfb_framebuffer_finalize(display);
		printk("Blink %d!\n", counter++);

		gpio_pin_write(gpio, LED, 0);
		strip_colors[0] = red;
		strip_colors[1] = green;
		strip_colors[2] = purple;
		strip_colors[3] = blue;
		led_strip_update_rgb(strip, strip_colors, STRIP_NUM_LEDS);
		k_sleep(SLEEP_TIME);

		gpio_pin_write(gpio, LED, 1);
		strip_colors[0] = green;
		strip_colors[1] = red;
		strip_colors[2] = blue;
		strip_colors[3] = purple;
		led_strip_update_rgb(strip, strip_colors, STRIP_NUM_LEDS);
		k_sleep(SLEEP_TIME);
	}
}