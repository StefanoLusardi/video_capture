#include <frame_sync.hpp>
#include <thread>

namespace vc
{
frame_sync::frame_sync(std::chrono::high_resolution_clock::duration tick)
{
    frame_time = tick;
}

frame_sync::~frame_sync()
{
    stop();
}

void frame_sync::start()
{
    decoding_start_time = std::chrono::high_resolution_clock::now();
}

void frame_sync::stop()
{
    estimate_sleep_time = std::chrono::milliseconds(1);
    avg_sleep_time = std::chrono::milliseconds(1);
    m2 = std::chrono::milliseconds(0);
    count = 1;
}

void frame_sync::update()
{
    auto decoding_end_time = std::chrono::high_resolution_clock::now();
    auto decoding_time = decoding_end_time - decoding_start_time;
    auto sleep_time = frame_time - decoding_time;
    sleep(sleep_time);
    decoding_start_time = std::chrono::high_resolution_clock::now();
}

void frame_sync::sleep(std::chrono::high_resolution_clock::duration remaining_sleep_time) 
{
    while (remaining_sleep_time > estimate_sleep_time)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto end = std::chrono::high_resolution_clock::now();

        auto measured_sleep = end - start;
        remaining_sleep_time -= measured_sleep;

        ++count;
        auto delta = measured_sleep - avg_sleep_time;
        avg_sleep_time += delta / count;
        m2   += delta.count() * (measured_sleep - avg_sleep_time);
        double stddev = sqrt(m2.count() / (count - 1));
        estimate_sleep_time = avg_sleep_time + std::chrono::nanoseconds((int)stddev);
    }

    auto start_spin_lock = std::chrono::high_resolution_clock::now();
    while (std::chrono::high_resolution_clock::now() - start_spin_lock < remaining_sleep_time)
        ; // spin lock
}

}
