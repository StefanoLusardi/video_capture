#include <memory>
#include <iostream>
#include <thread>

#include <opencv2/highgui.hpp>
// #include <opencv2/videoio.hpp> // OpenCV backend
// #include <opencv2/imgproc.hpp> // OpenCV backend

#define VIDEO_CAPTURE_LOG_ENABLED 1
#include <video_capture/video_capture.hpp> // Custom FFMPEG backend


using namespace std::chrono_literals;

void run_opencv(const char* video_path)
{
	cv::VideoCapture vc;

	if(!vc.open(video_path))
	{
		std::cout << "Unable to open" << std::endl;
		return;
	}

	cv::Mat frame;

	auto start = std::chrono::high_resolution_clock::now();
	while(vc.read(frame))
	{
		cv::imshow("OpenCV", frame);
		cv::waitKey(1);
	}
	auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
	std::cout << "OpenCV: " << ms_int.count() << "ms\n";

	vc.release();
	cv::destroyWindow("OpenCV");
}

void logger_info(const std::string& str)  { std::cout << "[  INFO ] " << str << std::endl; }
void logger_error(const std::string& str) { std::cout << "[ ERROR ] " << str << std::endl; }

void run_ffmpeg(const char* video_path, vc::decode_support decode_support, const char* name)
{
	vc::video_capture vc;
	vc.set_info_callback(logger_info);
	vc.set_error_callback(logger_error);
	// vc.set_log_callback(logger_all);

	if(!vc.open(video_path, decode_support))
	{
		std::cout << "Unable to open " << video_path << std::endl;
		return;
	}

	auto [w, h] = vc.get_frame_size();
	cv::Mat frame(h, w, CV_8UC3);
	
	auto start = std::chrono::high_resolution_clock::now();
	while(vc.next(&frame.data))
	{
		cv::imshow(name, frame);
		cv::waitKey(1);
	}
	auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
	std::cout << name << ": " << ms_int.count() << "ms\n";

	vc.release();
	cv::destroyWindow(name);
}

int main(int argc, char** argv)
{
	auto video_path = "C:/Users/StefanoLusardi/Desktop/test_4.mp4";
	
	// if (argc < 1)
	// 	return -1;

	// auto video_path = argv[1];
	
	std::thread ffmpeg_thread_sw([video_path]{run_ffmpeg(video_path, vc::decode_support::SW, "SW");});
	// std::thread ffmpeg_thread_hw([video_path]{run_ffmpeg(video_path, vc::decode_support::HW, "HW");});
	// std::thread opencv_thread([video_path]{run_opencv(video_path);});
	
	if(ffmpeg_thread_sw.joinable())
		ffmpeg_thread_sw.join();
		
	// if(ffmpeg_thread_hw.joinable())
	// 	ffmpeg_thread_hw.join();
	
	// if(opencv_thread.joinable())
	// 	opencv_thread.join();
	
	return 0;
}