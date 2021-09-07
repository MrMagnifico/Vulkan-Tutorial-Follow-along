#include "renderer.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <set>

Renderer::Renderer(Window& window, LogicalDevice& device) : window{ window }, vulkan_device{ device } {
	recreateSwapChain();
	createCommandBuffers();
}

Renderer::~Renderer() {
	freeCommandBuffers();
}

void
Renderer::createCommandBuffers() {
	command_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT); // Number of command buffers is pre-determined and not tied to other resources (e.g: swapchain image count)

	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = vulkan_device.getCommandPool();
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)command_buffers.size();
	if (vkAllocateCommandBuffers(vulkan_device.getDevice(), &alloc_info, command_buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed allocate command buffers");
	}
}

void
Renderer::freeCommandBuffers() {
	vkFreeCommandBuffers(
		vulkan_device.getDevice(),
		vulkan_device.getCommandPool(),
		static_cast<uint32_t>(command_buffers.size()),
		command_buffers.data());
	command_buffers.clear();
}

void
Renderer::recreateSwapChain() {
	// Wait until window is in a drawable state
	VkExtent2D extent = window.getExtent();
	while (extent.width == 0 || extent.height == 0) {
		extent = window.getExtent();
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(vulkan_device.getDevice());

	if (device_swap_chain == nullptr) { device_swap_chain = std::make_unique<SwapChain>(vulkan_device, extent); }
	else {
		// Allows for continuing to execute remaining drawing commands if an old swapchain existed
		std::shared_ptr<SwapChain> old_swap_chain = std::move(device_swap_chain);
		device_swap_chain = std::make_unique<SwapChain>(vulkan_device, extent, old_swap_chain);
		
		// Check if the render pass is incompatible with the new swapchain
		if (!old_swap_chain->compareSwapFormats(*device_swap_chain.get())) {
			throw std::runtime_error("Swap chain image or depth format has changed");
		}
	}
}

VkCommandBuffer
Renderer::beginFrame() {
	assert(!is_frame_started && "Cannot begin frame while another frame is already in progress");

	auto result = device_swap_chain->acquireNextImage(&current_image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) { // Swapchain no longer compatible with surface, must be recreated and current image abandoned
		recreateSwapChain();
		return nullptr;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { // Check that image was acquired and can be presented (even suboptimally)
		throw std::runtime_error("Failed to acquire swapchain image");
	}
	
	is_frame_started = true;

	VkCommandBuffer command_buffer = getCurrentCommandBuffer();
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer");
	}
	return command_buffer;
}

void
Renderer::endFrame() {
	assert(is_frame_started && "Cannot end a frame while rendering a frame is not in progress");

	VkCommandBuffer command_buffer = getCurrentCommandBuffer();
	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer");
	}

	VkResult result = device_swap_chain->submitCommandBuffers(&command_buffer, &current_image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) { // Swapchain no longer compatible or swapchain suboptimal (we recreate here because we've already presented the image, as opposed to the previous check where we are yet to present) or window resize flag was raised
		window.resetWindowResizedFlag();
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swapchain image");
	}

	is_frame_started = false;
	current_frame_index = (current_frame_index + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void
Renderer::beginSwapChainRenderPass(VkCommandBuffer command_buffer) {
	assert(is_frame_started && "Cannot call beginSwapChainRenderPass while rendering a frame is not in progress");
	assert(command_buffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

	VkRenderPassBeginInfo render_pass_begin_info{};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = device_swap_chain->getRenderPass();
	render_pass_begin_info.framebuffer = device_swap_chain->getFramebuffer(current_image_index);
	render_pass_begin_info.renderArea.offset = { 0, 0 };
	render_pass_begin_info.renderArea.extent = device_swap_chain->getSwapChainExtent();
	VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f }; // When clearing previous pixels, set their values to completely black with no transparency
	render_pass_begin_info.clearValueCount = 1;
	render_pass_begin_info.pClearValues = &clear_color;
	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE); // Finalise render pass begin command

	// Describes region of the frame buffer to render to
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(device_swap_chain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(device_swap_chain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	// Defines region where pixels should actually be rendered (basically clipping)
	VkRect2D scissor{ {0, 0}, device_swap_chain->getSwapChainExtent() };
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void
Renderer::endSwapChainRenderPass(VkCommandBuffer command_buffer) {
	assert(is_frame_started && "Cannot call endSwapChainRenderPass while rendering a frame is not in progress");
	assert(command_buffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

	vkCmdEndRenderPass(command_buffer);
}
