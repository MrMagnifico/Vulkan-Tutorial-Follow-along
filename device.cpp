#include "device.hpp"

bool
QueueFamilyIndices::isComplete() {
	return graphics_family.has_value() && present_family.has_value();
}

VkPhysicalDevice
DeviceUtils::findSuitableDevice(VkPhysicalDevice* devices, size_t device_count, VkSurfaceKHR& surface) {
	for (uint32_t counter = 0; counter < device_count; counter++) {
		if (DeviceUtils::isDeviceSuitable(devices[counter], surface)) return devices[counter];
	}
	return VK_NULL_HANDLE;
}

QueueFamilyIndices
DeviceUtils::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR& surface) {
	QueueFamilyIndices indices;

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

	// Find first queue families supporting needed functions
	uint32_t i = 0;
	for (auto& queue_family : queue_families) {
		if (indices.isComplete()) break;

		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphics_family = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport) indices.present_family = i;

		i++;
	}

	return indices;
}

SwapChainSupportDetails
DeviceUtils::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR& surface) {
	SwapChainSupportDetails details;

	// Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	// Surface formats (pixel format, color space)
	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
	if (format_count > 0) {
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
	}

	// Available presentation modes
	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
	if (present_mode_count > 0) {
		details.present_modes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
	}

	return details;
}


//TODO: Expand to HDR colour ranges?
VkSurfaceFormatKHR
DeviceUtils::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_swap_formats) {
	for (const auto& format : available_swap_formats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return format;
	}

	// Fall back on first format if 32bpc SRGB is not found
	available_swap_formats[0];
}

VkPresentModeKHR
DeviceUtils::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) {
	// Prefer 'triple buffering'
	for (auto& mode : available_present_modes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
	}

	// Fall back on 'v-sync'
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
DeviceUtils::chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
	// Vulkan manages the swap chain extent for us (https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSurfaceCapabilitiesKHR.html)
	if (capabilities.currentExtent.width != UINT32_MAX) return capabilities.currentExtent;
	// We need to manage the extent manually
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actual_extent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actual_extent;
	}
}

bool
DeviceUtils::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR& surface) {
	// Graphics and present queue families exist
	QueueFamilyIndices indices = DeviceUtils::findQueueFamilies(device, surface);

	// Required device extensions exist
	bool extensions_supported = DeviceUtils::checkDeviceExtensionSupport(device);

	// At least a single image format and a single presentation mode exist
	bool swap_chain_adequate = false;
	if (extensions_supported) {
		SwapChainSupportDetails swap_chain_support = DeviceUtils::querySwapChainSupport(device, surface);
		swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
	}

	return indices.isComplete() && extensions_supported && swap_chain_adequate;
}

bool
DeviceUtils::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	// Acquire extensions supported by device
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());

	// Ensure needed extensions are supported
	std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
	for (auto& extension : extensions) { required_extensions.erase(extension.extensionName); }
	return required_extensions.empty();
}
