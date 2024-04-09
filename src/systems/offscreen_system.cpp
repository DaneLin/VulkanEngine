#include "offscreen_system.hpp"
#include "arc_buffer.hpp"

// std
#include <vector>
namespace arc
{
    struct
    {
        ArcBuffer matrices;
        ArcBuffer params;
    };
    OffscreenSystem::OffscreenSystem(ArcDevice &device, VkRenderPass &renderPass, VkDescriptorSetLayout &globalDesc)
        : arcDevice(device)
    {
        createPipelineLayout(globalDesc);
        createPipeline(renderPass);
    }

    void OffscreenSystem::createPipelineLayout(VkDescriptorSetLayout &globalDesc)
    {
    }

    void OffscreenSystem::createPipeline(VkRenderPass &renderPass)
    {
    }

    OffscreenSystem::~OffscreenSystem()
    {
        vkDestroyPipelineLayout(arcDevice.device(), attachmentRead, nullptr);
        vkDestroyPipelineLayout(arcDevice.device(), attachmentWrite, nullptr);
    }

}