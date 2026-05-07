#include "lrp.h"

#include <stdio.h>
#include <string.h>

#define TEST_ASSERT(cond)                                                       \
    do {                                                                       \
        if (!(cond)) {                                                          \
            printf("FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
            return 1;                                                           \
        }                                                                       \
    } while (0)

static int test_encode_decode_data_frame(void)
{
    lrp_frame_t frame;
    lrp_frame_t decoded;
    uint8_t buffer[LRP_MAX_FRAME_SIZE];
    uint16_t out_len = 0;
    uint8_t payload[] = {0x10, 0x20, 0x30, 0x40};

    memset(&frame, 0, sizeof(frame));
    memset(&decoded, 0, sizeof(decoded));

    frame.version = 0xFFu;
    frame.flags = LRP_FLAG_ACK_REQUIRED;
    frame.type = LRP_TYPE_DATA;
    frame.ttl = 1u;
    frame.network_id = 0x1001u;
    frame.src_id = 0x0101u;
    frame.dst_id = LRP_ADDR_BASE;
    frame.seq = 0x0042u;
    frame.payload_len = (uint8_t)sizeof(payload);
    memcpy(frame.payload, payload, sizeof(payload));

    TEST_ASSERT(lrp_frame_encode(&frame, buffer, sizeof(buffer), &out_len) == LRP_OK);
    TEST_ASSERT(out_len == (LRP_HEADER_SIZE + sizeof(payload) + LRP_CRC_SIZE));
    TEST_ASSERT(buffer[0] == LRP_VERSION);

    TEST_ASSERT(lrp_frame_decode(buffer, out_len, &decoded) == LRP_OK);
    TEST_ASSERT(decoded.version == LRP_VERSION);
    TEST_ASSERT(decoded.flags == frame.flags);
    TEST_ASSERT(decoded.type == frame.type);
    TEST_ASSERT(decoded.ttl == frame.ttl);
    TEST_ASSERT(decoded.network_id == frame.network_id);
    TEST_ASSERT(decoded.src_id == frame.src_id);
    TEST_ASSERT(decoded.dst_id == frame.dst_id);
    TEST_ASSERT(decoded.seq == frame.seq);
    TEST_ASSERT(decoded.payload_len == frame.payload_len);
    TEST_ASSERT(memcmp(decoded.payload, payload, sizeof(payload)) == 0);

    return 0;
}

static int test_encode_decode_ack_zero_payload(void)
{
    lrp_frame_t frame;
    lrp_frame_t decoded;
    uint8_t buffer[LRP_MAX_FRAME_SIZE];
    uint16_t out_len = 0;

    memset(&frame, 0, sizeof(frame));
    memset(&decoded, 0, sizeof(decoded));

    frame.flags = LRP_FLAG_IS_ACK;
    frame.type = LRP_TYPE_ACK;
    frame.ttl = 1u;
    frame.network_id = 0x1001u;
    frame.src_id = LRP_ADDR_BASE;
    frame.dst_id = 0x0101u;
    frame.seq = 0x0042u;
    frame.payload_len = 0u;

    TEST_ASSERT(lrp_frame_encode(&frame, buffer, sizeof(buffer), &out_len) == LRP_OK);
    TEST_ASSERT(out_len == (LRP_HEADER_SIZE + LRP_CRC_SIZE));
    TEST_ASSERT(lrp_frame_decode(buffer, out_len, &decoded) == LRP_OK);
    TEST_ASSERT(decoded.version == LRP_VERSION);
    TEST_ASSERT(decoded.flags == LRP_FLAG_IS_ACK);
    TEST_ASSERT(decoded.type == LRP_TYPE_ACK);
    TEST_ASSERT(decoded.payload_len == 0u);

    return 0;
}

static int test_bad_crc_rejected(void)
{
    lrp_frame_t frame;
    lrp_frame_t decoded;
    uint8_t buffer[LRP_MAX_FRAME_SIZE];
    uint16_t out_len = 0;

    memset(&frame, 0, sizeof(frame));
    memset(&decoded, 0, sizeof(decoded));

    frame.type = LRP_TYPE_DATA;
    frame.network_id = 0x1001u;
    frame.src_id = 0x0101u;
    frame.dst_id = LRP_ADDR_BASE;
    frame.seq = 1u;

    TEST_ASSERT(lrp_frame_encode(&frame, buffer, sizeof(buffer), &out_len) == LRP_OK);
    buffer[4] ^= 0x01u;
    TEST_ASSERT(lrp_frame_decode(buffer, out_len, &decoded) == LRP_ERR_INVALID_CRC);

    return 0;
}

static int test_bad_version_rejected(void)
{
    lrp_frame_t frame;
    lrp_frame_t decoded;
    uint8_t buffer[LRP_MAX_FRAME_SIZE];
    uint16_t out_len = 0;
    uint16_t crc;

    memset(&frame, 0, sizeof(frame));
    memset(&decoded, 0, sizeof(decoded));

    frame.type = LRP_TYPE_DATA;
    frame.network_id = 0x1001u;
    frame.src_id = 0x0101u;
    frame.dst_id = LRP_ADDR_BASE;
    frame.seq = 1u;

    TEST_ASSERT(lrp_frame_encode(&frame, buffer, sizeof(buffer), &out_len) == LRP_OK);

    buffer[0] = 0x02u;
    crc = lrp_crc16_ccitt(buffer, LRP_HEADER_SIZE);
    buffer[LRP_HEADER_SIZE] = (uint8_t)(crc & 0xFFu);
    buffer[LRP_HEADER_SIZE + 1u] = (uint8_t)((crc >> 8u) & 0xFFu);

    TEST_ASSERT(lrp_frame_decode(buffer, out_len, &decoded) == LRP_ERR_INVALID_VERSION);

    return 0;
}

static int test_bad_flags_rejected(void)
{
    lrp_frame_t frame;
    uint8_t buffer[LRP_MAX_FRAME_SIZE];
    uint16_t out_len = 0;

    memset(&frame, 0, sizeof(frame));

    frame.flags = 0x80u;
    frame.type = LRP_TYPE_DATA;
    frame.network_id = 0x1001u;
    frame.src_id = 0x0101u;
    frame.dst_id = LRP_ADDR_BASE;
    frame.seq = 1u;

    TEST_ASSERT(lrp_frame_encode(&frame, buffer, sizeof(buffer), &out_len) == LRP_ERR_INVALID_FLAGS);

    return 0;
}

static int test_invalid_length_rejected(void)
{
    lrp_frame_t frame;
    lrp_frame_t decoded;
    uint8_t buffer[LRP_MAX_FRAME_SIZE];
    uint16_t out_len = 0;

    memset(&frame, 0, sizeof(frame));
    memset(&decoded, 0, sizeof(decoded));

    frame.type = LRP_TYPE_DATA;
    frame.network_id = 0x1001u;
    frame.src_id = 0x0101u;
    frame.dst_id = LRP_ADDR_BASE;
    frame.seq = 1u;

    TEST_ASSERT(lrp_frame_encode(&frame, buffer, sizeof(buffer), &out_len) == LRP_OK);
    TEST_ASSERT(lrp_frame_decode(buffer, (uint16_t)(out_len - 1u), &decoded) == LRP_ERR_INVALID_LENGTH);

    return 0;
}

static int test_buffer_too_small_rejected(void)
{
    lrp_frame_t frame;
    uint8_t buffer[LRP_HEADER_SIZE];
    uint16_t out_len = 0;

    memset(&frame, 0, sizeof(frame));

    frame.type = LRP_TYPE_DATA;
    frame.network_id = 0x1001u;
    frame.src_id = 0x0101u;
    frame.dst_id = LRP_ADDR_BASE;
    frame.seq = 1u;
    frame.payload_len = 4u;

    TEST_ASSERT(lrp_frame_encode(&frame, buffer, sizeof(buffer), &out_len) == LRP_ERR_BUFFER_TOO_SMALL);

    return 0;
}

static int test_invalid_type_rejected(void)
{
    lrp_frame_t frame;
    uint8_t buffer[LRP_MAX_FRAME_SIZE];
    uint16_t out_len = 0;

    memset(&frame, 0, sizeof(frame));

    frame.type = 0x7Fu;
    frame.network_id = 0x1001u;
    frame.src_id = 0x0101u;
    frame.dst_id = LRP_ADDR_BASE;
    frame.seq = 1u;

    TEST_ASSERT(lrp_frame_encode(&frame, buffer, sizeof(buffer), &out_len) == LRP_ERR_INVALID_TYPE);

    return 0;
}

int main(void)
{
    TEST_ASSERT(test_encode_decode_data_frame() == 0);
    TEST_ASSERT(test_encode_decode_ack_zero_payload() == 0);
    TEST_ASSERT(test_bad_crc_rejected() == 0);
    TEST_ASSERT(test_bad_version_rejected() == 0);
    TEST_ASSERT(test_bad_flags_rejected() == 0);
    TEST_ASSERT(test_invalid_length_rejected() == 0);
    TEST_ASSERT(test_buffer_too_small_rejected() == 0);
    TEST_ASSERT(test_invalid_type_rejected() == 0);

    printf("PASS: frame layer strict validation\n");
    return 0;
}
