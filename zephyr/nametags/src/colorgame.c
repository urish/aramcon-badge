#include <logging/log.h>
LOG_MODULE_REGISTER(colorgame);

#include <bluetooth/bluetooth.h>

#include "drivers/neopixels.h"
#include "drivers/vibration_motor.h"
#include "colorgame.h"

#define PACKET_VENDOR_ID (0x1337)
#define RSSI_THRESHOLD (-60)
#define BLAST_TIMEOUT (5000)

u32_t last_blast;

struct __attribute__((__packed__)) colorgame_packet
{
  u16_t vendor_id;
  u8_t r;
  u8_t g;
  u8_t b;
};

static struct led_rgb my_color = {
    .r = 0,
    .g = 0x10,
    .b = 0x10,
};

void colorgame_packet_handler(void *buf, u8_t len, s8_t rssi)
{
  struct colorgame_packet *packet = buf;
  if (len >= sizeof(packet) && packet->vendor_id == PACKET_VENDOR_ID)
  {
    LOG_INF("Incoming blast, RSSI=%d - %s", rssi, rssi >= RSSI_THRESHOLD ? "accepted" : "rejected");
    if (rssi >= RSSI_THRESHOLD)
    {
      my_color.r = packet->r;
      my_color.g = packet->g;
      my_color.b = packet->b;
      pulse_vibration_motor(400);
      write_neopixels_all(my_color, true);
    }
  }
}

void colorgame_blast(bool is_advertising)
{
  const struct colorgame_packet data_packet = {
      .vendor_id = PACKET_VENDOR_ID,
      .r = my_color.r,
      .g = my_color.g,
      .b = my_color.b,
  };

  const struct bt_data ad[] = {
      BT_DATA(BT_DATA_MANUFACTURER_DATA, &data_packet, sizeof(data_packet)),
  };

  s32_t delta = k_uptime_get_32() - last_blast;
  if (delta < BLAST_TIMEOUT) {
    return;
  }

  if (is_advertising) {
    bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
  } else {
    bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
  }

  last_blast = k_uptime_get_32();
}

void colorgame_init()
{
  last_blast = k_uptime_get_32();
}