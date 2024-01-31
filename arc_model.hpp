#ifndef __ARC_MODEL_H__
#define __ARC_MODEL_H__

#include "arc_device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

namespace arc
{
    class ArcModel
    {
    public:
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        ArcModel(ArcDevice &device, const std::vector<Vertex> &vertices);
        ~ArcModel();

        ArcModel(const ArcModel &) = delete;
        ArcModel operator=(const ArcModel &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);

    private:
        ArcDevice &arcDevice;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };
}
#endif // __ARC_MODEL_H__