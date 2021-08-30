#pragma once

#include "window.hpp"

#include <optional>
#include <vector>

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
	bool isComplete() { return graphics_family.has_value() && present_family.has_value(); }
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
/// Class for managing device resources and functionality
/// </summary>
class LogicalDevice {
public:
	VkPhysicalDeviceProperties physical_device_properties;

	LogicalDevice(Window& window);
	~LogicalDevice();

	VkDevice getDevice() { return device_; }
	VkSurfaceKHR getSurface() { return surface_; }
	VkQueue getGraphicsQueue() { return graphics_queue_; }
	VkQueue getPresentQueue() { return present_queue_; }
	VkCommandPool getCommandPool() { return command_pool; }

	SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physical_device); }
	QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physical_device); }
private:
#ifdef NDEBUG
	const bool enable_validation_layers = false;
#else
	const bool enable_validation_layers = true;
#endif
	const std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };

	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	Window& window;

	VkDevice device_;
	VkSurfaceKHR surface_;
	VkQueue graphics_queue_;
	VkQueue present_queue_;

	VkCommandPool command_pool;

	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createCommandPool();

	/// <summary>
	/// Verify that all required validation layers (as specified in <c>validation_layers</c> are present
	/// </summary>
	/// <returns>A boolean indicating whether all the needed layers are present or not</returns>
	bool checkValidationLayerSupport();
	/// <summary>
	/// Acquires a list of needed extensions based on <c>device_extensions</c>,
	/// the extensions needed by GLFW and (if enabled) the extensions needed for debug validation layers
	/// </summary>
	/// <returns>String vector containing names of needed extensions</returns>
	std::vector<const char*> getRequiredExtensions();
	/// <summary>
	/// Fetches a suitable device according to certain requirements.
	/// See <c>isDeviceSuitable</c>
	/// </summary>
	/// <param name="devices">Array of available devices to select from</param>
	/// <param name="device_count">Number of devices in the given array</param>
	/// <param name="surface">Surface to be rendered to</param>
	/// <returns>Handle for a device that satisfies the established requirements</returns>
	VkPhysicalDevice findSuitableDevice(VkPhysicalDevice* devices, size_t device_count);
	/// <summary>
	/// Fetches indices of queue families according to definition of QueueFamilyIndices
	/// </summary>
	/// <param name="device">Device to fetch queues from</param>
	/// <param name="surface">Surface to be rendered to</param>
	/// <returns>Struct containing (potentially present) indices of queues</returns>
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	/// <summary>
	/// Queries for exact capabilities of swap chain support
	/// </summary>
	/// <param name="device">Device to check capabilities of</param>
	/// <param name="surface">Surface to be rendered to</param>
	/// <returns>Struct containing details of swap chain support as indicated by the definition of SwapChainSupportDetails</returns>
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
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
	bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR& surface);
	/// <summary>
	/// Check that the given device supported the extensions contained in <c>device_extensions</c>
	/// </summary>
	/// <param name="device">Device to check extension support for</param>
	/// <returns>Indication if device supports these extensions</returns>
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
};
