#include <cstdio>
#include <cstring>
#include <stdexcept>
#include "queue/share_queue_write.h"
#include "controller/controller.h"

static int creates = 0, closes = 0, pushes = 0;
static int camera_width = 0, camera_height = 0;

static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

bool shared_queue_create(share_queue* queue, int mode, int format,
                         int width, int height, uint64_t interval, int length) {
    require(mode == ModeVideo && format == AV_PIX_FMT_RGBA, "queue format");
    require(interval == 50000000 && length == 10, "queue timing");
    camera_width = width;
    camera_height = height;
    queue->open = true;
    ++creates;
    return true;
}

void shared_queue_write_close(share_queue* queue) {
    // Access the state just as the production queue close does. ASan detects a
    // stale controller passing freed state here on a repeated stop.
    require(queue->open, "queue closed more than once");
    queue->open = false;
    ++closes;
}

bool shared_queue_set_delay(share_queue* queue, int delay) {
    require(queue->open && delay == 0, "queue delay");
    return true;
}

bool shared_queue_push_video(share_queue* queue, uint32_t* stride,
                             uint32_t width, uint32_t height, uint8_t**,
                             uint64_t timestamp) {
    require(queue->open, "send on closed queue");
    require(width == static_cast<uint32_t>(camera_width) &&
            height == static_cast<uint32_t>(camera_height), "frame dimensions");
    require(stride[0] == width * 4 && timestamp == 1000000000, "frame metadata");
    ++pushes;
    return true;
}

static void start(int width = 16, int height = 8) {
    require(virtual_output_start(width, height, 20, 0), "camera failed to start");
    require(virtual_output_is_running(), "camera not running after start");
}

int main(int argc, char** argv) {
    try {
        require(argc == 2, "specify one test case");
        if (!std::strcmp(argv[1], "stop-before-start")) {
            virtual_output_stop();
            virtual_output_stop();
            require(!virtual_output_is_running() && closes == 0, "stop before start");
        } else if (!std::strcmp(argv[1], "send-stop")) {
            start();
            virtual_video(nullptr); // Queue is stubbed; checks metadata only.
            virtual_output_stop();
            virtual_video(nullptr);
            require(!virtual_output_is_running() && pushes == 1 && closes == 1,
                    "send-stop lifecycle");
        } else if (!std::strcmp(argv[1], "duplicate-start")) {
            start();
            require(!virtual_output_start(32, 16, 20, 0), "duplicate start accepted");
            require(virtual_output_is_running() && creates == 1, "active camera lost");
            virtual_video(nullptr);
            virtual_output_stop();
        } else if (!std::strcmp(argv[1], "double-stop")) {
            start();
            virtual_output_stop();
            virtual_output_stop();
            require(closes == 1 && !virtual_output_is_running(), "double stop");
        } else if (!std::strcmp(argv[1], "restart")) {
            start();
            virtual_output_stop();
            start(32, 16);
            virtual_video(nullptr);
            virtual_output_stop();
            require(creates == 2 && closes == 2 && pushes == 1, "restart lifecycle");
        } else if (!std::strcmp(argv[1], "repeat")) {
            for (int i = 0; i < 25; ++i) {
                start(16 + i, 8 + i);
                virtual_video(nullptr);
                virtual_output_stop();
            }
            require(creates == 25 && closes == 25 && pushes == 25, "repeat lifecycle");
        } else {
            throw std::runtime_error("unknown test case");
        }
        std::printf("PASS %s\n", argv[1]);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL %s: %s\n", argc > 1 ? argv[1] : "", error.what());
        return 1;
    }
}
