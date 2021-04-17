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

	const auto sleep_time = std::chrono::milliseconds(static_cast<int>(1000/fps.value()));
	
	const std::string window_title = "FFMPEG Video Player with OpenCV UI";
	cv::namedWindow(window_title);

	auto start_time = std::chrono::steady_clock::now();
	while(true)
	{
		if(!vc.next(&frame.data))
			break;

		cv::imshow(window_title, frame);
		cv::waitKey(1);

		auto elapsed_time = std::chrono::steady_clock::now() - start_time;
		if(elapsed_time > sleep_time)
		{
			// std::cout << "[D] " << elapsed_time.count() << std::endl;
		}
		else
		{
			// std::cout << "[OK]" << std::endl;
			auto wait_time = sleep_time - elapsed_time;
			std::this_thread::sleep_for(wait_time);
			// auto wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(sleep_time - elapsed_time).count();
			// cv::waitKey(wait_time);
		}
		start_time = std::chrono::steady_clock::now();
	}

	vc.release();
	cv::destroyAllWindows();
	
	return 0;
}