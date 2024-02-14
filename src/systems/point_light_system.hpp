#pragma once

#include "arc_camera.hpp"
#include "arc_pipeline.hpp"
#include "arc_device.hpp"
#include "arc_game_object.hpp"
#include "arc_frame_info.hpp"

// std
#include <vector>

namespace arc
{
    class PointLightSystem
    {
    public:
        PointLightSystem(ArcDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem &) = delete;
        PointLightSystem operator=(const PointLightSystem &) = delete;

        void render(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

    private:
        ArcDevice &arcDevice;
        std::unique_ptr<ArcPipeline> arcPipeline;
        VkPipelineLayout pipelineLayout;
    };
} // namespace arc
