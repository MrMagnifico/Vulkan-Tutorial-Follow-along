#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include "window.hpp"

#include <memory>

class CoreApp {
public:
	static const int WIDTH = 640;
	static const int HEIGHT = 480;

	CoreApp();
	~CoreApp();

	/// <summary>
	/// Entrypoint to the application
	/// </summary>
	void run();


	void createPipelineLayout();
	void createPipeline();

	/// <summary>
	/// Print supported instance extensions to stdout
	/// </summary>
	void printSupportedExtensions();

private:
	Window window{ WIDTH, HEIGHT, "Vulkan Tutorial" };
	LogicalDevice vulkan_device{ window };
	SwapChain device_swap_chain{ vulkan_device, window.getExtent() };
	std::unique_ptr<GraphicsPipeline> pipeline;
	VkPipelineLayout pipeline_layout;
};
