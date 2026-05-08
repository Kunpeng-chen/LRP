#ifndef LRP_PORT_H
#define LRP_PORT_H

#include "lrp_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*lrp_time_ms_fn_t)(void *ctx);
typedef uint32_t (*lrp_random_u32_fn_t)(void *ctx);

typedef struct {
    lrp_time_ms_fn_t time_ms;
    lrp_random_u32_fn_t random_u32;
    void *ctx;
} lrp_port_t;

uint32_t lrp_port_time_ms(
    const lrp_port_t *port
);

uint32_t lrp_port_random_u32(
    const lrp_port_t *port
);

#ifdef __cplusplus
}
#endif

#endif /* LRP_PORT_H */
