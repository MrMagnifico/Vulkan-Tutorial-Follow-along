#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

/// <summary>
/// Thin wrapper class around a GLFW window
/// </summary>
class Window {
public:
	/// <summary>
	/// Create a new Window object and initialise its underlying GLFW window
	/// </summary>
	/// <param name="width">Width of the window in pixels</param>
	/// <param name="height">Height of the window in pixels</param>
	/// <param name="title">Title of the window</param>
	Window(int width, int height, std::string title);
	~Window();

	/// <summary>
	/// Wrapper over GLFW functionality to check if the window should be closed
	/// </summary>
	/// <returns>Boolean indicating if the window should be closed</returns>
	bool shouldClose() { return glfwWindowShouldClose(window); }

	/// <summary>
	/// Creates a window surface for Vulkan to render onto
	/// </summary>
	/// <param name="instance">Vulkan application instance</param>
	/// <param name="surface">Pointer to struct to store surface data</param>
	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	void resetWindowResizedFlag() { frame_buffer_resized = false; }

	int getWidth() { return width; }
	int getHeight() { return height; }
	VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
	std::string getTitle() { return title; }
	GLFWwindow* getGlfwWindowPtr() { return window; }
	bool wasWindowResized() { return frame_buffer_resized; }

private:
	int width;
	int height;
	std::string title;
	GLFWwindow* window;
	bool frame_buffer_resized = false;

	/// <summary>
	/// Initialise the internal GLFW window
	/// </summary>
	void initWindow();

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};
