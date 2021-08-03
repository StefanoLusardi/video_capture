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


const auto video_path = "../../../../tests/data/testsrc_10sec_30fps.mkv";

class VideoCaptureFixture_OpenCV : public CppBenchmark::Benchmark
{
public:
    using Benchmark::Benchmark;

protected:
    cv::VideoCapture _vc;
	cv::Mat _frame;

    void Initialize(CppBenchmark::Context& context) override
	{
		if(!_vc.open(video_path))
		{
			std::cout << "Unable to open " << video_path << std::endl;
			context.Cancel();
			return;
		}

   		const auto w = _vc.get(cv::CAP_PROP_FRAME_WIDTH);
		const auto h = _vc.get(cv::CAP_PROP_FRAME_HEIGHT);
		_frame = cv::Mat(w, h, CV_8UC3);
	}

    void Cleanup(CppBenchmark::Context& context) override 
	{ 
		_vc.release();
	}

	void Run(CppBenchmark::Context& context) override
	{	
		while(_vc.read(_frame))
		{
			// This condition should never be triggered.
			if (_frame.empty())
				context.Cancel();
		}
	}
};

class VideoCaptureFixture_Lib : public CppBenchmark::Benchmark
{
public:
    using Benchmark::Benchmark;

protected:
	vc::video_capture _vc;
	cv::Mat _frame;

    void Initialize(CppBenchmark::Context& context) override
	{
		int param = context.x();
		const vc::decode_support decode_support = static_cast<vc::decode_support>(param);
		if(!_vc.open(video_path, decode_support))
		{
			std::cout << "Unable to open " << video_path << std::endl;
			context.Cancel();
			return;
		}

		const auto size = _vc.get_frame_size(); 
		const auto [w, h] = size.value();
		_frame = cv::Mat(h, w, CV_8UC3);
	}

    void Cleanup(CppBenchmark::Context& context) override 
	{ 
		_vc.release();
	}

	void Run(CppBenchmark::Context& context) override
	{	
		while(_vc.read(&_frame.data))
		{
			// This condition should never be triggered.
			if (_frame.empty())
				context.Cancel();
		}
	}
};

BENCHMARK_CLASS(VideoCaptureFixture_Lib, 
	"VideoCaptureFixture.SW", 
	Settings()
	.Operations(1)
	.Attempts(2)
	.Param(static_cast<int>(vc::decode_support::SW)))

BENCHMARK_CLASS(VideoCaptureFixture_Lib, 
	"VideoCaptureFixture.HW", 
	Settings()
	.Operations(1)
	.Attempts(2)
	.Param(static_cast<int>(vc::decode_support::HW)))

BENCHMARK_CLASS(VideoCaptureFixture_OpenCV, 
	"VideoCaptureFixture.OpenCV", 
	Settings()
	.Operations(1)
	.Attempts(2))
	
BENCHMARK_MAIN()
