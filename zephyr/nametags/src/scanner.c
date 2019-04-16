#include <logging/log.h>
LOG_MODULE_REGISTER(scanner);

#include <zephyr/types.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "drivers/neopixels.h"
#include "colorgame.h"
#include "scanner.h"

static int counter = 0;

#define PACKET_VENDOR_ID (0xf00d)

struct __attribute__((__packed__)) control_packet
{
  u16_t vendor_id;
  u8_t r;
  u8_t g;
  u8_t b;
};

static void scan_cb(const bt_addr_le_t *addr, s8_t rssi, u8_t adv_type,
                    struct net_buf_simple *ad)
{
  counter++;

  while (ad->len > 1)
  {
    u8_t len = net_buf_simple_pull_u8(ad);
    u8_t type;

    /* Check for early termination */
    if (len == 0U)
    {
      return;
    }

    if (len > ad->len)
    {
      LOG_WRN("AD malformed");
      return;
    }

    type = net_buf_simple_pull_u8(ad);
    u8_t content_len = len - 1;
    struct control_packet *content = net_buf_simple_pull_mem(ad, content_len);

    if (type == BT_DATA_MANUFACTURER_DATA) {
      colorgame_packet_handler(content, content_len, rssi);
    }

    if (type == BT_DATA_MANUFACTURER_DATA &&
        content_len >= sizeof(struct control_packet) &&
        content->vendor_id == PACKET_VENDOR_ID)
    {
      LOG_INF("Control packet: R=%02x, G=%02x, B=%02x", content->r, content->g, content->b);
      const struct led_rgb color = {
          .r = content->r,
          .g = content->g,
          .b = content->b,
      };
      write_neopixels_all(color, true);
      return;
    }
  }
}

int scanner_init()
{
  static struct bt_le_scan_param scan_param = {
      .type = BT_HCI_LE_SCAN_PASSIVE,
      .filter_dup = BT_HCI_LE_SCAN_FILTER_DUP_DISABLE,
      .interval = 0x0010,
      .window = 0x0010,
  };

  int err = bt_le_scan_start(&scan_param, scan_cb);
  if (err)
  {
    LOG_ERR("Starting scanning failed (err %d)", err);
    return err;
  }

  return 0;
}
