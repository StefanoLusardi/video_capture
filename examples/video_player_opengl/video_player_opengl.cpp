/**
 * example: 	video_player_opengl
 * author:		Stefano Lusardi
 * date:		Jun 2021
 * description:	Example to show how to integrate cv::video_capture in a simple video player based on OpenGL (using GLFW). 
 * 				Single threaded: Main thread decodes and draws subsequent frames.
 * 				Note that this serves only as an example, as in real world application 
 * 				you might want to handle decoding and rendering on separate threads (see any video_player_xxx_multi_thread).
*/

#include <memory>
#include <iostream>
#include <thread>
#include <atomic>

#include <video_capture/video_capture.hpp>
#include <video_capture/raw_frame.hpp>

#include <GLFW/glfw3.h>

using namespace std::chrono_literals;


double get_elapsed_time()
{
	static std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>> start_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_time = std::chrono::steady_clock::now() - start_time;
	return elapsed_time.count();
}

int main(int argc, char **argv)
{
	std::cout << "GLFW version: " << glfwGetVersionString() << std::endl;
	vc::video_capture vc;
	const auto video_path = "../../../../tests/data/testsrc_120sec_30fps.mkv";

	vc.open(video_path, vc::decode_support::HW);

	const auto fps = vc.get_fps();
	const auto size = vc.get_frame_size();
	const auto [frame_width, frame_height] = size.value();

	if (!glfwInit())
	{
		std::cout << "Couldn't init GLFW" << std::endl;
		return 1;
	}

	GLFWwindow *window = glfwCreateWindow(frame_width, frame_height, "Video Player OpenGL", NULL, NULL);
	if (!window)
	{
		std::cout << "Couldn't open window" << std::endl;
		return 1;
	}

	glfwMakeContextCurrent(window);

	GLuint tex_handle;
	glGenTextures(1, &tex_handle);
	glBindTexture(GL_TEXTURE_2D, tex_handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>> start_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_time(0.0);
	
	auto total_start_time = std::chrono::high_resolution_clock::now();
	auto total_end_time = std::chrono::high_resolution_clock::now();

	int window_width, window_height;
	glfwGetFramebufferSize(window, &window_width, &window_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, window_width, window_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	std::unique_ptr<vc::raw_frame> frame;
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		frame = std::make_unique<vc::raw_frame>();
		if (!vc.next_frame(frame.get()))
		{
			total_end_time = std::chrono::high_resolution_clock::now();
			std::cout << "Couldn't load video frame" << std::endl;
			std::cout << "Video finished" << std::endl;
			break;
		}

		if (const auto timeout = frame->pts - get_elapsed_time(); timeout > 0.0)
			std::this_thread::sleep_for(std::chrono::duration<double>(timeout));

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame->data.data());

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_handle);
		glBegin(GL_QUADS);

		glTexCoord2d(0, 0);
		glVertex2i(0, 0);

		glTexCoord2d(1, 0);
		glVertex2i(frame_width, 0);

		glTexCoord2d(1, 1);
		glVertex2i(frame_width, frame_height);

		glTexCoord2d(0, 1);
		glVertex2i(0, frame_height);

		glEnd();
		glDisable(GL_TEXTURE_2D);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	std::cout << "Decode time: " << std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count() << "ms" << std::endl;
	
	vc.release();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}