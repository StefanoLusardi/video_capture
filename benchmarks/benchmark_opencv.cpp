/**
 * banchmark: 	benchmark_opencv
 * author:		Stefano Lusardi
 * date:		Jul 2021
 * description:	Comparison between OpenCV::VideoCapture and cv::video_capture. 
*/

#include <iostream>
#include <video_capture/video_capture.hpp>
#include <opencv2/videoio.hpp>
#include <benchmark/cppbenchmark.h>

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
	cv::Mat frame(w, h, CV_8UC3);

	while(vc.read(frame))
	{
	}

	vc.release();
}

void run_video_capture(const char* video_path, vc::decode_support decode_support)
{
	vc::video_capture vc;
	if(!vc.open(video_path, decode_support))
	{
		std::cout << "Unable to open " << video_path << std::endl;
		return;
	}

	const auto size = vc.get_frame_size(); 
	const auto [w, h] = size.value();
	cv::Mat frame(h, w, CV_8UC3);
	
	// TODO: fixme -- CRASH!
	while(vc.next(&frame.data))
	{
	}

	vc.release();
}

const auto video_path = "../../../../tests/data/testsrc_10sec_30fps.mkv";

BENCHMARK("video_capture_SW")
{
    run_video_capture(video_path, vc::decode_support::SW);
}

BENCHMARK("video_capture_HW")
{
    run_video_capture(video_path, vc::decode_support::HW);
}

BENCHMARK("OpenCV")
{
    run_opencv(video_path);
}

BENCHMARK_MAIN()
