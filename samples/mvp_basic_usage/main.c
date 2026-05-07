#include "lrp.h"

#include <stdio.h>
#include <string.h>

#define NETWORK_ID 0x1001u
#define ENDPOINT_ID 0x0101u
#define RELAY_ID 0x0202u
#define BASE_ID LRP_ADDR_BASE

static void print_frame(const char *label, const lrp_frame_t *frame)
{
    printf(
        "%s: type=%u src=0x%04X dst=0x%04X seq=%u ttl=%u payload_len=%u\n",
        label,
        frame->type,
        frame->src_id,
        frame->dst_id,
        frame->seq,
        frame->ttl,
        frame->payload_len
    );
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

static lrp_frame_t make_sensor_data_frame(void)
{
    lrp_frame_t frame;

    memset(&frame, 0, sizeof(frame));

    frame.flags = LRP_FLAG_ACK_REQUIRED;
    frame.type = LRP_TYPE_DATA;
    frame.ttl = 1u;
    frame.network_id = NETWORK_ID;
    frame.src_id = ENDPOINT_ID;
    frame.dst_id = BASE_ID;
    frame.seq = 1u;

    frame.payload[0] = 25u;
    frame.payload[1] = 60u;
    frame.payload[2] = 98u;
    frame.payload_len = 3u;

    return frame;
}

static lrp_frame_t make_ack_frame(const lrp_frame_t *data)
{
    lrp_frame_t ack;

    memset(&ack, 0, sizeof(ack));

    ack.flags = LRP_FLAG_IS_ACK;
    ack.type = LRP_TYPE_ACK;
    ack.ttl = 1u;
    ack.network_id = data->network_id;
    ack.src_id = data->dst_id;
    ack.dst_id = data->src_id;
    ack.seq = data->seq;
    ack.payload_len = 0u;

    return ack;
}

int main(void)
{
    lrp_router_t relay;
    lrp_router_t base;
    lrp_router_t endpoint;

    lrp_frame_t endpoint_data;
    lrp_frame_t decoded_data;
    lrp_frame_t relay_forward_data;
    lrp_frame_t base_ack;
    lrp_frame_t relay_forward_ack;

    uint8_t buffer[LRP_MAX_FRAME_SIZE];
    uint16_t buffer_len = 0u;

    lrp_status_t status;
    lrp_decision_t decision;

    init_router(&endpoint, ENDPOINT_ID, false);
    init_router(&relay, RELAY_ID, true);
    init_router(&base, BASE_ID, false);

    endpoint_data = make_sensor_data_frame();
    print_frame("Endpoint creates DATA", &endpoint_data);

    status = lrp_frame_encode(
        &endpoint_data,
        buffer,
        sizeof(buffer),
        &buffer_len
    );

    if (status != LRP_OK) {
        printf("DATA encode failed: %d\n", status);
        return 1;
    }

    printf("Endpoint encodes DATA into %u bytes\n", buffer_len);

    status = lrp_frame_decode(buffer, buffer_len, &decoded_data);

    if (status != LRP_OK) {
        printf("Relay decode DATA failed: %d\n", status);
        return 1;
    }

    decision = lrp_router_on_frame(
        &relay,
        &decoded_data,
        &relay_forward_data,
        1000u
    );

    if (decision != LRP_DECISION_FORWARD) {
        printf("Relay did not forward DATA: decision=%d\n", decision);
        return 1;
    }

    print_frame("Relay forwards DATA", &relay_forward_data);

    decision = lrp_router_on_frame(
        &base,
        &relay_forward_data,
        NULL,
        2000u
    );

    if (decision != LRP_DECISION_CONSUME) {
        printf("Base did not consume DATA: decision=%d\n", decision);
        return 1;
    }

    print_frame("Base consumes DATA", &relay_forward_data);

    base_ack = make_ack_frame(&relay_forward_data);
    print_frame("Base creates ACK", &base_ack);

    decision = lrp_router_on_frame(
        &relay,
        &base_ack,
        &relay_forward_ack,
        3000u
    );

    if (decision != LRP_DECISION_FORWARD) {
        printf("Relay did not forward ACK: decision=%d\n", decision);
        return 1;
    }

    print_frame("Relay forwards ACK", &relay_forward_ack);

    decision = lrp_router_on_frame(
        &endpoint,
        &relay_forward_ack,
        NULL,
        4000u
    );

    if (decision != LRP_DECISION_CONSUME) {
        printf("Endpoint did not consume ACK: decision=%d\n", decision);
        return 1;
    }

    print_frame("Endpoint receives ACK", &relay_forward_ack);

    if (relay_forward_ack.seq != endpoint_data.seq) {
        printf("ACK seq mismatch\n");
        return 1;
    }

    printf("PASS: MVP basic usage sample\n");

    return 0;
}
