#include "arc_model.hpp"
#include "arc_utils.hpp"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <iostream>
#include <cstring>
#include <unordered_map>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace std
{
    template <>
    struct hash<arc::ArcModel::Vertex>
    {
        size_t operator()(arc::ArcModel::Vertex const &vertex) const noexcept
        {
            size_t seed = 0;
            arc::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace arc
{
    std::vector<VkVertexInputBindingDescription> ArcModel::Vertex::getBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> ArcModel::Vertex::getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Vertex::position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Vertex::color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Vertex::normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, Vertex::uv)});

        return attributeDescriptions;
    }

    ArcModel::ArcModel(ArcDevice &device, const ArcModel::Builder &builder)
        : arcDevice{device}
    {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    ArcModel::~ArcModel()
    {
    }

    void ArcModel::createVertexBuffers(const std::vector<Vertex> &vertices)
    {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");

        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        ArcBuffer stagingBuffer{
            arcDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)vertices.data());

        vertexBuffer = std::make_unique<ArcBuffer>(
            arcDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        arcDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }

    void ArcModel::createIndexBuffers(const std::vector<uint32_t> &indices)
    {
        // std::cout << "indexCount : " << indices.size() << '\n';
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer)
            return;

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        ArcBuffer stagingBuffer{
            arcDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)indices.data());

        indexBuffer = std::make_unique<ArcBuffer>(
            arcDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        arcDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

    std::unique_ptr<ArcModel> ArcModel::createModelFromFile(ArcDevice &device, const std::string &filepath)
    {
        Builder builder{};
        builder.loadModel(filepath);
        std::cout << "Vertex count: " << builder.vertices.size() << '\n';
        std::cout << "Index count: " << builder.indices.size() << '\n';
        return std::make_unique<ArcModel>(device, builder);
    }

    void ArcModel::draw(VkCommandBuffer commandBuffer)
    {
        if (hasIndexBuffer)
        {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        }
        else
        {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }

    void ArcModel::bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer)
        {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void ArcModel::Builder::loadModel(const std::string &filepath)
    {
        // tinyobj::attrib_t attrib;
        // std::vector<tinyobj::shape_t> shapes;
        // std::vector<tinyobj::material_t> materials;
        // std::string warn, err;

        // std::string enginePath = ENGINE_DIR + filepath;
        // if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, enginePath.c_str()))
        // {
        //     throw std::runtime_error(warn + err);
        // }

        std::string enginePath = ENGINE_DIR + filepath;
        tinyobj::ObjReaderConfig readerConfig;
        tinyobj::ObjReader reader;
        readerConfig.mtl_search_path = "";

        if (!reader.ParseFromFile(enginePath, readerConfig))
        {
            if (!reader.Error().empty())
            {
                throw std::runtime_error(reader.Error());
            }
        }

        if (!reader.Warning().empty())
        {
            std::cout << "TinyObjReader: " << reader.Warning() << '\n';
        }

        auto &attrib = reader.GetAttrib();
        auto &shapes = reader.GetShapes();

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        // loop over shapes
        for (size_t s = 0; s < shapes.size(); ++s)
        {
            size_t indexOffset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f)
            {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                for (size_t v = 0; v < fv; v++)
                {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[indexOffset + v];
                    Vertex vertex{};
                    if (idx.vertex_index >= 0)
                    {
                        vertex.position.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                        vertex.position.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                        vertex.position.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                        vertex.color.x = attrib.colors[3 * size_t(idx.vertex_index) + 0];
                        vertex.color.y = attrib.colors[3 * size_t(idx.vertex_index) + 1];
                        vertex.color.z = attrib.colors[3 * size_t(idx.vertex_index) + 2];
                    }

                    // Check if normal index
                    if (idx.normal_index >= 0)
                    {
                        vertex.normal.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        vertex.normal.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        vertex.normal.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    }

                    if (idx.texcoord_index >= 0)
                    {
                        vertex.uv.x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                        vertex.uv.y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    }

                    if (uniqueVertices.count(vertex) == 0)
                    {
                        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                        vertices.push_back(vertex);
                    }
                    indices.push_back(uniqueVertices[vertex]);
                }
                indexOffset += fv;
            }
        }
    }
}