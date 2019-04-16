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
#include <display/cfb.h>
#include <misc/util.h>
#include <flash.h>
#include <nvs/nvs.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "drivers/battery_voltage.h"
#include "drivers/buttons.h"
#include "drivers/led.h"
#include "drivers/neopixels.h"
#include "drivers/vibration_motor.h"
#include "agenda.h"
#include "colorgame.h"
#include "qrcode.h"
#include "scanner.h"
#include "logo.h"

#define SLEEP_TIME 	500

#define DISPLAY_DRIVER		"DISPLAY"

#define BT_UUID_DISP       BT_UUID_DECLARE_16(0xfeef)
#define BT_UUID_DISP_DATA  BT_UUID_DECLARE_16(0xfeee)

#define BT_UUID_BATT_VOLTAGE BT_UUID_DECLARE_128(0x9a, 0x24, 0xd0, 0xc6, 0xb0, 0x1f, 0x44, 0x4d, 0x85, 0xff, 0x59, 0x12, 0x13, 0x41, 0xad, 0x34)

#define DISPLAY_WIDTH 296
#define DISPLAY_HEIGHT 128

#define DISPLAY_ID 2
#define DISPLAY2_ID 3

struct device *display;
static bool bluetooth_ready = false;
static bool display_set = false;
static const bt_addr_le_t *remote_device = NULL;


static const struct display_buffer_descriptor desc = {
.buf_size = DISPLAY_WIDTH * DISPLAY_HEIGHT / 8,
.width = DISPLAY_WIDTH,
.height = DISPLAY_HEIGHT,
.pitch = DISPLAY_WIDTH,
};

static bool is_advertising = false;
static struct nvs_fs fs;

void init_storage()
{
	int rc = 0;
	struct flash_pages_info info;
    fs.offset = DT_FLASH_AREA_STORAGE_OFFSET;
	rc = flash_get_page_info_by_offs(device_get_binding(DT_FLASH_DEV_NAME),
					 fs.offset, &info);
	if (rc) {
		LOG_ERR("Unable to get page info");
	}
	fs.sector_size = info.size;
	fs.sector_count = 3U;

	rc = nvs_init(&fs, DT_FLASH_DEV_NAME);
	if (rc) {
		LOG_ERR("Flash Init failed\n");
	}
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
	cfb_print(display, "*** Badge READY! ***", 0, 0);
	
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

		if (remote_device) {
			sprintf(buf, "Connected! %s", bluetooth_mac_to_str(remote_device));
			cfb_print(display, buf, 0, 80);
		}
	}

	cfb_framebuffer_finalize(display);
}

/* Bluetooth */
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
  BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0xef, 0xfe),
};

static void connected(struct bt_conn *conn, u8_t err) {
	if (err) {
		LOG_ERR("Failed to connect to %s (%u)\n", bluetooth_mac_to_str(bt_conn_get_dst(conn)), err);
		return;
	}

	remote_device = bt_conn_get_dst(conn);
	LOG_INF("Connected %s\n", bluetooth_mac_to_str(remote_device));

  update_display();
}

u8_t display_buf[5000] = {0};
bool display_dirty = false;

static void disconnected(struct bt_conn *conn, u8_t reason) {
	LOG_INF("Disconnected from %s (reason %u)\n", bluetooth_mac_to_str(remote_device), reason);
	remote_device = NULL;
	nvs_delete(&fs, DISPLAY_ID);
	nvs_delete(&fs, DISPLAY2_ID);
	nvs_write(&fs, DISPLAY_ID, display_buf, 2000);
	nvs_write(&fs, DISPLAY2_ID, display_buf + 2000, 3000);
}


static void set_pixel(uint16_t x, uint16_t y, bool value) {
	uint16_t bitmapOffs = (y / 8) * DISPLAY_WIDTH + x;
	uint8_t bit = 7 - (y % 8);
	if (value) {
		display_buf[bitmapOffs] |= 1 << bit;
	} else {
		display_buf[bitmapOffs] &= ~(1 << bit);
	}
}

#define QR_VERSION (3)

static void draw_qr(const char *text, uint16_t x, uint16_t y, uint8_t scale) {
	QRCode qrcode;
	uint8_t qrcode_data[qrcode_getBufferSize(QR_VERSION)];
	qrcode_initText(&qrcode, qrcode_data, QR_VERSION, ECC_QUARTILE, text);

	for (uint8_t qr_y = 0; qr_y < qrcode.size; qr_y++) {
		for (uint8_t qr_x = 0; qr_x < qrcode.size; qr_x++) {
			for (uint8_t sx = 0; sx < scale; sx++) {
				for (uint8_t sy = 0; sy < scale; sy++) {
					set_pixel(x + qr_x * scale + sx, y + qr_y * scale + sy, !qrcode_getModule(&qrcode, qr_x, qr_y));
				}
			}
		}
	}
}

struct write_disp_data_req {
	u16_t pos:15;
  u8_t dirty:1;
  u8_t data[18];
} __packed;

static ssize_t write_disp_data(struct bt_conn *conn,
				const struct bt_gatt_attr *attr,
				const void *buf, u16_t len, u16_t offset,
				u8_t flags)
{
	const struct write_disp_data_req *req = buf;

	if (!len && len < 1) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

  if (len == 1) {
    // This means: clear screen
    memset(&display_buf, *((u8_t*)buf), sizeof(display_buf));
  } else {
    memcpy(&display_buf[req->pos], req->data, len - 2);
    display_dirty = req->dirty;
  }

  display_set = true;

	return len;
}

