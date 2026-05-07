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
    uint16_t seq,
    uint8_t type
)
{
    lrp_frame_t frame;

    memset(&frame, 0, sizeof(frame));

    frame.network_id = network_id;
    frame.src_id = src_id;
    frame.dst_id = LRP_ADDR_BASE;
    frame.seq = seq;
    frame.type = type;

    return frame;
}

static int test_first_seen_not_duplicate(void)
{
    lrp_dedup_cache_t cache;
    lrp_frame_t frame = make_frame(0x1001u, 0x0101u, 1u, LRP_TYPE_DATA);

    lrp_dedup_init(&cache);

    TEST_ASSERT(lrp_dedup_check_and_remember(&cache, &frame, 1000u) == false);

    return 0;
}

static int test_second_seen_is_duplicate(void)
{
    lrp_dedup_cache_t cache;
    lrp_frame_t frame = make_frame(0x1001u, 0x0101u, 1u, LRP_TYPE_DATA);

    lrp_dedup_init(&cache);

    TEST_ASSERT(lrp_dedup_check_and_remember(&cache, &frame, 1000u) == false);
    TEST_ASSERT(lrp_dedup_check_and_remember(&cache, &frame, 1001u) == true);

    return 0;
}

static int test_different_seq_not_duplicate(void)
{
    lrp_dedup_cache_t cache;
    lrp_frame_t frame1 = make_frame(0x1001u, 0x0101u, 1u, LRP_TYPE_DATA);
    lrp_frame_t frame2 = make_frame(0x1001u, 0x0101u, 2u, LRP_TYPE_DATA);

    lrp_dedup_init(&cache);

    TEST_ASSERT(lrp_dedup_check_and_remember(&cache, &frame1, 1000u) == false);
    TEST_ASSERT(lrp_dedup_check_and_remember(&cache, &frame2, 1001u) == false);

    return 0;
}

static int test_different_src_not_duplicate(void)
{
    lrp_dedup_cache_t cache;
    lrp_frame_t frame1 = make_frame(0x1001u, 0x0101u, 1u, LRP_TYPE_DATA);
    lrp_frame_t frame2 = make_frame(0x1001u, 0x0202u, 1u, LRP_TYPE_DATA);

    lrp_dedup_init(&cache);

    TEST_ASSERT(lrp_dedup_check_and_remember(&cache, &frame1, 1000u) == false);
    TEST_ASSERT(lrp_dedup_check_and_remember(&cache, &frame2, 1001u) == false);

    return 0;
}

static int test_different_type_not_duplicate(void)
{
    lrp_dedup_cache_t cache;
    lrp_frame_t frame1 = make_frame(0x1001u, 0x0101u, 1u, LRP_TYPE_DATA);
    lrp_frame_t frame2 = make_frame(0x1001u, 0x0101u, 1u, LRP_TYPE_ACK);

    lrp_dedup_init(&cache);

    TEST_ASSERT(lrp_dedup_check_and_remember(&cache, &frame1, 1000u) == false);
    TEST_ASSERT(lrp_dedup_check_and_remember(&cache, &frame2, 1001u) == false);

    return 0;
}

static int test_different_network_not_duplicate(void)
{
    lrp_dedup_cache_t cache;
    lrp_frame_t frame1 = make_frame(0x1001u, 0x0101u, 1u, LRP_TYPE_DATA);
    lrp_frame_t frame2 = make_frame(0x2002u, 0x0101u, 1u, LRP_TYPE_DATA);

    lrp_dedup_init(&cache);

    TEST_ASSERT(lrp_dedup_check_and_remember(&cache, &frame1, 1000u) == false);
    TEST_ASSERT(lrp_dedup_check_and_remember(&cache, &frame2, 1001u) == false);

    return 0;
}

static int test_expired_entry_not_duplicate(void)
{
    lrp_dedup_cache_t cache;
    lrp_frame_t frame = make_frame(0x1001u, 0x0101u, 1u, LRP_TYPE_DATA);

    lrp_dedup_init(&cache);

    TEST_ASSERT(lrp_dedup_check_and_remember(&cache, &frame, 1000u) == false);

    TEST_ASSERT(
        lrp_dedup_check_and_remember(
            &cache,
            &frame,
            1000u + LRP_DEDUP_TTL_MS + 1u
        ) == false
    );

    return 0;
}

int main(void)
{
    TEST_ASSERT(test_first_seen_not_duplicate() == 0);
    TEST_ASSERT(test_second_seen_is_duplicate() == 0);
    TEST_ASSERT(test_different_seq_not_duplicate() == 0);
    TEST_ASSERT(test_different_src_not_duplicate() == 0);
    TEST_ASSERT(test_different_type_not_duplicate() == 0);
    TEST_ASSERT(test_different_network_not_duplicate() == 0);
    TEST_ASSERT(test_expired_entry_not_duplicate() == 0);

    printf("PASS: dedup layer strict validation\n");

    return 0;
}
