#ifndef LRP_DEDUP_H
#define LRP_DEDUP_H

#include "lrp_type.h"

#ifdef __cplusplus
extern "C" {
#endif

void lrp_dedup_init(
    lrp_dedup_cache_t *cache
);

bool lrp_dedup_check_and_remember(
    lrp_dedup_cache_t *cache,
    const lrp_frame_t *frame,
    uint32_t now_ms
);

#ifdef __cplusplus
}
#endif

#endif /* LRP_DEDUP_H */
