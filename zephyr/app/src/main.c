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
#include <stdio.h>
#include <misc/util.h>

#include "drivers/led.h"
#include "drivers/buttons.h"
#include "drivers/vibration_motor.h"
#include "drivers/neopixels.h"
#include "drivers/display.h"
#include "drivers/battery_voltage.h"
#include "ble_service.h"

#define SLEEP_TIME 	500

static u32_t button_val[BUTTON_COUNT] = {}; 

static uint16_t counter = 0;



void init_drivers()
{
	init_led();
	init_buttons();
	init_vibration_motor();
	init_neopixels();
	init_display();
	init_battery_voltage();
}

void read_inputs()
{
	for (u32_t i = 0; i < BUTTON_COUNT; ++i) {
		button_read(i, &button_val[i]);
	}

	sample_battery_voltage();
}

void update_neopixels()
{
	if (counter % 2) {
		write_neopixel(0, red);
		write_neopixel(1, green);
		write_neopixel(2, purple);
		write_neopixel(3, blue);
	} else {
		write_neopixel(0, green);
		write_neopixel(1, red);
		write_neopixel(2, blue);
		write_neopixel(3, purple);
	}
}

void update_display() {
	char buf[64];
	clear_display();
	print_line_to_display(0, "*** Badge Self-Test ***");

	sprintf(buf, "Battery: %.2fv", read_battery_voltage());
	print_line_to_display(1, buf);	

	if (ble_service_is_ready()) {
		sprintf(buf, "Name: %s", get_ble_name());
		print_line_to_display(2, buf);

	const char* mac = get_ble_mac();
		if (mac) {
			sprintf(buf, "Mac:  %s", mac);
			print_line_to_display(3, buf);
		}

		sprintf(buf, "BTNs L:%d M:%d R:%d", !button_val[2], !button_val[1], !button_val[0]);
		print_line_to_display(4, buf);

		const char* remote_mac = get_ble_remote_mac();
		if (remote_mac) {
			sprintf(buf, "Connected! %s", remote_mac);
			print_line_to_display(5, buf);
		}
	}

	// TODO display MP3 status
	// TODO display Flash status
	// TODO display accelerometer status (LIS2DH driver)

	char *status = "\\|/-";
	sprintf(buf, "%c", status[counter % sizeof(status)]);
	print_line_to_display(6 , buf);
}

void update_state()
{
	update_neopixels();
	update_display();
}

void write_outputs()
{
		// Control the vibrator, based on middle button
		write_vibration_motor(!button_val[1]);
		write_led(counter % 2);
		flush_neopixels();
		flush_display();
}

void main(void) {
	LOG_INF("Starting app...\n");

	init_drivers();
	init_ble_service();

	while (1) {
		read_inputs();
		update_state();
		write_outputs();
		k_sleep(SLEEP_TIME);
		counter++;
	}
}