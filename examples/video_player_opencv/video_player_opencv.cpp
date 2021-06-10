#include <memory>
#include <iostream>
#include <thread>

// #define VIDEO_CAPTURE_LOG_ENABLED 1
#include <video_capture/video_capture.hpp> 
#include <video_capture/frame_sync.hpp>
#include <opencv2/highgui.hpp> // OpenCV GUI

void log_info(const std::string& str)  { std::cout << "[  INFO ] " << str << std::endl; }
void log_error(const std::string& str) { std::cout << "[ ERROR ] " << str << std::endl; }

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

	vc::video_capture vc;
	vc.set_log_callback(log_info, vc::log_level::info);
	vc.set_log_callback(log_error, vc::log_level::error);

	if(!vc.open(video_path, vc::decode_support::HW))
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

	const auto frame_time = std::chrono::nanoseconds(static_cast<int>(1'000'000'000/fps.value()));
	
	const std::string window_title = "FFMPEG Video Player with OpenCV UI";
	cv::namedWindow(window_title);

	int n_frames = 0;
	auto total_start_time = std::chrono::high_resolution_clock::now();

	vc::frame_sync fs = vc::frame_sync(frame_time);

	fs.start();
	while(true)
	{
		if(!vc.next(&frame.data))
			break;

		cv::imshow(window_title, frame);
		cv::waitKey(1);
		++n_frames;

		fs.update();
	}

	auto total_end_time = std::chrono::high_resolution_clock::now();
	std::cout << "Decode time: " << std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count() << "ms" << std::endl;
	std::cout << "Decoded Frames: " << n_frames << std::endl;

	vc.release();
	cv::destroyAllWindows();
	
	return 0;
}