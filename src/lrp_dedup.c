#include "lrp_dedup.h"

#include <limits.h>
#include <string.h>

static int lrp_dedup_match(
    const lrp_dedup_entry_t *entry,
    const lrp_frame_t *frame
)
{
    return (
        entry->used &&
        (entry->network_id == frame->network_id) &&
        (entry->src_id == frame->src_id) &&
        (entry->seq == frame->seq) &&
        (entry->type == frame->type)
    );
}

void lrp_dedup_init(lrp_dedup_cache_t *cache)
{
    if (cache == NULL) {
        return;
    }

    memset(cache, 0, sizeof(*cache));
}

bool lrp_dedup_check_and_remember(
    lrp_dedup_cache_t *cache,
    const lrp_frame_t *frame,
    uint32_t now_ms
)
{
    uint32_t oldest_timestamp = UINT32_MAX;
    size_t oldest_index = 0u;

    if ((cache == NULL) || (frame == NULL)) {
        return false;
    }

    for (size_t i = 0u; i < LRP_DEDUP_CACHE_SIZE; ++i) {

        if (!cache->entries[i].used) {
            oldest_index = i;
            oldest_timestamp = 0u;
            continue;
        }

        if ((now_ms - cache->entries[i].timestamp_ms) > LRP_DEDUP_TTL_MS) {
            cache->entries[i].used = false;
            oldest_index = i;
            oldest_timestamp = 0u;
            continue;
        }

        if (lrp_dedup_match(&cache->entries[i], frame)) {
            return true;
        }

        if (cache->entries[i].timestamp_ms < oldest_timestamp) {
            oldest_timestamp = cache->entries[i].timestamp_ms;
            oldest_index = i;
        }
    }

    cache->entries[oldest_index].network_id = frame->network_id;
    cache->entries[oldest_index].src_id = frame->src_id;
    cache->entries[oldest_index].seq = frame->seq;
    cache->entries[oldest_index].type = frame->type;
    cache->entries[oldest_index].timestamp_ms = now_ms;
    cache->entries[oldest_index].used = true;

    return false;
}
