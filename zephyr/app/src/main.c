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
#include <display/cfb.h>
#include <misc/util.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <hal/nrf_saadc.h>
#include "led.h"
#include "buttons.h"
#include "vibration_motor.h"
#include "neopixels.h"

#define BATTERY_VOLTAGE_PIN NRF_SAADC_INPUT_AIN6

#define SLEEP_TIME 	500

#define DISPLAY_DRIVER		"DISPLAY"

static u32_t button_val[BUTTON_COUNT] = {}; 

static uint16_t counter = 0;
static struct device *display;
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

void update_display() {
	char buf[64];
	cfb_framebuffer_clear(display, true);
	cfb_print(display, "*** Badge Self-Test ***", 0, 0);

	sprintf(buf, "Battery: %.2fv", adc_val_to_voltage(adc_buffer[0]));
	cfb_print(display, buf, 0, 16);	

	if (bluetooth_ready) {
		sprintf(buf, "Name: %s", bt_get_name());
		cfb_print(display, buf, 0, 32);

		bt_addr_le_t addr;
		size_t count = 1;
		bt_id_get(&addr, &count);
		if (count) {
			sprintf(buf, "Mac:  %s", bluetooth_mac_to_str(&addr));
			cfb_print(display, buf, 0, 48);
		}

		sprintf(buf, "BTNs L:%d M:%d R:%d", !button_val[2], !button_val[1], !button_val[0]);
		cfb_print(display, buf, 0, 64);

		if (remote_device) {
			sprintf(buf, "Connected! %s", bluetooth_mac_to_str(remote_device));
			cfb_print(display, buf, 0, 80);
		}
	}

	// TODO display MP3 status
	// TODO display Flash status
	// TODO display accelerometer status (LIS2DH driver)

	char *status = "\\|/-";
	sprintf(buf, "%c", status[counter % sizeof(status)]);
	cfb_print(display, buf, 0, 96);

	cfb_framebuffer_finalize(display);
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

void main(void) {
	LOG_INF("Starting app...\n");

	init_led();
  
	init_buttons();

	init_vibration_motor();

	init_neopixels();
	
	// Display
	display = device_get_binding(DISPLAY_DRIVER);
	if (display == NULL) {
		LOG_ERR("Display device not found");
		return;
	}

	if (display_set_pixel_format(display, PIXEL_FORMAT_MONO10) != 0) {
		LOG_ERR("Failed to set required pixel format");
		return;
	}

	if (cfb_framebuffer_init(display)) {
		LOG_ERR("Framebuffer initialization failed!");
		return;
	}

	cfb_framebuffer_clear(display, true);

	display_blanking_off(display);

	u16_t rows;
	u8_t ppt;
	u8_t font_width;
	u8_t font_height;
	rows = cfb_get_display_parameter(display, CFB_DISPLAY_ROWS);
	ppt = cfb_get_display_parameter(display, CFB_DISPLAY_PPT);
	for (int idx = 0; idx < 42; idx++) {
		if (cfb_get_font_size(display, idx, &font_width, &font_height)) {
			break;
		}
		cfb_framebuffer_set_font(display, idx);
		LOG_INF("font width %d, font height %d\n",
		       font_width, font_height);
	}

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