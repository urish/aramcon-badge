#include "ble_service.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ble_service);


static bool bluetooth_ready = false;
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

void init_ble_service()
{
    int err = bt_enable(bt_ready);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return;
	}	
}

const char *get_ble_name()
{
    return bt_get_name();
}

const char* get_ble_mac()
{
    bt_addr_le_t addr;
    size_t count = 1;
    bt_id_get(&addr, &count);
    if(count != 1) {
        LOG_ERR("unexpected address count");
		return NULL;
    }
    return bluetooth_mac_to_str(&addr);
}

const char* get_ble_remote_mac()
{
   return remote_device ? bluetooth_mac_to_str(remote_device) : NULL;
    
}

bool ble_service_is_ready()
{
    return bluetooth_ready;
}