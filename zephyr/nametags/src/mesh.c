#include <logging/log.h>
LOG_MODULE_REGISTER(mesh);

#include <zephyr.h>
#include <device.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>

#include "mesh.h"

#define MOD_LF 0x0000

#define GROUP_ADDR 0xc000
#define PUBLISHER_ADDR 0x000f

#define OP_VENDOR_BUTTON BT_MESH_MODEL_OP_3(0x00, BT_COMP_ID_LF)

static const u8_t net_key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
static const u8_t dev_key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
static const u8_t app_key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};

static const u16_t net_idx;
static const u16_t app_idx;
static const u32_t iv_index;
static u8_t flags;
static u16_t addr = 0x0b0c;

static void heartbeat(u8_t hops, u16_t feat)
{
  LOG_INF("heartbeat, %d, %d", hops, feat);
}

static struct bt_mesh_cfg_srv cfg_srv = {
    .relay = BT_MESH_RELAY_ENABLED,
    .beacon = BT_MESH_BEACON_ENABLED,
    .frnd = BT_MESH_FRIEND_NOT_SUPPORTED,
    .default_ttl = 7,

    /* 3 transmissions with 20ms interval */
    .net_transmit = BT_MESH_TRANSMIT(2, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(3, 20),

    .hb_sub.func = heartbeat,
};

static void vnd_button_pressed(struct bt_mesh_model *model,
                               struct bt_mesh_msg_ctx *ctx,
                               struct net_buf_simple *buf)
{
  LOG_INF("src 0x%04x val\n", ctx->addr);

  if (ctx->addr == bt_mesh_model_elem(model)->addr)
  {
    return;
  }

  LOG_INF("button pressed");
}

static const struct bt_mesh_model_op vnd_ops[] = {
    {OP_VENDOR_BUTTON, 0, vnd_button_pressed},
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model vnd_models[] = {
    BT_MESH_MODEL_VND(BT_COMP_ID_LF, MOD_LF, vnd_ops, NULL, NULL),
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

static struct bt_mesh_cfg_cli cfg_cli = {};

static void attention_on(struct bt_mesh_model *model)
{
  LOG_INF("attention_on()");
}

static void attention_off(struct bt_mesh_model *model)
{
  LOG_INF("attention_off()");
}

static const struct bt_mesh_health_srv_cb health_srv_cb = {
    .attn_on = attention_on,
    .attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
    .cb = &health_srv_cb,
};

static struct bt_mesh_model root_models[] = {
    BT_MESH_MODEL_CFG_SRV(&cfg_srv),
    BT_MESH_MODEL_CFG_CLI(&cfg_cli),
    BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
};

static struct bt_mesh_elem elements[] = {
    BT_MESH_ELEM(0, root_models, vnd_models),
};

static const struct bt_mesh_comp comp = {
    .cid = BT_COMP_ID_LF,
    .elem = elements,
    .elem_count = ARRAY_SIZE(elements),
};

static void mesh_configure()
{
  /* Add Application Key */
  bt_mesh_cfg_app_key_add(net_idx, addr, net_idx, app_idx, app_key, NULL);

  /* Bind to vendor model */
  bt_mesh_cfg_mod_app_bind_vnd(net_idx, addr, addr, app_idx,
                               MOD_LF, BT_COMP_ID_LF, NULL);

  /* Bind to Health model */
  bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx,
                           BT_MESH_MODEL_ID_HEALTH_SRV, NULL);

  /* Add model subscription */
  bt_mesh_cfg_mod_sub_add_vnd(net_idx, addr, addr, GROUP_ADDR,
                              MOD_LF, BT_COMP_ID_LF, NULL);
}

static const u8_t dev_uuid[16] = {0xdd, 0xdd};

static const struct bt_mesh_prov prov = {
    .uuid = dev_uuid,
};

void mesh_init()
{
  int err = bt_mesh_init(&prov, &comp);
  if (err)
  {
    LOG_ERR("Initializing mesh failed (err %d)\n", err);
    return;
  }
  err = bt_mesh_provision(net_key, net_idx, flags, iv_index, addr,
                          dev_key);
  if (err == -EALREADY)
  {
    LOG_INF("Using stored settings\n");
  }
  else if (err)
  {
    LOG_ERR("Provisioning failed (err %d)\n", err);
    return;
  }
  else
  {
    LOG_INF("Provisioning completed\n");
    mesh_configure();
  }
}