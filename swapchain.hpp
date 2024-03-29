#pragma once

#include "device.hpp"

#include <memory>
#include <vector>


class SwapChain {
public:
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	SwapChain(LogicalDevice& device, VkExtent2D window_extent);
	SwapChain(LogicalDevice& device, VkExtent2D window_extent, std::shared_ptr<SwapChain> previous);
	~SwapChain();

	VkFramebuffer getFramebuffer(int index) { return swap_chain_framebuffers[index]; }
	VkImageView getImageView(int index) { return swap_chain_image_views[index]; }
	size_t imageCount() { return swap_chain_images.size(); }
	VkFormat getSwapChainImageFormat() { return swap_chain_image_format; }
	VkExtent2D getSwapChainExtent() { return swap_chain_extent; }
	uint32_t getWidth() { return swap_chain_extent.width; }
	uint32_t getHeight() { return swap_chain_extent.height; }
	VkRenderPass getRenderPass() { return render_pass; }

	/// <summary>
	/// Acquire the next available image to be rendered to
	/// </summary>
	/// <param name="image_index">Location to store the index of the image</param>
	/// <returns></returns>
	VkResult acquireNextImage(uint32_t* image_index);
	VkResult submitCommandBuffers(const VkCommandBuffer* command_buffer, uint32_t* image_index);

private:
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;

	std::vector<VkImage> swap_chain_images;
	std::vector<VkImageView> swap_chain_image_views;
	std::vector<VkFramebuffer> swap_chain_framebuffers;

	VkRenderPass render_pass;

	LogicalDevice& device;
	VkExtent2D window_extent;

	VkSwapchainKHR swap_chain;
	std::shared_ptr<SwapChain> old_swap_chain;

	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> images_in_flight; // Keeps track of which images are in flight by their fences so they're not accidentally used before work being done on them has concluded
	size_t current_frame = 0;

	void init();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createFramebuffers();
	void createSynchronisationObjects();

	/// <summary>
	/// Choose a suitable swap format (currently attempts to select 32bpc sRGB) 
	/// </summary>
	/// <param name="available_swap_formats">Swap formats supported by device-surface combination</param>
	/// <returns>A swap format to use for swap chain construction</returns>
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_swap_formats);
	/// <summary>
	/// Choose a suitable swap present mode (currently attempts to select mailbox/triple buffering)
	/// </summary>
	/// <param name="available_present_modes">Present modes supported by device-surface combination</param>
	/// <returns>A present mode to use for swap chain construction</returns>
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
	/// <summary>
	/// Define the dimensions of the images contained in the swap chain
	/// </summary>
	/// <param name="window">Window housing the surface to be rendered to</param>
	/// <param name="capabilities">Surface capabilities of the device-surface combination</param>
	/// <returns>An extent defining the window surface (within bounds of capabilities) for swap chain construction</returns>
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};
