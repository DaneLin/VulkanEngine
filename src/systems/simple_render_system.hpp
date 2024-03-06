#pragma once

#include "arc_pipeline.hpp"
#include "arc_device.hpp"
#include "arc_frame_info.hpp"

// std
#include <vector>

namespace arc
{
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(ArcDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

    private:
        ArcDevice &arcDevice;
        std::unique_ptr<ArcPipeline> arcPipeline;
        VkPipelineLayout pipelineLayout;
    };
} // namespace arc
