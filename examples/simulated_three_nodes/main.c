#include "lrp.h"

#include <stdio.h>
#include <string.h>

#define NETWORK_ID 0x1001u
#define ENDPOINT_A 0x0101u
#define RELAY_B    0x0202u
#define BASE_C     0x0001u

#define CHECK(cond, msg)                                                       \
    do {                                                                       \
        if (!(cond)) {                                                          \
            printf("FAIL: %s\n", msg);                                         \
            return 1;                                                           \
        }                                                                       \
    } while (0)

static void make_data_frame(lrp_frame_t *frame)
{
    const uint8_t payload[] = {0x11u, 0x22u, 0x33u, 0x44u};

    memset(frame, 0, sizeof(*frame));

    frame->flags = LRP_FLAG_ACK_REQUIRED;
    frame->type = LRP_TYPE_DATA;
    frame->ttl = 1u;
    frame->network_id = NETWORK_ID;
    frame->src_id = ENDPOINT_A;
    frame->dst_id = BASE_C;
    frame->seq = 1u;
    frame->payload_len = (uint8_t)sizeof(payload);
    memcpy(frame->payload, payload, sizeof(payload));
}

static void make_ack_frame(const lrp_frame_t *data, lrp_frame_t *ack)
{
    memset(ack, 0, sizeof(*ack));

    ack->flags = LRP_FLAG_IS_ACK;
    ack->type = LRP_TYPE_ACK;
    ack->ttl = 1u;
    ack->network_id = data->network_id;
    ack->src_id = data->dst_id;
    ack->dst_id = data->src_id;
    ack->seq = data->seq;
    ack->payload_len = 0u;
}

static void init_router(
    lrp_router_t *router,
    uint16_t node_id,
    bool relay_enabled
)
{
    lrp_router_config_t config;

    config.network_id = NETWORK_ID;
    config.node_id = node_id;
    config.relay_enabled = relay_enabled;

    lrp_router_init(router, &config);
}

int main(void)
{
    lrp_router_t endpoint_a;
    lrp_router_t relay_b;
    lrp_router_t base_c;

    lrp_frame_t data_from_a;
    lrp_frame_t data_from_b;
    lrp_frame_t ack_from_c;
    lrp_frame_t ack_from_b;

    lrp_decision_t decision;

    init_router(&endpoint_a, ENDPOINT_A, false);
    init_router(&relay_b, RELAY_B, true);
    init_router(&base_c, BASE_C, false);

    make_data_frame(&data_from_a);

    printf("Endpoint A sends DATA seq=%u ttl=%u\n", data_from_a.seq, data_from_a.ttl);

    decision = lrp_router_on_frame(&relay_b, &data_from_a, &data_from_b, 1000u);
    CHECK(decision == LRP_DECISION_FORWARD, "Relay B should forward DATA");
    CHECK(data_from_b.ttl == 0u, "Relay B should decrement DATA TTL");

    printf("Relay B forwards DATA seq=%u ttl=%u\n", data_from_b.seq, data_from_b.ttl);

    decision = lrp_router_on_frame(&base_c, &data_from_b, NULL, 2000u);
    CHECK(decision == LRP_DECISION_CONSUME, "Base C should consume DATA");

    printf("Base C consumes DATA seq=%u\n", data_from_b.seq);

    make_ack_frame(&data_from_b, &ack_from_c);

    printf("Base C sends ACK seq=%u ttl=%u\n", ack_from_c.seq, ack_from_c.ttl);

    decision = lrp_router_on_frame(&relay_b, &ack_from_c, &ack_from_b, 3000u);
    CHECK(decision == LRP_DECISION_FORWARD, "Relay B should forward ACK");
    CHECK(ack_from_b.ttl == 0u, "Relay B should decrement ACK TTL");

    printf("Relay B forwards ACK seq=%u ttl=%u\n", ack_from_b.seq, ack_from_b.ttl);

    decision = lrp_router_on_frame(&endpoint_a, &ack_from_b, NULL, 4000u);
    CHECK(decision == LRP_DECISION_CONSUME, "Endpoint A should consume ACK");
    CHECK(ack_from_b.type == LRP_TYPE_ACK, "Endpoint A should receive ACK type");
    CHECK(ack_from_b.seq == data_from_a.seq, "ACK seq should match DATA seq");
    CHECK(ack_from_b.src_id == BASE_C, "ACK source should be Base C");
    CHECK(ack_from_b.dst_id == ENDPOINT_A, "ACK destination should be Endpoint A");

    printf("Endpoint A receives ACK seq=%u\n", ack_from_b.seq);
    printf("PASS: simulated three-node relay path\n");

    return 0;
}
