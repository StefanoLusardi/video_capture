/**
 * banchmark: 	benchmark_opencv
 * author:		Stefano Lusardi
 * date:		Jul 2021
 * description:	Comparison between OpenCV::VideoCapture and cv::video_capture. 
*/

#include <iostream>
#include <video_capture/video_capture.hpp>
#include <video_capture/raw_frame.hpp>
#include <opencv2/videoio.hpp>
#include <benchmark/cppbenchmark.h>


const auto video_path = "../../../../tests/data/v.mp4";
// const auto video_path = "../../../../tests/data/testsrc_30sec_30fps.mkv";

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

		if(!_vc.isOpened())
		{
			std::cout << "cv::VideoCapture is not opened" << std::endl;
			context.Cancel();
			return;
		}
	}

    void Cleanup(CppBenchmark::Context& context) override 
	{ 
		_vc.release();
	}

	void Run(CppBenchmark::Context& context) override
	{	
		while(_vc.read(_frame))
		{
		}
	}
};

class VideoCaptureFixture_RawData : public CppBenchmark::Benchmark
{
public:
    using Benchmark::Benchmark;

protected:
	vc::video_capture _vc;
	uint8_t* _data = {};

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

		if(!_vc.is_opened())
		{
			std::cout << "vc::video_capture is not opened" << std::endl;
			context.Cancel();
			return;
		}
	}

    void Cleanup(CppBenchmark::Context& context) override 
	{ 
		_vc.release();
	}

	void Run(CppBenchmark::Context& context) override
	{	
		while(_vc.read(&_data))
		{
		}
	}
};

class VideoCaptureFixture_RawFrame : public CppBenchmark::Benchmark
{
public:
    using Benchmark::Benchmark;

protected:
	vc::video_capture _vc;
	vc::raw_frame _frame;

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

		if(!_vc.is_opened())
		{
			std::cout << "vc::video_capture is not opened" << std::endl;
			context.Cancel();
			return;
		}

	    _frame.data.resize(_vc.get_frame_size_in_bytes().value());
	}

    void Cleanup(CppBenchmark::Context& context) override 
	{ 
		_vc.release();
	}

	void Run(CppBenchmark::Context& context) override
	{	
		while(_vc.read(&_frame))
		{
		}
	}
};

const auto attempts = 1;
const auto operations = 1;

BENCHMARK_CLASS(VideoCaptureFixture_RawData,
	"VideoCaptureFixture.RawData.SW",
	Settings().Attempts(attempts).Operations(operations).Param(static_cast<int>(vc::decode_support::SW)))

BENCHMARK_CLASS(VideoCaptureFixture_RawFrame,
	"VideoCaptureFixture.RawFrame.SW", 
	Settings().Attempts(attempts).Operations(operations).Param(static_cast<int>(vc::decode_support::SW)))

// BENCHMARK_CLASS(VideoCaptureFixture_OpenCV,		
// 	"VideoCaptureFixture.OpenCV",
// 	Settings().Attempts(attempts).Operations(operations))

BENCHMARK_CLASS(VideoCaptureFixture_RawData,
	"VideoCaptureFixture.RawData.HW",
	Settings().Attempts(attempts).Operations(operations).Param(static_cast<int>(vc::decode_support::HW)))

BENCHMARK_CLASS(VideoCaptureFixture_RawFrame,
	"VideoCaptureFixture.RawFrame.HW", 
	Settings().Attempts(attempts).Operations(operations).Param(static_cast<int>(vc::decode_support::HW)))

BENCHMARK_MAIN()

