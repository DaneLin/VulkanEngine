#pragma once

#include "arc_device.hpp"
#include "arc_pipeline.hpp"
#include "arc_frame_info.hpp"

namespace arc
{
    class MultisamplingSystem
    {
    public:
        MultisamplingSystem(ArcDevice &arcDevice, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
        ~MultisamplingSystem();

        MultisamplingSystem(const MultisamplingSystem &) = delete;
        MultisamplingSystem operator=(const MultisamplingSystem &) = delete;

        void RenderGameObjects(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

    private:
        ArcDevice &arcDevice;
        std::unique_ptr<ArcPipeline> arcPipeline;
        VkPipelineLayout pipelineLayout;
    };
}