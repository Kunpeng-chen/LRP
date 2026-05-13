#include "lrp_router.h"
#include "lrp_dedup.h"

#include <string.h>

static uint32_t lrp_router_calculate_delay_ms(
    const lrp_router_t *router,
    uint32_t random_u32
)
{
    uint32_t jitter;

    if (router->config.relay_delay_jitter_ms == 0u) {
        return router->config.relay_delay_base_ms;
    }

    jitter = random_u32 % (router->config.relay_delay_jitter_ms + 1u);

    return router->config.relay_delay_base_ms + jitter;
}

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

lrp_router_result_t lrp_router_on_frame_with_delay(
    lrp_router_t *router,
    const lrp_frame_t *in,
    lrp_frame_t *out,
    uint32_t now_ms,
    uint32_t random_u32
)
{
    lrp_router_result_t result;

    result.decision = LRP_DECISION_DROP;
    result.delay_ms = 0u;

    if ((router == NULL) || (in == NULL)) {
        return result;
    }

    if (in->network_id != router->config.network_id) {
        return result;
    }

    if (lrp_dedup_check_and_remember(&router->dedup, in, now_ms)) {
        return result;
    }

    if (
        (in->dst_id == router->config.node_id) ||
        (in->dst_id == LRP_ADDR_BROADCAST)
    ) {
        result.decision = LRP_DECISION_CONSUME;
        return result;
    }

    if (!router->config.relay_enabled) {
        return result;
    }

    if (in->ttl == 0u) {
        return result;
    }

    if (out == NULL) {
        return result;
    }

    *out = *in;

    out->ttl = (uint8_t)(in->ttl - 1u);

    result.decision = LRP_DECISION_FORWARD;
    result.delay_ms = lrp_router_calculate_delay_ms(router, random_u32);

    return result;
}

lrp_decision_t lrp_router_on_frame(
    lrp_router_t *router,
    const lrp_frame_t *in,
    lrp_frame_t *out,
    uint32_t now_ms
)
{
    lrp_router_result_t result;

    result = lrp_router_on_frame_with_delay(router, in, out, now_ms, 0u);

    return result.decision;
}
