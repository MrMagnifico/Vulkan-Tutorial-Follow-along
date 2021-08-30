#include "swapchain.hpp"

#include <algorithm>
#include <stdexcept>

SwapChain::SwapChain(LogicalDevice& device, VkExtent2D window_extent) : device{ device }, window_extent{ window_extent } {
	createSwapChain();
	createImageViews();
	createRenderPass();
	createFramebuffers();
	createSynchronisationObjects();
}

SwapChain::~SwapChain() {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device.getDevice(), image_available_semaphores[i], nullptr);
		vkDestroySemaphore(device.getDevice(), render_finished_semaphores[i], nullptr);
		vkDestroyFence(device.getDevice(), in_flight_fences[i], nullptr);
	}

	for (auto framebuffer : swap_chain_framebuffers) { vkDestroyFramebuffer(device.getDevice(), framebuffer, nullptr); }

	vkDestroyRenderPass(device.getDevice(), render_pass, nullptr);
	
	for (auto& image_view : swap_chain_image_views) { vkDestroyImageView(device.getDevice(), image_view, nullptr); }
	swap_chain_image_views.clear();

	if (swap_chain != nullptr) {
		vkDestroySwapchainKHR(device.getDevice(), swap_chain, nullptr);
		swap_chain = nullptr;
	}
}

VkResult
SwapChain::acquireNextImage(uint32_t* image_index) {
	vkWaitForFences(device.getDevice(), 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

	auto result = vkAcquireNextImageKHR(
		device.getDevice(),
		swap_chain,
		UINT64_MAX, // Disables image availability timeout
		image_available_semaphores[current_frame], // Signal semaphore for current frame once the image has been retrieved and is available
		VK_NULL_HANDLE, // No fences are used
		image_index);

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (images_in_flight[*image_index] != VK_NULL_HANDLE) { vkWaitForFences(device.getDevice(), 1, &images_in_flight[*image_index], VK_TRUE, UINT64_MAX); }
	images_in_flight[*image_index] = in_flight_fences[current_frame]; // Mark the image as now being in use by this frame

	return result;
}

VkResult
SwapChain::submitCommandBuffers(const VkCommandBuffer* command_buffer, uint32_t* image_index) {
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	
	// Wait on color writing step until an image has been retrieved from the swapchain
	VkSemaphore wait_semaphores[] = { image_available_semaphores[current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;

	// Submit the command buffer linked to 
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = command_buffer;

	// Signal the rendering finished semaphore when the command buffer has finished executing
	VkSemaphore signal_semaphores[] = { render_finished_semaphores[current_frame] };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	vkResetFences(device.getDevice(), 1, &in_flight_fences[current_frame]);

	if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submit_info, in_flight_fences[current_frame]) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer");
	}

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores; // Wait until rendering has finished before presenting
	VkSwapchainKHR swap_chains[] = { swap_chain }; // Specify which swapchain to present images to
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = image_index; // Specify which of the swapchain images to present to
	present_info.pResults = nullptr;

	current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	return vkQueuePresentKHR(device.getPresentQueue(), &present_info);
}

void
SwapChain::createSwapChain() {
	SwapChainSupportDetails swap_chain_support = device.getSwapChainSupport();

	// Acquire swap chain initialisation details
	VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(swap_chain_support.formats);
	VkPresentModeKHR present_mode = chooseSwapPresentMode(swap_chain_support.present_modes);
	VkExtent2D extent = chooseSwapExtent(swap_chain_support.capabilities);

	// Choose appropriate minimum number of images to keep in swap chain at any time
	uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
	if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) {
		image_count = swap_chain_support.capabilities.maxImageCount;
	}

	// Populate swapchain creation struct
	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = device.getSurface();
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Images will be used for direct colour attachment (i.e: presented as is with no further processing)
	create_info.preTransform = swap_chain_support.capabilities.currentTransform; // No transformation desired
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Ignore alpha channel, no blending needed
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE; // Ignore obscured pixels (it's 2D occlussion culling bro)
	create_info.oldSwapchain = VK_NULL_HANDLE; // Only single swapchain for now, no fallback

	// Populate info about image sharing amongst queue families
	QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
	uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

	// Graphics and present family are different, sharing needed
	if (indices.graphics_family != indices.present_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	}
	// Graphics and present family are the same, set exclusive ownership as no sharing will be bothered with
	else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;
	}

	swap_chain_image_format = surface_format.format; // Store handle to image format
	swap_chain_extent = extent; // Store handle to surface extent
	if (vkCreateSwapchainKHR(device.getDevice(), &create_info, nullptr, &swap_chain) != VK_SUCCESS) { // Create handle for actual swap chain
		throw std::runtime_error("Failed to create swapchain");
	}

	// Create handle for images in swap chain
	vkGetSwapchainImagesKHR(device.getDevice(), swap_chain, &image_count, nullptr);
	swap_chain_images.resize(image_count);
	vkGetSwapchainImagesKHR(device.getDevice(), swap_chain, &image_count, swap_chain_images.data());
}

