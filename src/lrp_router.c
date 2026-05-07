#include "lrp_router.h"
#include "lrp_dedup.h"

#include <string.h>

void lrp_router_init(
    lrp_router_t *router,
    const lrp_router_config_t *config
)
{
    if ((router == NULL) || (config == NULL)) {
        return;
    }

    memset(router, 0, sizeof(*router));

    router->config = *config;

    lrp_dedup_init(&router->dedup);
}

lrp_decision_t lrp_router_on_frame(
    lrp_router_t *router,
    const lrp_frame_t *in,
    lrp_frame_t *out,
    uint32_t now_ms
)
{
    if ((router == NULL) || (in == NULL)) {
        return LRP_DECISION_DROP;
    }

    if (in->network_id != router->config.network_id) {
        return LRP_DECISION_DROP;
    }

    if (lrp_dedup_check_and_remember(&router->dedup, in, now_ms)) {
        return LRP_DECISION_DROP;
    }

    if (
        (in->dst_id == router->config.node_id) ||
        (in->dst_id == LRP_ADDR_BROADCAST)
    ) {
        return LRP_DECISION_CONSUME;
    }

    if (!router->config.relay_enabled) {
        return LRP_DECISION_DROP;
    }

    if (in->ttl == 0u) {
        return LRP_DECISION_DROP;
    }

    if (out == NULL) {
        return LRP_DECISION_DROP;
    }

    *out = *in;

    out->ttl = (uint8_t)(in->ttl - 1u);

    return LRP_DECISION_FORWARD;
}
