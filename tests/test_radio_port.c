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

typedef struct {
    uint32_t now_ms;
    uint32_t random_value;
    uint8_t sent[LRP_MAX_FRAME_SIZE];
    uint16_t sent_len;
    bool send_called;
} test_ctx_t;

static uint32_t fake_time_ms(void *ctx)
{
    test_ctx_t *test = (test_ctx_t *)ctx;
    return test->now_ms;
}

static uint32_t fake_random_u32(void *ctx)
{
    test_ctx_t *test = (test_ctx_t *)ctx;
    return test->random_value;
}

static bool fake_radio_send(void *ctx, const uint8_t *data, uint16_t len)
{
    test_ctx_t *test = (test_ctx_t *)ctx;

    if ((data == NULL) || (len > LRP_MAX_FRAME_SIZE)) {
        return false;
    }

    memcpy(test->sent, data, len);
    test->sent_len = len;
    test->send_called = true;

    return true;
}

static int test_port_callbacks(void)
{
    test_ctx_t ctx;
    lrp_port_t port;

    memset(&ctx, 0, sizeof(ctx));

    ctx.now_ms = 1234u;
    ctx.random_value = 0xA5A5A5A5u;

    port.time_ms = fake_time_ms;
    port.random_u32 = fake_random_u32;
    port.ctx = &ctx;

    TEST_ASSERT(lrp_port_time_ms(&port) == 1234u);
    TEST_ASSERT(lrp_port_random_u32(&port) == 0xA5A5A5A5u);
    TEST_ASSERT(lrp_port_time_ms(NULL) == 0u);
    TEST_ASSERT(lrp_port_random_u32(NULL) == 0u);

    return 0;
}

static int test_radio_send_callback(void)
{
    test_ctx_t ctx;
    lrp_radio_t radio;
    uint8_t data[] = {0x01u, 0x02u, 0x03u};

    memset(&ctx, 0, sizeof(ctx));

    radio.send = fake_radio_send;
    radio.ctx = &ctx;

    TEST_ASSERT(lrp_radio_send(&radio, data, (uint16_t)sizeof(data)) == true);
    TEST_ASSERT(ctx.send_called == true);
    TEST_ASSERT(ctx.sent_len == sizeof(data));
    TEST_ASSERT(memcmp(ctx.sent, data, sizeof(data)) == 0);

    TEST_ASSERT(lrp_radio_send(NULL, data, (uint16_t)sizeof(data)) == false);
    TEST_ASSERT(lrp_radio_send(&radio, NULL, (uint16_t)sizeof(data)) == false);
    TEST_ASSERT(lrp_radio_send(&radio, data, 0u) == false);

    return 0;
}

static int test_encode_then_radio_send_then_decode(void)
{
    test_ctx_t ctx;
    lrp_radio_t radio;
    lrp_frame_t frame;
    lrp_frame_t decoded;
    uint8_t buffer[LRP_MAX_FRAME_SIZE];
    uint16_t buffer_len = 0u;

    memset(&ctx, 0, sizeof(ctx));
    memset(&frame, 0, sizeof(frame));
    memset(&decoded, 0, sizeof(decoded));

    radio.send = fake_radio_send;
    radio.ctx = &ctx;

    frame.flags = LRP_FLAG_ACK_REQUIRED;
    frame.type = LRP_TYPE_DATA;
    frame.ttl = 1u;
    frame.network_id = 0x1001u;
    frame.src_id = 0x0101u;
    frame.dst_id = LRP_ADDR_BASE;
    frame.seq = 7u;
    frame.payload_len = 2u;
    frame.payload[0] = 0xAAu;
    frame.payload[1] = 0x55u;

    TEST_ASSERT(
        lrp_frame_encode(&frame, buffer, sizeof(buffer), &buffer_len) ==
        LRP_OK
    );

    TEST_ASSERT(lrp_radio_send(&radio, buffer, buffer_len) == true);
    TEST_ASSERT(ctx.send_called == true);
    TEST_ASSERT(ctx.sent_len == buffer_len);

    TEST_ASSERT(lrp_frame_decode(ctx.sent, ctx.sent_len, &decoded) == LRP_OK);
    TEST_ASSERT(decoded.type == LRP_TYPE_DATA);
    TEST_ASSERT(decoded.seq == frame.seq);
    TEST_ASSERT(decoded.payload_len == frame.payload_len);
    TEST_ASSERT(memcmp(decoded.payload, frame.payload, frame.payload_len) == 0);

    return 0;
}

int main(void)
{
    TEST_ASSERT(test_port_callbacks() == 0);
    TEST_ASSERT(test_radio_send_callback() == 0);
    TEST_ASSERT(test_encode_then_radio_send_then_decode() == 0);

    printf("PASS: radio and port abstraction strict validation\n");

    return 0;
}
