#ifndef __COLORGAME_H__
#define __COLORGAME_H__

#include <zephyr/types.h>
#include <nvs/nvs.h>

void colorgame_packet_handler(void *buf, u8_t len, s8_t rssi);

void colorgame_blast(bool is_advertising);

void colorgame_init(struct nvs_fs * fs);

#endif // __COLORGAME_H__