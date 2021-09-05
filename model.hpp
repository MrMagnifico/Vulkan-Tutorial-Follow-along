#pragma once

#include "device.hpp"

#include <glm/glm.hpp>

#include <array>
#include <vector>

/// <summary>
/// Defines data elements of a vertex and methods to get Vulkan descriptions of these elements
/// </summary>
struct Vertex {
    /// <summary>
    /// Position of the vertex
    /// </summary>
    glm::vec2 pos;
    /// <summary>
    /// Color of the vertex
    /// </summary>
    glm::vec3 color;

    /// <summary>
    /// Creates a description of how to interpret vertex data stored in memory as will be used by the vertex shader
    /// </summary>
    /// <returns>A Vulkan-usable struct defining layout of vertex data in memory</returns>
    static VkVertexInputBindingDescription getBindingDescription();
    /// <summary>
    /// Defines how to extract specific vertex attributes from the raw data of a single vertex
    /// </summary>
    /// <returns>A 2 element array defining how position and color should be extracted</returns>
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

class Model {
public:
    Model(LogicalDevice& device, const std::vector<Vertex>& vertices);
    ~Model();

    /// <summary>
    /// Binds the vertex buffer of this model to the given command buffer
    /// </summary>
    /// <param name="command_buffer">Command buffer to bind to</param>
    void bind(VkCommandBuffer command_buffer);
    /// <summary>
    /// Adds a draw command for all of the vertices of this model to the given command buffer
    /// </summary>
    /// <param name="command_buffer">Command buffer to add draw command to</param>
    void draw(VkCommandBuffer command_buffer);

private:
    LogicalDevice& logical_device;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;
    uint32_t vertex_count;

    /// <summary>
    /// Creates a buffer on the Vulkan device to store vertex data
    /// </summary>
    /// <param name="vertices">Vertices whose data is to be stored</param>
    void createVertexBuffers(const std::vector<Vertex> vertices);
};
