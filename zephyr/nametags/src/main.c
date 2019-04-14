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
#include <led_strip.h>
#include <display/cfb.h>
#include <misc/util.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "drivers/buttons.h"
#include "agenda.h"
#include "qrcode.h"

#define LED_PORT LED0_GPIO_CONTROLLER
#define LED	LED0_GPIO_PIN

#define STRIP_NUM_LEDS 4
#define STRIP_DEV_NAME DT_WORLDSEMI_WS2812_0_LABEL

#define SLEEP_TIME 	500

#define DISPLAY_DRIVER		"DISPLAY"

#define BT_UUID_DISP       BT_UUID_DECLARE_16(0xfeef)
#define BT_UUID_DISP_DATA  BT_UUID_DECLARE_16(0xfeee)

#define DISPLAY_WIDTH 296
#define DISPLAY_HEIGHT 128

static const struct led_rgb red = { .r = 0x02, .g = 0x00, .b = 0x00 };
static const struct led_rgb green = { .r = 0x0, .g = 0x02, .b = 0x0 };
static const struct led_rgb blue = { .r = 0x0, .g = 0x0, .b = 0x02 };
static const struct led_rgb purple = { .r = 0x10, .g = 0x0, .b = 0x02 };

static struct led_rgb strip_colors[STRIP_NUM_LEDS];

static uint16_t counter = 0;
struct device *display;
static bool bluetooth_ready = false;
static bool display_set = false;
static const bt_addr_le_t *remote_device = NULL;

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

	char *status = "\\|/-";
	sprintf(buf, "%c", status[counter % sizeof(status)]);
	cfb_print(display, buf, 0, 96);

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

static void disconnected(struct bt_conn *conn, u8_t reason) {
	LOG_INF("Disconnected from %s (reason %u)\n", bluetooth_mac_to_str(remote_device), reason);
	remote_device = NULL;
}

u8_t display_buf[5000] = {0};
bool display_dirty = false;

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
	qrcode_initText(&qrcode, qrcode_data, QR_VERSION, ECC_HIGH, text);

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

char disp_char_buf[20] = {0};

static struct bt_gatt_attr disp_attrs[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_DISP),
	BT_GATT_CHARACTERISTIC(BT_UUID_DISP_DATA,
			       BT_GATT_CHRC_WRITE | BT_GATT_CHRC_INDICATE,
			       BT_GATT_PERM_WRITE, NULL, write_disp_data,
			       &disp_char_buf),
};

static struct bt_gatt_service disp_svc = BT_GATT_SERVICE(disp_attrs);

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
	bt_gatt_service_register(&disp_svc);

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

	init_buttons();

	// LED
	struct device *gpio = device_get_binding(LED_PORT);
	gpio_pin_configure(gpio, LED, GPIO_DIR_OUT);
  
	// Neopixels
	struct device *strip = device_get_binding(STRIP_DEV_NAME);
	if (!strip) {
		LOG_ERR("LED strip device %s not found", STRIP_DEV_NAME);
		return;
	}

	strip_colors[0] = purple;
	strip_colors[1] = purple;
	strip_colors[2] = purple;
	strip_colors[3] = purple;
	led_strip_update_rgb(strip, strip_colors, STRIP_NUM_LEDS);
	
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

	// Bluetooth
	int err = bt_enable(bt_ready);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return;
	}	

  const struct display_buffer_descriptor desc = {
    .buf_size = DISPLAY_WIDTH * DISPLAY_HEIGHT / 8,
    .width = DISPLAY_WIDTH,
    .height = DISPLAY_HEIGHT,
    .pitch = DISPLAY_WIDTH,
  };

	memset(display_buf, 0xff, sizeof(display_buf));
	draw_qr("https://urish.org", 6, 6, 4);
	display_write(display, 0, 0, &desc, display_buf);

	while (1) {
		if (button_read(LEFT_BUTTON)) {
			agenda_prev();
		}
		if (button_read(MIDDLE_BUTTON)) {
			memset(display_buf, 0xff, sizeof(display_buf));
			draw_qr("https://urish.org", 6, 6, 4);
			display_write(display, 0, 0, &desc, display_buf);
		}
		if (button_read(RIGHT_BUTTON)) {
			agenda_next();
		}

    if (display_dirty) {
			display_write(display, 0, 0, &desc, display_buf);
      display_dirty = false;
    }

		gpio_pin_write(gpio, LED, counter % 2);
		if (counter % 2) {
			strip_colors[0] = red;
			strip_colors[1] = green;
			strip_colors[2] = purple;
			strip_colors[3] = blue;
		} else {
			strip_colors[0] = green;
			strip_colors[1] = red;
			strip_colors[2] = blue;
			strip_colors[3] = purple;
		}
		led_strip_update_rgb(strip, strip_colors, STRIP_NUM_LEDS);
		k_sleep(SLEEP_TIME);

		counter++;
	}
}