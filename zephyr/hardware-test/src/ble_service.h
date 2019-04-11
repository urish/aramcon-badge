#ifndef __BLE_SERVICE_H__
#define __BLE_SERVICE_H__
#include <zephyr.h>

void init_ble_service();

const char *get_ble_name();

const char* get_ble_mac();

const char* get_ble_remote_mac();

bool ble_service_is_ready();





#endif // __BLE_SERVICE_H__