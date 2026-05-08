#include "lrp_port.h"

#include <stddef.h>

uint32_t lrp_port_time_ms(
    const lrp_port_t *port
)
{
    if ((port == NULL) || (port->time_ms == NULL)) {
        return 0u;
    }

    return port->time_ms(port->ctx);
}

uint32_t lrp_port_random_u32(
    const lrp_port_t *port
)
{
    if ((port == NULL) || (port->random_u32 == NULL)) {
        return 0u;
    }

    return port->random_u32(port->ctx);
}
