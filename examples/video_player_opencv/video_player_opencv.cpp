#include <memory>
#include <iostream>
#include <thread>

#define VIDEO_CAPTURE_LOG_ENABLED 1
#include <video_capture/video_capture.hpp> // Custom FFMPEG backend
#include <opencv2/highgui.hpp> // OpenCV GUI

void log_info(const std::string& str)  { std::cout << "[  INFO ] " << str << std::endl; }
void log_error(const std::string& str) { std::cout << "[ ERROR ] " << str << std::endl; }

void sleep(std::chrono::high_resolution_clock::duration remaining_sleep_time) 
{
    using namespace std;
    using namespace chrono;

    static std::chrono::high_resolution_clock::duration estimate_sleep_time = milliseconds(1);
    static std::chrono::high_resolution_clock::duration avg_sleep_time = milliseconds(1);
    static std::chrono::high_resolution_clock::duration m2 = milliseconds(0);
    static int64_t count = 1;

    while (remaining_sleep_time > estimate_sleep_time)
	{
        auto start = high_resolution_clock::now();
        this_thread::sleep_for(milliseconds(1));
        auto end = high_resolution_clock::now();

        auto measured_sleep = end - start;
        remaining_sleep_time -= measured_sleep;

        ++count;
        auto delta = measured_sleep - avg_sleep_time;
        avg_sleep_time += delta / count;
		m2   += delta.count() * (measured_sleep - avg_sleep_time);
        double stddev = sqrt(m2.count() / (count - 1));
        estimate_sleep_time = avg_sleep_time + std::chrono::nanoseconds((int)stddev);
    }

    auto start_spin_lock = high_resolution_clock::now();
    while (high_resolution_clock::now() - start_spin_lock < remaining_sleep_time)
		; // spin lock
}

void sleep(double seconds) 
{
    using namespace std;
    using namespace chrono;

    static double estimate = 5e-3;
    static double mean = 5e-3;
    static double m2 = 0;
    static int64_t count = 1;

    while (seconds > estimate)
	{
        auto start = high_resolution_clock::now();
        this_thread::sleep_for(milliseconds(1));
        auto end = high_resolution_clock::now();

        double observed = (end - start).count() / 1e9;
        seconds -= observed;

        ++count;
        double delta = observed - mean;
        mean += delta / count;
        m2   += delta * (observed - mean);
        double stddev = sqrt(m2 / (count - 1));
        estimate = mean + stddev;
    }

    // spin lock
    auto start = high_resolution_clock::now();
    while ((high_resolution_clock::now() - start).count() / 1e9 < seconds);
}

int main(int argc, char** argv)
{
	std::string video_path;
	if (argc < 2)
	{
		std::cout << "Missing video file path." << std::endl;
		std::cout << "Usage: \nvideo_player_opencv <VIDEO_PATH>" << std::endl;
		// video_path = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
		video_path = "../../../../tests/data/testsrc_10sec_30fps.mkv";
		std::cout << "Using default RTSP video stream: " << video_path << std::endl;
	}
	else
	{
		video_path = argv[1];
		std::cout << "Using video file: " << video_path << std::endl;
	}

	video_path = "../../../../tests/data/testsrc_120sec_30fps.mkv";

	vc::video_capture vc;
	vc.set_log_callback(log_info, vc::log_level::info);
	vc.set_log_callback(log_error, vc::log_level::error);

	if(!vc.open(video_path, vc::decode_support::SW))
	{
		std::cout << "Unable to open " << video_path << std::endl;
		return -1;
	}

	const auto size = vc.get_frame_size(); 
	if(!size)
	{
		std::cout << "Unable to retrieve frame size from " << video_path << std::endl;
		return -1;
	}

	const auto fps = vc.get_fps();
	if(!fps)
	{
		std::cout << "Unable to retrieve FPS from " << video_path << std::endl;
		return -1;
	}

	// NOTE: height is the number of cv::Mat rows, width is the number of cv::Mat cols.
	const auto [w, h] = size.value();
	cv::Mat frame(h, w, CV_8UC3); 

	// const auto frame_time = std::chrono::milliseconds(static_cast<int>(1'000/fps.value()));
	const auto frame_time = std::chrono::nanoseconds(static_cast<int>(1'000'000'000/fps.value()));
	
	const std::string window_title = "FFMPEG Video Player with OpenCV UI";
	cv::namedWindow(window_title);

	int n_frames = 0;
	auto total_start_time = std::chrono::high_resolution_clock::now();

	auto decoding_start_time = std::chrono::high_resolution_clock::now();
	// auto decoding_start_time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now());
	while(true)
	{
		if(!vc.next(&frame.data))
			break;

		cv::imshow(window_title, frame);
		cv::waitKey(1);
		++n_frames;

		auto decoding_end_time = std::chrono::high_resolution_clock::now();
		// auto decoding_end_time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now());
		auto decoding_time = decoding_end_time - decoding_start_time;
		auto sleep_time = frame_time - decoding_time;
		sleep(sleep_time);
		// sleep(sleep_time.count()/1'000'000'000.0);
		// std::this_thread::sleep_for(sleep_time);
		decoding_start_time = std::chrono::high_resolution_clock::now();
		// decoding_start_time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now());
	}

	auto total_end_time = std::chrono::high_resolution_clock::now();
	std::cout << "Decode time: " << std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count() << "ms" << std::endl;
	std::cout << "Decoded Frames: " << n_frames << std::endl;

	vc.release();
	cv::destroyAllWindows();
	
	return 0;
}