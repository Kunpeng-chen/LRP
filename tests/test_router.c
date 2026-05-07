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

static lrp_frame_t make_frame(
    uint16_t network_id,
    uint16_t src_id,
    uint16_t dst_id,
    uint16_t seq,
    uint8_t ttl
)
{
    lrp_frame_t frame;

    memset(&frame, 0, sizeof(frame));

    frame.network_id = network_id;
    frame.src_id = src_id;
    frame.dst_id = dst_id;
    frame.seq = seq;
    frame.ttl = ttl;
    frame.type = LRP_TYPE_DATA;

    frame.payload_len = 2u;
    frame.payload[0] = 0x11u;
    frame.payload[1] = 0x22u;

    return frame;
}

static lrp_router_t make_router(bool relay_enabled)
{
    lrp_router_t router;
    lrp_router_config_t config;

    config.network_id = 0x1001u;
    config.node_id = 0x0202u;
    config.relay_enabled = relay_enabled;

    lrp_router_init(&router, &config);

    return router;
}

static int test_consume_when_dst_is_self(void)
{
    lrp_router_t router = make_router(true);
    lrp_frame_t frame = make_frame(0x1001u, 0x0101u, 0x0202u, 1u, 1u);

    TEST_ASSERT(
        lrp_router_on_frame(&router, &frame, NULL, 1000u) ==
        LRP_DECISION_CONSUME
    );

    return 0;
}

static int test_drop_when_network_mismatch(void)
{
    lrp_router_t router = make_router(true);
    lrp_frame_t frame = make_frame(0x9999u, 0x0101u, 0x0303u, 1u, 1u);

    TEST_ASSERT(
        lrp_router_on_frame(&router, &frame, NULL, 1000u) ==
        LRP_DECISION_DROP
    );

    return 0;
}

static int test_drop_when_duplicate(void)
{
    lrp_router_t router = make_router(true);
    lrp_frame_t frame = make_frame(0x1001u, 0x0101u, 0x0303u, 1u, 1u);

    lrp_frame_t out;

    TEST_ASSERT(
        lrp_router_on_frame(&router, &frame, &out, 1000u) ==
        LRP_DECISION_FORWARD
    );

    TEST_ASSERT(
        lrp_router_on_frame(&router, &frame, &out, 1001u) ==
        LRP_DECISION_DROP
    );

    return 0;
}

static int test_drop_when_relay_disabled(void)
{
    lrp_router_t router = make_router(false);
    lrp_frame_t frame = make_frame(0x1001u, 0x0101u, 0x0303u, 1u, 1u);

    TEST_ASSERT(
        lrp_router_on_frame(&router, &frame, NULL, 1000u) ==
        LRP_DECISION_DROP
    );

    return 0;
}

static int test_drop_when_ttl_zero(void)
{
    lrp_router_t router = make_router(true);
    lrp_frame_t frame = make_frame(0x1001u, 0x0101u, 0x0303u, 1u, 0u);

    TEST_ASSERT(
        lrp_router_on_frame(&router, &frame, NULL, 1000u) ==
        LRP_DECISION_DROP
    );

    return 0;
}

static int test_forward_when_valid(void)
{
    lrp_router_t router = make_router(true);
    lrp_frame_t in = make_frame(0x1001u, 0x0101u, 0x0303u, 1u, 2u);
    lrp_frame_t out;

    memset(&out, 0, sizeof(out));

    TEST_ASSERT(
        lrp_router_on_frame(&router, &in, &out, 1000u) ==
        LRP_DECISION_FORWARD
    );

    TEST_ASSERT(out.ttl == 1u);
    TEST_ASSERT(out.network_id == in.network_id);
    TEST_ASSERT(out.src_id == in.src_id);
    TEST_ASSERT(out.dst_id == in.dst_id);
    TEST_ASSERT(out.seq == in.seq);
    TEST_ASSERT(out.type == in.type);
    TEST_ASSERT(out.payload_len == in.payload_len);
    TEST_ASSERT(memcmp(out.payload, in.payload, in.payload_len) == 0);

    return 0;
}

int main(void)
{
    TEST_ASSERT(test_consume_when_dst_is_self() == 0);
    TEST_ASSERT(test_drop_when_network_mismatch() == 0);
    TEST_ASSERT(test_drop_when_duplicate() == 0);
    TEST_ASSERT(test_drop_when_relay_disabled() == 0);
    TEST_ASSERT(test_drop_when_ttl_zero() == 0);
    TEST_ASSERT(test_forward_when_valid() == 0);

    printf("PASS: router layer strict validation\n");

    return 0;
}
