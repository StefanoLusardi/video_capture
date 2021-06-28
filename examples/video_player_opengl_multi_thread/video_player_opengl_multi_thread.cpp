#include <memory>
#include <iostream>
#include <thread>
#include <atomic>
#include  <cstring>

#include <video_capture/video_capture.hpp>
#include <video_capture/frame_queue.hpp>

#include <GLFW/glfw3.h>

using namespace std::chrono_literals;

struct raw_frame
{
	std::vector<uint8_t> data;
	double pts = 0.0;
};

void decode_thread(vc::video_capture& vc, vc::frame_queue<raw_frame*>& frame_queue, uint frame_size_byte)
{
	int frames_decoded = 0;
	while(true)
	{
		std::this_thread::yield();
		// std::this_thread::sleep_for(10ms);

		uint8_t* data = {};
		raw_frame* frame = new raw_frame();
		if (!vc.next(&data, &frame->pts))
		{
			std::cout << "Video finished" << std::endl;
			std::cout << "frames decoded: " << frames_decoded << std::endl;
			break;
		}

		frame->data = std::vector<uint8_t>(frame_size_byte);
		std::memcpy(frame->data.data(), data, frame_size_byte);

		frame_queue.put(frame);
		++frames_decoded;
	}
}

bool setup_opengl(GLFWwindow** window, GLuint& texture_handle, int frame_width, int frame_height)
{
	if (!glfwInit())
	{
		std::cout << "Couldn't init GLFW" << std::endl;
		return false;
	}

	*window = glfwCreateWindow(frame_width, frame_height, "Video Player OpenGL", NULL, NULL);
	if (!window)
	{
		std::cout << "Couldn't open window" << std::endl;
		return false;
	}

	glfwMakeContextCurrent(*window);

	// Generate texture
	glGenTextures(1, &texture_handle);
	glBindTexture(GL_TEXTURE_2D, texture_handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	int window_width, window_height;
	glfwGetFramebufferSize(*window, &window_width, &window_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, window_width, window_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	glfwSetTime(0.0);

	return true;
}

void draw_frame(GLFWwindow *window, GLuint& texture_handle, int frame_width, int frame_height, uint8_t* frame_data)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame_data);

	// Render whatever you want
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_handle);
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

int main(int argc, char **argv)
{
	std::cout << "GLFW version: " << glfwGetVersionString() << std::endl;
	vc::video_capture vc;
	const auto video_path = "../../../../tests/data/testsrc_120sec_30fps.mkv";

	vc.open(video_path, vc::decode_support::HW);

	const auto fps = vc.get_fps();
	const auto frame_size = vc.get_frame_size();
	const auto [frame_width, frame_height] = frame_size.value();

	auto total_start_time = std::chrono::high_resolution_clock::now();
	auto total_end_time = std::chrono::high_resolution_clock::now();

	uint frame_size_byte = frame_width * frame_height * 3;
	vc::frame_queue<raw_frame*> frame_queue(10);
	std::thread t(&decode_thread, std::ref(vc), std::ref(frame_queue), frame_size_byte);

	GLFWwindow *window = nullptr;
	GLuint texture_handle;

	if(!setup_opengl(&window, texture_handle, frame_width, frame_height))
		return EXIT_FAILURE;

	int frames_shown = 0;
	raw_frame* frame = nullptr;

	while (!glfwWindowShouldClose(window))
	{
		if(!frame_queue.try_get(&frame))
			break;

		while (frame->pts > glfwGetTime())
		{
			if(const auto timeout = frame->pts - glfwGetTime(); timeout > 0.0)
				glfwWaitEventsTimeout(timeout);
		}

		draw_frame(window, texture_handle, frame_width, frame_height, frame->data.data());
		++frames_shown;
		delete frame;
	}

	total_end_time = std::chrono::high_resolution_clock::now();
	std::cout << "Decode time: " << std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count() << "ms" << std::endl;
	std::cout << "frames shown:   " << frames_shown << std::endl;
	
	t.join();
	vc.release();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}