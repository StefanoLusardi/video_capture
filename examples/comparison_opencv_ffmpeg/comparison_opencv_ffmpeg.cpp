/**
 * example: 	simple_decode
 * author:		Stefano Lusardi
 * date:		Jun 2021
 * description:	Comparison between OpenCV::VideoCapture and cv::video_capture. 
 * 				Public APIs are very similar, while private ones are simplified quite a lot.
 * 				The frames are using OpenCV::imshow in both examples.
 * 				Note that sleep time between consecutive frames is not accurate here, 
 * 				see any video_player_xxx example for a more accurate playback.
*/

#include <memory>
#include <iostream>
#include <thread>

#include <video_capture/video_capture.hpp>
#include <video_capture/raw_frame.hpp>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

void run_opencv(const char* video_path, const char* name)
{
	cv::VideoCapture vc;
	if(!vc.open(video_path))
	{
		std::cout << "Unable to open " << video_path << std::endl;
		return;
	}

	const auto w = vc.get(cv::CAP_PROP_FRAME_WIDTH);
	const auto h = vc.get(cv::CAP_PROP_FRAME_HEIGHT);
	const auto fps = vc.get(cv::CAP_PROP_FPS);
	const auto sleep_time = static_cast<int>(fps);
	cv::Mat frame(w, h, CV_8UC3);

	auto start = std::chrono::steady_clock::now();
	while(vc.read(frame))
	{
		cv::imshow(name, frame);
		cv::waitKey(sleep_time);
	}
	auto end = std::chrono::steady_clock::now();

	std::cout << name << " - " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << std::endl;

	vc.release();
	cv::destroyWindow(name);
}

void run_ffmpeg(const char* video_path, const char* name, vc::decode_support decode_support)
{
	vc::video_capture vc;
	if(!vc.open(video_path, decode_support))
	{
		std::cout << "Unable to open " << video_path << std::endl;
		return;
	}

	const auto size = vc.get_frame_size(); 
	const auto [w, h] = size.value();
	const auto fps = vc.get_fps();
	const auto sleep_time = static_cast<int>(fps.value_or(1));
	cv::Mat frame(h, w, CV_8UC3);

	auto start = std::chrono::steady_clock::now();
	while(vc.read(&frame.data))
	{
		cv::imshow(name, frame);
		cv::waitKey(sleep_time);
	}
	auto end = std::chrono::steady_clock::now();
	
	std::cout << name << " - " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << std::endl;

	vc.release();
	cv::destroyWindow(name);
}

int main(int argc, char** argv)
{
	// const auto video_path = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
	const auto video_path = "../../../../tests/data/testsrc_10sec_30fps.mkv";
	
	run_opencv(video_path, "OpenCV");
	run_ffmpeg(video_path, "Lib SW", vc::decode_support::SW);

	// std::thread opencv_thread([video_path]{run_opencv(video_path, "OpenCV");});
	// std::thread ffmpeg_thread_sw([video_path]{run_ffmpeg(video_path, "Lib SW", vc::decode_support::SW);});
	
	// if(ffmpeg_thread_sw.joinable())
	// 	ffmpeg_thread_sw.join();
	
	// if(opencv_thread.joinable())
	// 	opencv_thread.join();

	return 0;
}