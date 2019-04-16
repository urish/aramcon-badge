#ifndef __COLORGAME_H__
#define __COLORGAME_H__

#include <zephyr/types.h>

void colorgame_packet_handler(void *buf, u8_t len, s8_t rssi);

void colorgame_blast();

void colorgame_init();

#endif // __COLORGAME_H__