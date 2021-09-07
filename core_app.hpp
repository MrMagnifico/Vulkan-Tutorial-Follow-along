#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include "scene_object.hpp"
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
	std::vector<SceneObject> scene_objects;

	/// <summary>
	/// Draws a single frame
	/// </summary>
	void drawFrame();
	/// <summary>
	/// Bind pipeline and add push constant and model data to the given buffer
	/// </summary>
	/// <param name="command_buffer">Command buffer to add scene object render data to</param>
	void renderSceneObjects(VkCommandBuffer command_buffer);

	void loadSceneObjects();
	void createPipelineLayout();
	void createPipeline();
	void createCommandBuffers();
	void freeCommandBuffers();
	void recordCommandBuffer(int image_index);
	void recreateSwapChain();
};
