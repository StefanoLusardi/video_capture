#include <video_capture_test.hpp>

namespace vc::test
{

// Test Error callback using a free function
std::string error_msg = std::string();
void error_cb(const std::string& s)
{
    error_msg = s;
}

TEST_F(video_capture_test, error_callback)
{ 
    vc->set_log_callback(&error_cb, vc::log_level::error);
    const auto fc = vc->get_frame_count();
    ASSERT_EQ(error_msg, "Video path must be opened first.");
}

TEST_F(video_capture_test, info_callback)
{ 
    // Test Info callback using a lambda
    std::string info_msg = std::string();
    auto info_cb = [&](const std::string& s){ 
        info_msg = s;
    };

    vc->set_log_callback(info_cb, vc::log_level::info);
    vc->open("rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov");
    ASSERT_EQ(info_msg, "Video Capture is initialized");
}

/*
TEST_F(video_capture_test, all_callback){ }

TEST_F(video_capture_test, open_default_decode){ }
TEST_F(video_capture_test, open_sw_decode){ }
TEST_F(video_capture_test, open_hw_decode){ }

TEST_F(video_capture_test, get_frame_count){ }
TEST_F(video_capture_test, get_duration){ }
TEST_F(video_capture_test, get_frame_size){ }
TEST_F(video_capture_test, get_frame_size_in_bytes){ }
TEST_F(video_capture_test, get_fps){ }

TEST_F(video_capture_test, next){ }
TEST_F(video_capture_test, next_frame){ }

TEST_F(video_capture_test, release){ }
*/
}