#ifndef LRP_H
#define LRP_H

#include "lrp_type.h"

#ifdef __cplusplus
extern "C" {
#endif

lrp_status_t lrp_frame_encode(
    const lrp_frame_t *frame,
    uint8_t *out,
    uint16_t out_size,
    uint16_t *out_len
);

lrp_status_t lrp_frame_decode(
    const uint8_t *data,
    uint16_t len,
    lrp_frame_t *out
);

uint16_t lrp_crc16_ccitt(
    const uint8_t *data,
    uint16_t len
);

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

#endif /* LRP_H */
