#pragma once

#include "api.hpp"
#include <chrono>

namespace vc
{
class API_VIDEO_CAPTURE frame_sync
{
public:
    explicit frame_sync(std::chrono::high_resolution_clock::duration tick);
    ~frame_sync() = default;
    void start();
    void reset();
    void update();

private:
    std::chrono::high_resolution_clock::duration frame_time;
	std::chrono::high_resolution_clock::time_point decoding_start_time;

    std::chrono::high_resolution_clock::duration estimate_sleep_time = std::chrono::milliseconds(1);
    std::chrono::high_resolution_clock::duration avg_sleep_time = std::chrono::milliseconds(1);
    std::chrono::high_resolution_clock::duration m2 = std::chrono::milliseconds(0);
    int count = 1;

    void sleep(std::chrono::high_resolution_clock::duration remaining_sleep_time);
};

}
