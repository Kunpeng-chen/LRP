#ifndef LRP_ROUTER_H
#define LRP_ROUTER_H

#include "lrp_type.h"

#ifdef __cplusplus
extern "C" {
#endif

void lrp_router_init(
    lrp_router_t *router,
    const lrp_router_config_t *config
);

lrp_decision_t lrp_router_on_frame(
    lrp_router_t *router,
    const lrp_frame_t *in,
    lrp_frame_t *out,
    uint32_t now_ms
);

#ifdef __cplusplus
}
#endif

#endif /* LRP_ROUTER_H */