void
SwapChain::createRenderPass() {
	VkAttachmentDescription colour_attachment{};
	colour_attachment.format = swap_chain_image_format;
	colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT; // Should match multi-sampling sample count
	colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear colour and depth data before drawing new frame
	colour_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Save rendered data in memory (so we can actually display the goddamn thing)
	colour_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Not doing anything with stencil buffer data, so we don't care
	colour_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // We don't care about previous image data
	colour_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Newly rendered data should be stored in a format suitable for presentation to the swapchain

	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Specify that this is a graphics subpass (because compute shaders are a thing, apparently)
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // The built-in pre-pass dependency should wait
	dependency.dstSubpass = 0; // And it waits on the subpass with index 0 (the only one in our case)
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Wait on the colour attachment output specifically, which occurs only after an image has been retrieved (see submitCommandBuffers)
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // The operation that should wait is in the color attachment stage
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // The exact operation that should wait is the final writing of the colors

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colour_attachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device.getDevice(), &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass");
	}
}

void
SwapChain::createImageViews() {
	swap_chain_image_views.resize(swap_chain_images.size());

	for (size_t i = 0; i < swap_chain_images.size(); i++) {
		VkImageViewCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = swap_chain_images[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // Swap chain images are normal 2D images
		create_info.format = swap_chain_image_format; // Format is same as that of the images themselves

		// Colour components are kept as is (i.e: multiplied by an identity matrix)
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Swap chain images are single flat 2D images with a single mipmap level and a single layer
		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Levels should be interpreted as straight colours
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device.getDevice(), &create_info, nullptr, &swap_chain_image_views[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view(s)");
		}
	}
}

void
SwapChain::createFramebuffers() {
	swap_chain_framebuffers.resize(swap_chain_image_views.size());

	for (size_t i = 0; i < swap_chain_image_views.size(); i++) {
		VkImageView attachments[] = { swap_chain_image_views[i] };

		VkFramebufferCreateInfo framebuffer_create_info{};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.renderPass = render_pass;
		framebuffer_create_info.attachmentCount = 1;
		framebuffer_create_info.pAttachments = attachments;
		framebuffer_create_info.width = swap_chain_extent.width;
		framebuffer_create_info.height = swap_chain_extent.height;
		framebuffer_create_info.layers = 1;

		if (vkCreateFramebuffer(device.getDevice(), &framebuffer_create_info, nullptr, &swap_chain_framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer");
		}
	}
}

void
SwapChain::createSynchronisationObjects() {
	image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
	images_in_flight.resize(swap_chain_images.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device.getDevice(), &semaphore_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device.getDevice(), &semaphore_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device.getDevice(), &fence_info, nullptr, &in_flight_fences[i])) {
			throw std::runtime_error("Failed to create synchronisation object(s) for a frame");
		}
	}
}

// TODO: Expand to HDR colour ranges?
VkSurfaceFormatKHR
SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_swap_formats) {
	for (const auto& format : available_swap_formats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return format;
	}

	// Fall back on first format if 32bpc SRGB is not found
	return available_swap_formats[0];
}

VkPresentModeKHR
SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) {
	// Prefer 'triple buffering'
	for (auto& mode : available_present_modes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
	}

	// Fall back on 'v-sync'
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	// Vulkan manages the swap chain extent for us (https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSurfaceCapabilitiesKHR.html)
	if (capabilities.currentExtent.width != UINT32_MAX) return capabilities.currentExtent;
	// We need to manage the extent manually
	else {
		VkExtent2D actual_extent = {
			static_cast<uint32_t>(window_extent.width),
			static_cast<uint32_t>(window_extent.height)
		};
		actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actual_extent;
	}
}
