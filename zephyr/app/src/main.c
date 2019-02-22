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
#include <adc.h>
#include <device.h>
#include <gpio.h>
#include <stdio.h>
#include <misc/util.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <hal/nrf_saadc.h>
#include "drivers/led.h"
#include "drivers/buttons.h"
#include "drivers/vibration_motor.h"
#include "drivers/neopixels.h"
#include "drivers/display.h"

#define BATTERY_VOLTAGE_PIN NRF_SAADC_INPUT_AIN6

#define SLEEP_TIME 	500

static u32_t button_val[BUTTON_COUNT] = {}; 

static uint16_t counter = 0;
static bool bluetooth_ready = false;
static const bt_addr_le_t *remote_device = NULL;
static int16_t adc_buffer[1] = {0};

static float adc_val_to_voltage(int16_t adc_val) {
		const u8_t adc_resolution = 10;
    const float adc_gain = 1/6.;
		const float adc_ref_voltage = 0.6f;
		if (adc_val < 0) {
			adc_val = 0;
		}
    return adc_val / ((adc_gain / (adc_ref_voltage)) * (1 << adc_resolution));
}

static char *bluetooth_mac_to_str(const bt_addr_le_t* addr) {
	static char buf[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(addr, buf, sizeof(buf));
	char *space = strchr(buf, ' ');
	if (space) {
		*space = 0;
	}
	return buf;
}

/* Bluetooth */
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
};

static void connected(struct bt_conn *conn, u8_t err) {
	if (err) {
		LOG_ERR("Failed to connect to %s (%u)\n", bluetooth_mac_to_str(bt_conn_get_dst(conn)), err);
		return;
	}

	remote_device = bt_conn_get_dst(conn);
	LOG_INF("Connected %s\n", bluetooth_mac_to_str(remote_device));
}

static void disconnected(struct bt_conn *conn, u8_t reason) {
	LOG_INF("Disconnected from %s (reason %u)\n", bluetooth_mac_to_str(remote_device), reason);
	remote_device = NULL;
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(int err) {
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return;
	}

	LOG_INF("Bluetooth initialized");

	bt_conn_cb_register(&conn_callbacks);

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return;
	}

	LOG_INF("Advertising successfully started\n");
	bluetooth_ready = true;
}

void update_display() {
	char buf[64];
	clear_display();
	print_line_to_display(0, "*** Badge Self-Test ***");

	sprintf(buf, "Battery: %.2fv", adc_val_to_voltage(adc_buffer[0]));
	print_line_to_display(1, buf);	

	if (bluetooth_ready) {
		sprintf(buf, "Name: %s", bt_get_name());
		print_line_to_display(2, buf);

		bt_addr_le_t addr;
		size_t count = 1;
		bt_id_get(&addr, &count);
		if (count) {
			sprintf(buf, "Mac:  %s", bluetooth_mac_to_str(&addr));
			print_line_to_display(3, buf);
		}

		sprintf(buf, "BTNs L:%d M:%d R:%d", !button_val[2], !button_val[1], !button_val[0]);
		print_line_to_display(4, buf);

		if (remote_device) {
			sprintf(buf, "Connected! %s", bluetooth_mac_to_str(remote_device));
			print_line_to_display(5, buf);
		}
	}

	// TODO display MP3 status
	// TODO display Flash status
	// TODO display accelerometer status (LIS2DH driver)

	char *status = "\\|/-";
	sprintf(buf, "%c", status[counter % sizeof(status)]);
	print_line_to_display(6 , buf);

	flush_display();
}

void main(void) {
	LOG_INF("Starting app...\n");

	init_led();
  
	init_buttons();

	init_vibration_motor();

	init_neopixels();
	
	// Display
	init_display();

	update_display();

	// ADC
	struct device *adc_dev = device_get_binding(DT_ADC_0_NAME);
	if (!adc_dev) {
		LOG_ERR("adc init failed :-(");
		return;
	}
	
	struct adc_channel_cfg adc_channel_cfg = {
		.gain             = ADC_GAIN_1_6,
		.reference        = ADC_REF_INTERNAL,
		.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10),
		.channel_id       = 0,
		.input_positive   = BATTERY_VOLTAGE_PIN,
	};

	u32_t ret = adc_channel_setup(adc_dev, &adc_channel_cfg);
	if (ret) {
		LOG_ERR("adc channel setup failed :-(");
		return;
	}

	const struct adc_sequence sequence = {
		.channels    = BIT(0),
		.buffer      = adc_buffer,
		.buffer_size = sizeof(adc_buffer),
		.resolution  = 10,
	};

	// Bluetooth
	int err = bt_enable(bt_ready);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return;
	}	

	while (1) {
		for (u32_t i = 0; i < BUTTON_COUNT; ++i) {
			button_read(i, &button_val[i]);
		}

		ret = adc_read(adc_dev, &sequence);
		if (ret) {
			LOG_ERR("adc read failed: %d", ret);
		}

		// Control the vibrator, based on middle button
		write_vibration_motor(!button_val[1]);

		write_led(counter % 2);

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
		update_neopixels();

		update_display();
		k_sleep(SLEEP_TIME);

		counter++;
	}
}