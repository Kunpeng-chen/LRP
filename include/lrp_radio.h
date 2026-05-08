#ifndef LRP_RADIO_H
#define LRP_RADIO_H

#include "lrp_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*lrp_radio_send_fn_t)(
    void *ctx,
    const uint8_t *data,
    uint16_t len
);

typedef struct {
    lrp_radio_send_fn_t send;
    void *ctx;
} lrp_radio_t;

bool lrp_radio_send(
    const lrp_radio_t *radio,
    const uint8_t *data,
    uint16_t len
);

#ifdef __cplusplus
}
#endif

#endif /* LRP_RADIO_H */
