#include "lrp_radio.h"

bool lrp_radio_send(
    const lrp_radio_t *radio,
    const uint8_t *data,
    uint16_t len
)
{
    if (
        (radio == NULL) ||
        (radio->send == NULL) ||
        (data == NULL) ||
        (len == 0u)
    ) {
        return false;
    }

    return radio->send(radio->ctx, data, len);
}
