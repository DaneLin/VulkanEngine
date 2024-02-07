#ifndef __ARC_MODEL_H__
#define __ARC_MODEL_H__

#include "arc_device.hpp"
#include "arc_buffer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace arc
{
    class ArcModel
    {
    public:
        struct Vertex
        {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const
            {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };

        struct Builder
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string &filepath);
        };

        ArcModel(ArcDevice &device, const ArcModel::Builder &builder);
        ~ArcModel();

        ArcModel(const ArcModel &) = delete;
        ArcModel operator=(const ArcModel &) = delete;

        static std::unique_ptr<ArcModel> createModelFromFile(ArcDevice &device, const std::string &filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

    private:
        ArcDevice &arcDevice;

        std::unique_ptr<ArcBuffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;

        std::unique_ptr<ArcBuffer> indexBuffer;
        uint32_t indexCount;
    };
}
#endif // __ARC_MODEL_H__