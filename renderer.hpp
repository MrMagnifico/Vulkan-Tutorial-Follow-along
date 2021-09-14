#pragma once

#include "device.hpp"
#include "swapchain.hpp"
#include "window.hpp"

#include <cassert>
#include <memory>
#include <vector>

/// <summary>
/// Manages the creation of resources needed for rendering and submission of final result to swap chain.
/// Rendering frames managed by the renderer are independent of other components (e.g: swapchain image count)
/// </summary>
class Renderer {
public:
	Renderer(Window& window, LogicalDevice& device);
	~Renderer();

	/// <summary>
	/// Acquire index of swap chain image to render frame output to and allocate a corresponding command buffer.
	/// This signals the start of rendering a new frame
	/// </summary>
	/// <returns>A command buffer ready to accept commands and data for rendering</returns>
	VkCommandBuffer beginFrame();
	/// <summary>
	/// Submit finalised command buffer and potentially trigger swapchain recreation if the submitted image is suboptimal
	/// This signals the end of rendering the newly made frame
	/// </summary>
	void endFrame();
	/// <summary>
	/// Add swap chain render pass start information to the command buffer
	/// </summary>
	/// <param name="command_buffer">Command buffer to add render pass information to</param>
	void beginSwapChainRenderPass(VkCommandBuffer command_buffer);
	/// <summary>
	/// Add swap chain render pass end information to the command buffer
	/// </summary>
	/// <param name="command_buffer">Command buffer to add render pass information to</param>
	void endSwapChainRenderPass(VkCommandBuffer command_buffer);

	VkRenderPass getSwapChainRenderPass() const { return device_swap_chain->getRenderPass(); }
	float getAspectRatio() const { return device_swap_chain->extentAspectRatio(); }
	bool isFrameInProgress() const { return is_frame_started; }
	VkCommandBuffer getCurrentCommandBuffer() const {
		assert(is_frame_started && "Cannot get current command buffer if a frame is not in progress");
		return command_buffers[current_frame_index];
	}
	int getFrameIndex() const { 
		assert(is_frame_started && "Cannot get current frame index if a frame is not in progress");
		return current_frame_index;
	}

private:
	uint32_t current_image_index;
	int current_frame_index;
	bool is_frame_started = false;

	Window& window;
	LogicalDevice& vulkan_device;
	std::unique_ptr<SwapChain> device_swap_chain;
	std::vector<VkCommandBuffer> command_buffers;

	void createCommandBuffers();
	void freeCommandBuffers();
	void recreateSwapChain();
};
