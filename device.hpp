#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <optional>
#include <set>
#include <vector>
#include <cstdint>
#include <algorithm>

const std::vector<const char*> device_extensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

/// <summary>
/// Holds the indices of the queue families required for rendering
/// </summary>
struct QueueFamilyIndices {
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;

	/// <summary>
	/// Determines whether all needed families are present
	/// </summary>
	/// <returns>Indication if an index exists for each member family</returns>
	bool isComplete();
};

/// <summary>
/// Holds data about the swap chain support of a particular phsysical device
/// </summary>
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

/// <summary>
/// Utility class containing static methods for managing and extracting data about physical and logical devices
/// </summary>
class DeviceUtils {
public:
	/// <summary>
	/// Fetches a suitable device according to certain requirements.
	/// See <c>isDeviceSuitable</c>
	/// </summary>
	/// <param name="devices">Array of available devices to select from</param>
	/// <param name="device_count">Number of devices in the given array</param>
	/// <param name="surface">Surface to be rendered to</param>
	/// <returns>Handle for a device that satisfies the established requirements</returns>
	static VkPhysicalDevice findSuitableDevice(VkPhysicalDevice* devices, size_t device_count, VkSurfaceKHR& surface);
	/// <summary>
	/// Fetches indices of queue families according to definition of QueueFamilyIndices
	/// </summary>
	/// <param name="device">Device to fetch queues from</param>
	/// <param name="surface">Surface to be rendered to</param>
	/// <returns>Struct containing (potentially present) indices of queues</returns>
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR& surface);
	/// <summary>
	/// Queries for exact capabilities of swap chain support
	/// </summary>
	/// <param name="device">Device to check capabilities of</param>
	/// <param name="surface">Surface to be rendered to</param>
	/// <returns>Struct containing details of swap chain support as indicated by the definition of SwapChainSupportDetails</returns>
	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR& surface);
	/// <summary>
	/// Choose a suitable swap format (currently attempts to select 32bpc sRGB) 
	/// </summary>
	/// <param name="available_swap_formats">Swap formats supported by device-surface combination</param>
	/// <returns>A swap format to use for swap chain construction</returns>
	static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_swap_formats);
	/// <summary>
	/// Choose a suitable swap present mode (currently attempts to select mailbox/triple buffering)
	/// </summary>
	/// <param name="available_present_modes">Present modes supported by device-surface combination</param>
	/// <returns>A present mode to use for swap chain construction</returns>
	static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
	/// <summary>
	/// Define the dimensions of the images contained in the swap chain
	/// </summary>
	/// <param name="window">Window housing the surface to be rendered to</param>
	/// <param name="capabilities">Surface capabilities of the device-surface combination</param>
	/// <returns>An extent defining the window surface (within bounds of capabilities) for swap chain construction</returns>
	static VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
private:
	/// <summary>
	/// Checks that the given device-surface combo meets the following criteria:
	/// (
	///		Graphics and present queue families exist -
	///		The device extensions contained in <c>device_extensions</c> exist -
	///		At least a single image format and presentation format exist
	/// )
	/// </summary>
	/// <param name="device">Device to evaluate suitability of</param>
	/// <param name="surface">Surface to be rendered to use for fetching properties</param>
	/// <returns>Indication if device is suitable for rendering to the surface</returns>
	static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR& surface);
	/// <summary>
	/// Check that the given device supported the extensions contained in <c>device_extensions</c>
	/// </summary>
	/// <param name="device">Device to check extension support for</param>
	/// <returns>Indication if device supports these extensions</returns>
	static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
};
