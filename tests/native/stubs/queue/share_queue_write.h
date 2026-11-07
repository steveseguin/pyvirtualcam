#pragma once
#include "share_queue.h"

bool shared_queue_create(share_queue*, int, int, int, int, uint64_t, int);
void shared_queue_write_close(share_queue*);
bool shared_queue_set_delay(share_queue*, int);
bool shared_queue_push_video(share_queue*, uint32_t*, uint32_t, uint32_t,
                             uint8_t**, uint64_t);
