#include "window.hpp"

#include <stdexcept>

Window::Window(int w, int h, std::string title) : width{ w }, height{ h }, title{ title } {
	initWindow();
}

Window::~Window() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

void
Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
	if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface");
	}
}

void
Window::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Do not create OpenGL context

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(window, this); // Store pointer to this Window instance in the GLFWwindow object
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void
Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	Window *window_container = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	window_container->frame_buffer_resized = true;
	window_container->width = width;
	window_container->height = height;
}
