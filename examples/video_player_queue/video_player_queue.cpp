#include <memory>
#include <iostream>
#include <thread>

#define VIDEO_CAPTURE_LOG_ENABLED 1
#include <video_capture/video_capture.hpp> 
#include <video_capture/frame_sync.hpp>
#include <video_capture/frame_queue.hpp>
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
		video_path =
		"rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_175k.mov";
		//  "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
		std::cout << "Using default RTSP video stream: " << video_path << std::endl;
	}
	else
	{
		video_path = argv[1];
		std::cout << "Using video file: " << video_path << std::endl;
	}

	video_path = "../../../../tests/data/testsrc_120sec_30fps.mkv";

	vc::video_capture vc;
	vc.set_log_callback(log_info, vc::log_level::info);
	vc.set_log_callback(log_error, vc::log_level::error);

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

	// const auto frame_time = std::chrono::milliseconds(static_cast<int>(1'000/fps.value()));
	const auto frame_time = std::chrono::nanoseconds(static_cast<int>(1'000'000'000/fps.value()));
	
	const std::string window_title = "FFMPEG Video Player with OpenCV UI";
	cv::namedWindow(window_title);

	int n_decode_frames = 0;
	vc::frame_queue<cv::Mat> fq;

	auto decode_thread = std::thread([&] ()
	{
		using namespace std::chrono_literals;
		uint8_t* frame_data = {};
		const auto [w, h] = size.value();
		while(true)
		{
			if(!vc.next(&frame_data))
				break;

			cv::Mat frame(cv::Size(w, h), CV_8UC3, frame_data); 
			
			fq.put(frame.clone());
			++n_decode_frames;
			std::this_thread::sleep_for(1ms);
		}
	});

	std::this_thread::sleep_for(std::chrono::seconds(1));

	vc::frame_sync fs = vc::frame_sync(frame_time);
	
	// NOTE: height is the number of cv::Mat rows, width is the number of cv::Mat cols.
	const auto [w, h] = size.value();
	int n_show_frames = 0;
	cv::Mat frame(cv::Size(w, h), CV_8UC3); 

	auto total_start_time = std::chrono::high_resolution_clock::now();
	fs.start();
	while(!fq.is_empty())
	{
		fq.get(&frame);
		cv::imshow(window_title, frame);
		cv::waitKey(1);
		++n_show_frames;
		fs.update();
		// std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}

	auto total_end_time = std::chrono::high_resolution_clock::now();
	std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count() << "ms" << std::endl;
	std::cout << "Decoded Frames: " << n_decode_frames << std::endl;
	std::cout << "Shown Frames: " << n_show_frames << std::endl;

	vc.release();
	cv::destroyAllWindows();
	decode_thread.join();
	
	return 0;
}