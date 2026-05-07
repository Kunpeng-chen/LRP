#include "lrp.h"

#include <stddef.h>
#include <string.h>

static void lrp_write_u16_le(uint8_t *dst, uint16_t value)
{
    dst[0] = (uint8_t)(value & 0xFFu);
    dst[1] = (uint8_t)((value >> 8u) & 0xFFu);
}

static uint16_t lrp_read_u16_le(const uint8_t *src)
{
    return (uint16_t)(
        ((uint16_t)src[0]) |
        ((uint16_t)src[1] << 8u)
    );
}

static int lrp_is_valid_type(uint8_t type)
{
    return (
        (type == LRP_TYPE_DATA) ||
        (type == LRP_TYPE_ACK)
    );
}

uint16_t lrp_crc16_ccitt(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFFu;
    uint16_t i;

    if (data == NULL) {
        return 0u;
    }

    for (i = 0u; i < len; ++i) {
        crc ^= (uint16_t)data[i] << 8u;

        for (uint8_t bit = 0u; bit < 8u; ++bit) {
            if ((crc & 0x8000u) != 0u) {
                crc = (uint16_t)((crc << 1u) ^ 0x1021u);
            } else {
                crc <<= 1u;
            }
        }
    }

    return crc;
}

lrp_status_t lrp_frame_encode(
    const lrp_frame_t *frame,
    uint8_t *out,
    uint16_t out_size,
    uint16_t *out_len
)
{
    uint16_t required_size;
    uint16_t crc;

    if ((frame == NULL) || (out == NULL) || (out_len == NULL)) {
        return LRP_ERR_NULL;
    }

    if (frame->payload_len > LRP_MAX_PAYLOAD_SIZE) {
        return LRP_ERR_PAYLOAD_TOO_LARGE;
    }

    if (!lrp_is_valid_type(frame->type)) {
        return LRP_ERR_INVALID_TYPE;
    }

    if ((frame->flags & (uint8_t)(~LRP_FLAG_MVP_MASK)) != 0u) {
        return LRP_ERR_INVALID_FLAGS;
    }

    required_size = (uint16_t)(
        LRP_HEADER_SIZE +
        frame->payload_len +
        LRP_CRC_SIZE
    );

    if (out_size < required_size) {
        return LRP_ERR_BUFFER_TOO_SMALL;
    }

    out[0] = LRP_VERSION;
    out[1] = frame->flags;
    out[2] = frame->type;
    out[3] = frame->ttl;

    lrp_write_u16_le(&out[4], frame->network_id);
    lrp_write_u16_le(&out[6], frame->src_id);
    lrp_write_u16_le(&out[8], frame->dst_id);
    lrp_write_u16_le(&out[10], frame->seq);

    out[12] = frame->payload_len;

    if (frame->payload_len > 0u) {
        memcpy(&out[13], frame->payload, frame->payload_len);
    }

    crc = lrp_crc16_ccitt(out, (uint16_t)(LRP_HEADER_SIZE + frame->payload_len));

    lrp_write_u16_le(
        &out[LRP_HEADER_SIZE + frame->payload_len],
        crc
    );

    *out_len = required_size;

    return LRP_OK;
}

lrp_status_t lrp_frame_decode(
    const uint8_t *data,
    uint16_t len,
    lrp_frame_t *out
)
{
    uint8_t payload_len;
    uint16_t expected_len;
    uint16_t received_crc;
    uint16_t computed_crc;

    if ((data == NULL) || (out == NULL)) {
        return LRP_ERR_NULL;
    }

    if (len < (LRP_HEADER_SIZE + LRP_CRC_SIZE)) {
        return LRP_ERR_INVALID_LENGTH;
    }

    payload_len = data[12];

    if (payload_len > LRP_MAX_PAYLOAD_SIZE) {
        return LRP_ERR_PAYLOAD_TOO_LARGE;
    }

    expected_len = (uint16_t)(
        LRP_HEADER_SIZE +
        payload_len +
        LRP_CRC_SIZE
    );

    if (len != expected_len) {
        return LRP_ERR_INVALID_LENGTH;
    }

    received_crc = lrp_read_u16_le(
        &data[LRP_HEADER_SIZE + payload_len]
    );

    computed_crc = lrp_crc16_ccitt(
        data,
        (uint16_t)(LRP_HEADER_SIZE + payload_len)
    );

    if (received_crc != computed_crc) {
        return LRP_ERR_INVALID_CRC;
    }

    if (data[0] != LRP_VERSION) {
        return LRP_ERR_INVALID_VERSION;
    }

    if (!lrp_is_valid_type(data[2])) {
        return LRP_ERR_INVALID_TYPE;
    }

    if ((data[1] & (uint8_t)(~LRP_FLAG_MVP_MASK)) != 0u) {
        return LRP_ERR_INVALID_FLAGS;
    }

    out->version = data[0];
    out->flags = data[1];
    out->type = data[2];
    out->ttl = data[3];

    out->network_id = lrp_read_u16_le(&data[4]);
    out->src_id = lrp_read_u16_le(&data[6]);
    out->dst_id = lrp_read_u16_le(&data[8]);
    out->seq = lrp_read_u16_le(&data[10]);

    out->payload_len = payload_len;

    if (payload_len > 0u) {
        memcpy(out->payload, &data[13], payload_len);
    }

    return LRP_OK;
}
