#pragma once

#include "arc_pipeline.hpp"
#include "arc_device.hpp"
#include "arc_frame_info.hpp"

// std
#include <vector>

namespace arc
{
    class SpecializationConstantSystem
    {
    public:
        SpecializationConstantSystem(ArcDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SpecializationConstantSystem();

        SpecializationConstantSystem(const SpecializationConstantSystem &) = delete;
        SpecializationConstantSystem operator=(const SpecializationConstantSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

    private:
        struct Pipelines
        {
            std::unique_ptr<ArcPipeline> phong;
            std::unique_ptr<ArcPipeline> toon;
            std::unique_ptr<ArcPipeline> textured;

        } pipelines;

    private:
        ArcDevice &arcDevice;
        std::unique_ptr<ArcPipeline> arcPhongPipeline;
        VkPipelineLayout pipelineLayout;
    };
} // namespace arc