static ssize_t read_battery_voltage_cb(struct bt_conn *conn,
				const struct bt_gatt_attr *attr,
				void *buf, u16_t len, u16_t offset)
{
	char *value = attr->user_data;

	sample_battery_voltage();
	sprintf(value, "%.2f", read_battery_voltage());

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, strlen(value));
}

char disp_char_buf[20] = {0};
char battery_voltage_buf[20] = {0};

static struct bt_gatt_attr disp_attrs[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_DISP),

	BT_GATT_CHARACTERISTIC(BT_UUID_DISP_DATA,
			       BT_GATT_CHRC_WRITE | BT_GATT_CHRC_INDICATE,
			       BT_GATT_PERM_WRITE, NULL, write_disp_data,
			       &disp_char_buf),
	BT_GATT_CUD("Display Buffer", BT_GATT_PERM_READ),

	BT_GATT_CHARACTERISTIC(BT_UUID_BATT_VOLTAGE,
			       BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, read_battery_voltage_cb, NULL,
			       &battery_voltage_buf),
	BT_GATT_CUD("Battery Voltage", BT_GATT_PERM_READ),
};

static struct bt_gatt_service disp_svc = BT_GATT_SERVICE(disp_attrs);

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static const char* get_name_suffix() {
	static char name_suffix[7];
	bt_addr_le_t addr;
	size_t count = 1;
	bt_id_get(&addr, &count);
	sprintf(name_suffix, "%02x%02x%02x", addr.a.val[3], addr.a.val[4], addr.a.val[5]);
	return name_suffix;
}

static const char* get_qr_url() {
	static char url[36];
	sprintf(url, "https://go.urish.org/AC?b=%s", get_name_suffix());
	return url;
}

static void bt_ready(int err) {
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return;
	}

	LOG_INF("Bluetooth initialized");

	bt_conn_cb_register(&conn_callbacks);
	bt_gatt_service_register(&disp_svc);

	char deviceName[32];
	sprintf(deviceName, "BADGE-%s", get_name_suffix());
	err = bt_set_name(deviceName);
	if (err) {
		LOG_ERR("Failed to set device name (err %d)", err);
		return;
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return;
	}
	is_advertising = true;

	scanner_init();

	LOG_INF("Advertising successfully started\n");
	bluetooth_ready = true;
}

void init_drivers(void) {
	init_buttons();
	init_battery_voltage();
	init_led();
	init_neopixels();
	init_vibration_motor();
	init_storage();
}

static void blast() {
	LOG_INF("Sending a blast packet...");
	colorgame_blast(is_advertising);
	k_sleep(250);
	if (is_advertising) {
		bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
	} else {
		bt_le_adv_stop();
	}
	k_sleep(1000);
	
}

void main(void) {
	LOG_INF("Starting app...\n");

	init_drivers();
	colorgame_init();

	breathe_led(1000);

	// Startup sequence: vibrate briefly and light LEDs
	write_neopixels_all(blue, true);
	write_vibration_motor(1);
	k_sleep(100);
	write_vibration_motor(0);

	k_sleep(200);
	write_vibration_motor(1);
	k_sleep(100);
	write_vibration_motor(0);
	write_neopixels_all(black, true);  

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

  struct display_capabilities cfg;

	display_get_capabilities(display, &cfg);
  LOG_INF("Display: %d x %d", cfg.x_resolution, cfg.y_resolution);

	if (cfb_framebuffer_init(display)) {
		LOG_ERR("Framebuffer initialization failed!");
		return;
	}

	cfb_framebuffer_clear(display, true);

	display_blanking_on(display);

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

	int err;
	err = bt_enable(bt_ready);
	if (err) {
		LOG_ERR("Bluetooth Failed to start (err %d)", err);
		return;
	}
	
	k_sleep(3000);

	memset(display_buf, 0xff, sizeof(display_buf));
	memcpy(display_buf, logo, sizeof(logo));
	draw_qr(get_qr_url(), 100, 6, 4);
	display_write(display, 0, 0, &desc, display_buf);

	bool qr_mode = false;
	int rc;
	while (1) {
		struct keyboard_event event;
		int ret = k_msgq_get(&keyboard_event_queue, &event, 500);

		if (!ret & event.pressed) {
			switch (event.button) {
				case LEFT_BUTTON:
					blast();
					break;

				case MIDDLE_BUTTON:
					qr_mode = !qr_mode;
					if(qr_mode) {
							memset(display_buf, 0xff, sizeof(display_buf));
							memcpy(display_buf, logo, sizeof(logo));
							draw_qr(get_qr_url(), 100, 6, 4);
							display_write(display, 0, 0, &desc, display_buf);
						err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
						is_advertising = true;
						if (err) {
							LOG_ERR("Advertising failed to start (err %d)", err);
						}
					} else {
						bt_le_adv_stop();
						is_advertising = false;
						rc = nvs_read(&fs, DISPLAY_ID, display_buf, 2000);
						if(rc <= 0) {
							LOG_ERR("cannot read display buf");
						} else {
							
							rc = nvs_read(&fs, DISPLAY2_ID, display_buf + 2000, 3000);
							if(rc <= 0) {
								LOG_ERR("cannot read display buf pt 2");
							}
							else {
								display_dirty = true;
								LOG_INF("showing display");
							}
						}
					}

					break;

				case RIGHT_BUTTON:
					bt_le_adv_stop();
					is_advertising = false;
					agenda_next();
					break;
			}
		}

    if (display_dirty) {
			display_write(display, 0, 0, &desc, display_buf);
      display_dirty = false;
    }
	}
}