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
    glm::vec3 pos{};
    /// <summary>
    /// Color of the vertex
    /// </summary>
    glm::vec3 color{};

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
    Model(LogicalDevice& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices);
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

    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;
    uint32_t index_count;
    bool has_index_buffer = false;

    /// <summary>
    /// Creates a buffer on the Vulkan device to store vertex data
    /// </summary>
    /// <param name="vertices">Vertices whose data is to be stored</param>
    void createVertexBuffers(const std::vector<Vertex> vertices);
    /// <summary>
    /// Creates a buffer on the Vulkan device to store triangle index data
    /// </summary>
    /// <param name="indices">Indices in vertex buffer of vertices used by each triangle (must be in groups of 3)</param>
    void createIndexBuffers(const std::vector<uint32_t> indices);
};
