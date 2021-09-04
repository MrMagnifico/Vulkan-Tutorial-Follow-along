#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include "window.hpp"

#include <memory>

class CoreApp {
public:
	static constexpr int WIDTH = 640;
	static constexpr int HEIGHT = 480;

	CoreApp();
	~CoreApp();

	/// <summary>
	/// Entrypoint to the application
	/// </summary>
	void run();

	/// <summary>
	/// Print supported instance extensions to stdout
	/// </summary>
	void printSupportedExtensions();

private:
	Window window{ WIDTH, HEIGHT, "Vulkan Tutorial" };
	LogicalDevice vulkan_device{ window };
	std::unique_ptr<SwapChain> device_swap_chain;
	std::unique_ptr<GraphicsPipeline> pipeline;
	VkPipelineLayout pipeline_layout;
	std::vector<VkCommandBuffer> command_buffers;

	/// <summary>
	/// Draws a single frame
	/// </summary>
	void drawFrame();

	void createPipelineLayout();
	void createPipeline();
	void createCommandBuffers();
	void freeCommandBuffers();
	void recordCommandBuffer(int image_index);
	void recreateSwapChain();
};
