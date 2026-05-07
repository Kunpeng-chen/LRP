#ifndef LRP_FRAME_H
#define LRP_FRAME_H

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

#ifdef __cplusplus
}
#endif

#endif /* LRP_FRAME_H */
