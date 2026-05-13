#ifndef LRP_TYPE_H
#define LRP_TYPE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LRP_VERSION              0x01u

#define LRP_HEADER_SIZE          13u
#define LRP_CRC_SIZE             2u
#define LRP_MAX_PAYLOAD_SIZE     64u
#define LRP_MAX_FRAME_SIZE       (LRP_HEADER_SIZE + LRP_MAX_PAYLOAD_SIZE + LRP_CRC_SIZE)

#define LRP_DEDUP_CACHE_SIZE     32u
#define LRP_DEDUP_TTL_MS         300000u

#define LRP_ADDR_INVALID         0x0000u
#define LRP_ADDR_BASE            0x0001u
#define LRP_ADDR_BROADCAST       0xFFFFu

#define LRP_FLAG_ACK_REQUIRED    0x01u
#define LRP_FLAG_IS_ACK          0x02u

#define LRP_FLAG_MVP_MASK        (LRP_FLAG_ACK_REQUIRED | LRP_FLAG_IS_ACK)

typedef enum {
    LRP_OK = 0,
    LRP_ERR_NULL,
    LRP_ERR_BUFFER_TOO_SMALL,
    LRP_ERR_INVALID_LENGTH,
    LRP_ERR_INVALID_VERSION,
    LRP_ERR_INVALID_TYPE,
    LRP_ERR_INVALID_FLAGS,
    LRP_ERR_INVALID_CRC,
    LRP_ERR_PAYLOAD_TOO_LARGE
} lrp_status_t;

typedef enum {
    LRP_TYPE_DATA = 0x01u,
    LRP_TYPE_ACK  = 0x02u
} lrp_packet_type_t;

typedef enum {
    LRP_DECISION_DROP = 0,
    LRP_DECISION_CONSUME,
    LRP_DECISION_FORWARD
} lrp_decision_t;

typedef struct {
    lrp_decision_t decision;
    uint32_t delay_ms;
} lrp_router_result_t;

typedef struct {
    uint8_t version;
    uint8_t flags;
    uint8_t type;
    uint8_t ttl;
    uint16_t network_id;
    uint16_t src_id;
    uint16_t dst_id;
    uint16_t seq;
    uint8_t payload_len;
    uint8_t payload[LRP_MAX_PAYLOAD_SIZE];
} lrp_frame_t;

typedef struct {
    uint16_t network_id;
    uint16_t src_id;
    uint16_t seq;
    uint8_t type;
    uint32_t timestamp_ms;
    bool used;
} lrp_dedup_entry_t;

typedef struct {
    lrp_dedup_entry_t entries[LRP_DEDUP_CACHE_SIZE];
} lrp_dedup_cache_t;

typedef struct {
    uint16_t network_id;
    uint16_t node_id;
    bool relay_enabled;
    uint32_t relay_delay_base_ms;
    uint32_t relay_delay_jitter_ms;
} lrp_router_config_t;

typedef struct {
    lrp_router_config_t config;
    lrp_dedup_cache_t dedup;
} lrp_router_t;

#ifdef __cplusplus
}
#endif

#endif /* LRP_TYPE_H */
