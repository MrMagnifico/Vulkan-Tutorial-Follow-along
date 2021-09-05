#include "model.hpp"

VkVertexInputBindingDescription
Vertex::getBindingDescription() {
	VkVertexInputBindingDescription binding_desc{};
	binding_desc.binding = 0;
	binding_desc.stride = sizeof(Vertex);
	binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_desc;
}

std::array<VkVertexInputAttributeDescription, 2>
Vertex::getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};

	// Position attribute
	attribute_descriptions[0].binding = 0;
	attribute_descriptions[0].location = 0; // Position data is in location 0 (check vertex shader)
	attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // 2 32-bit signed float components for position
	attribute_descriptions[0].offset = offsetof(Vertex, pos); // Offset of position into vertex data block

	// Color attribute
	attribute_descriptions[1].binding = 0;
	attribute_descriptions[1].location = 1; // Color data is in location 1 (check vertex shader)
	attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // 3 32-bit signed float components for position
	attribute_descriptions[1].offset = offsetof(Vertex, color); // Offset of color into vertex data block

	return attribute_descriptions;
}

Model::Model(LogicalDevice& device, const std::vector<Vertex>& vertices) : logical_device{ device } {
	createVertexBuffers(vertices);
}

Model::~Model() {
	vkDestroyBuffer(logical_device.getDevice(), vertex_buffer, nullptr);
	vkFreeMemory(logical_device.getDevice(), vertex_buffer_memory, nullptr);
}

void
Model::bind(VkCommandBuffer command_buffer) {
	VkBuffer buffers[] = { vertex_buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
}

void
Model::draw(VkCommandBuffer command_buffer) {
	vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
}

void
Model::createVertexBuffers(const std::vector<Vertex> vertices) {
	// Determine how big the buffer needs to be
	vertex_count = static_cast<uint32_t>(vertices.size());
	assert(vertex_count >= 3 && "Number of vertices in a model must be at least 3");
	VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

	// Create a staging buffer to move resources to from host memory
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	logical_device.createBuffer(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // Buffer will be used for copying to vertex buffer from
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // Underlying memory should be visible to the host device (i.e: CPU)
		staging_buffer,
		staging_buffer_memory
	);

	// Copy vertex data to staging buffer
	void* data;
	vkMapMemory(logical_device.getDevice(), staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
	vkUnmapMemory(logical_device.getDevice(), staging_buffer_memory);

	// Create vertex buffer with underlying memory type being the most optimal for the graphics device (which is why we use a staging buffer first) and copy vertex data to it
	logical_device.createBuffer(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, // Buffer will be used as a vertex buffer and will be transferred to from the staging buffer
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // Underlying memory is the most efficient for access by the Vulkan device
		vertex_buffer,
		vertex_buffer_memory);
	logical_device.copyBuffer(staging_buffer, vertex_buffer, buffer_size);

	// Clean up staging buffer resources
	vkDestroyBuffer(logical_device.getDevice(), staging_buffer, nullptr);
	vkFreeMemory(logical_device.getDevice(), staging_buffer_memory, nullptr);
}
