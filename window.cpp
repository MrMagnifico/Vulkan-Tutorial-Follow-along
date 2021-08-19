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
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Do not make window resizable

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}
