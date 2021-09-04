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
