#include "multisampling_system.hpp"

// std
#include <vector>

namespace arc
{
    MultisamplingSystem::MultisamplingSystem(ArcDevice &arcDevice, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout)
        : arcDevice(arcDevice)
    {
        createPipelineLayout(descriptorSetLayout);
        createPipeline(renderPass);
    }

    void MultisamplingSystem::RenderGameObjects(FrameInfo &frameInfo)
    {
        arcPipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &frameInfo.globalDescriptorSet,
            0, nullptr);

        for (auto &kv : frameInfo.gameObjects)
        {
            auto &obj = kv.second;
            if (obj.model == nullptr)
                continue;
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

    void MultisamplingSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        std::vector<VkDescriptorSetLayout> descriptorSetLayout{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    }

    void MultisamplingSystem::createPipeline(VkRenderPass renderPass)
    {
    }

    MultisamplingSystem::~MultisamplingSystem()
    {
        vkDestroyPipelineLayout(arcDevice.device(), pipelineLayout, nullptr);
    }

}