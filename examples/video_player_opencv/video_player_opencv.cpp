#include <memory>
#include <iostream>
#include <thread>

#define VIDEO_CAPTURE_LOG_ENABLED 1
#include <video_capture/video_capture.hpp> // Custom FFMPEG backend
#include <opencv2/highgui.hpp> // OpenCV GUI

void logger_info(const std::string& str)  { std::cout << "[  INFO ] " << str << std::endl; }
void logger_error(const std::string& str) { std::cout << "[ ERROR ] " << str << std::endl; }

int main(int argc, char** argv)
{
	std::string video_path;
	if (argc < 2)
	{
		std::cout << "Missing video file path." << std::endl;
		std::cout << "Usage: \nvideo_player_opencv <VIDEO_PATH>" << std::endl;
		video_path = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
		std::cout << "Using default RTSP video stream: " << video_path << std::endl;
	}
	else
	{
		video_path = argv[1];
		std::cout << "Using video file: " << video_path << std::endl;
	}

	vc::video_capture vc;
	vc.set_log_callback(logger_info, vc::log_level::info);
	vc.set_log_callback(logger_error, vc::log_level::error);

	if(!vc.open(video_path))
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

	// const auto sleep_time = static_cast<int>(1000.f/fps.value());
	const auto sleep_time = std::chrono::nanoseconds(static_cast<int>(1'000'000'000/fps.value()));
	// const auto sleep_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(sleep_time);
	// auto start_time = std::chrono::steady_clock::now();
	while(true)
	{
		auto start_time = std::chrono::steady_clock::now();
		if(!vc.next(&frame.data))
			break;


		cv::imshow("FFMPEG Video Player with OpenCV UI", frame);
		cv::waitKey(1);

		if(auto d = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count(); d > 16)
			std::cout << "[D:] " << d << std::endl;

		// auto elapsed_time = std::chrono::steady_clock::now() - start_time;
		// if (elapsed_time < sleep_time)
		// {
		// 	auto wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(sleep_time - elapsed_time);
		// 	cv::waitKey(wait_time.count());
		// }
		// else
		// 	cv::waitKey(1);
		// start_time = std::chrono::steady_clock::now();
	}

	vc.release();
	cv::destroyAllWindows();
	
	return 0;
}