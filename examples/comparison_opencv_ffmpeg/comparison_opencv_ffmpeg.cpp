/**
 * example: 	simple_decode
 * author:		Stefano Lusardi
 * date:		Jun 2021
 * description:	Comparison between OpenCV::VideoCapture and cv::video_capture. 
 * 				API is very similar, yet simplified quite a lot.
 * 				The frames are using OpenCV::imshow in both examples.
 * 				Note that sleep time between consecutive frames is not accurate here, 
 * 				see any video_player_xxx example for a more accurate playback.
*/

#include <memory>
#include <iostream>
#include <thread>

#define VIDEO_CAPTURE_LOG_ENABLED 1
#include <video_capture/video_capture.hpp> // Custom FFMPEG backend
#include <opencv2/highgui.hpp> // OpenCV GUI

void run_opencv(const char* video_path)
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
	// cv::Mat frame;

	while(vc.read(frame))
	{
		cv::imshow("OpenCV", frame);
		cv::waitKey(sleep_time);
	}

	vc.release();
	cv::destroyWindow("OpenCV");
}

void run_ffmpeg(const char* video_path, vc::decode_support decode_support, const char* name)
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
	
	while(vc.next(&frame.data))
	{
		cv::imshow(name, frame);
		cv::waitKey(sleep_time);
	}

	vc.release();
	cv::destroyWindow(name);
}

int main(int argc, char** argv)
{
	auto video_path = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
	
	// if (argc < 1)
	// 	return -1;

	// auto video_path = argv[1];
	
	// std::thread ffmpeg_thread_hw([video_path]{run_ffmpeg(video_path, vc::decode_support::HW, "HW");});
	std::thread ffmpeg_thread_sw([video_path]{run_ffmpeg(video_path, vc::decode_support::SW, "SW");});
	std::thread opencv_thread([video_path]{run_opencv(video_path);});
	
	// if(ffmpeg_thread_hw.joinable())
	// 	ffmpeg_thread_hw.join();
	
	if(ffmpeg_thread_sw.joinable())
		ffmpeg_thread_sw.join();
		
	if(opencv_thread.joinable())
		opencv_thread.join();

	return 0;
}