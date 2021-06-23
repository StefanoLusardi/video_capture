#include <memory>
#include <iostream>
#include <thread>
#include <atomic>

#include <video_capture/video_capture.hpp>
#include <video_capture/frame_queue.hpp>

#include <GLFW/glfw3.h>

using namespace std::chrono_literals;

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

	// Generate texture
	GLuint tex_handle;
	glGenTextures(1, &tex_handle);
	glBindTexture(GL_TEXTURE_2D, tex_handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	uint8_t *frame_data = {};

	auto total_start_time = std::chrono::high_resolution_clock::now();
	auto total_end_time = std::chrono::high_resolution_clock::now();

	int window_width, window_height;
	glfwGetFramebufferSize(window, &window_width, &window_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, window_width, window_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Read a new frame and load it into texture
		double pts = 0.0;
		if (!vc.next(&frame_data, &pts))
		{
			total_end_time = std::chrono::high_resolution_clock::now();
			std::cout << "Couldn't load video frame" << std::endl;
			break;
		}

		static bool first_frame = true;
		if (first_frame)
		{
			total_start_time = std::chrono::high_resolution_clock::now();
			glfwSetTime(0.0);
			first_frame = false;
		}

		while (pts > glfwGetTime())
		{
			if(const auto timeout = pts - glfwGetTime(); timeout > 0.0)
				glfwWaitEventsTimeout(timeout);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_BGR, GL_UNSIGNED_BYTE, frame_data);

		// Render whatever you want
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