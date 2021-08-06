/**
 * example: 	video_player_opencv
 * author:		Stefano Lusardi
 * date:		Jun 2021
 * description:	Example to show how to integrate cv::video_capture in a simple video player based on OpenCV framework. 
 * 				Single threaded: Main thread decodes and draws subsequent frames.
 * 				Note that this serves only as an example, as in real world application 
 * 				you might want to handle decoding and rendering on separate threads (see any video_player_xxx_multi_thread).
*/

#include <memory>
#include <iostream>
#include <thread>

#include <video_capture/video_capture.hpp>
#include <opencv2/highgui.hpp>

void cb_log_info(const std::string& str)  { std::cout << "[  INFO ] " << str << std::endl; }
void cb_log_error(const std::string& str) { std::cout << "[ ERROR ] " << str << std::endl; }

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
	vc.set_log_callback(cb_log_info, vc::log_level::info);
	vc.set_log_callback(cb_log_error, vc::log_level::error);

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

	while(true)
	{
		if(!vc.read(&frame.data))
			break;

		cv::imshow(window_title, frame);
		cv::waitKey(1);
		++n_frames;
	}

	auto total_end_time = std::chrono::high_resolution_clock::now();
	std::cout << "Decode time: " << std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count() << "ms" << std::endl;
	std::cout << "Decoded Frames: " << n_frames << std::endl;

	vc.release();
	cv::destroyAllWindows();
	
	return 0;
}