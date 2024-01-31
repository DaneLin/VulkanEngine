#pragma once

#include "arc_camera.hpp"
#include "arc_pipeline.hpp"
#include "arc_device.hpp"
#include "arc_game_object.hpp"

// std
#include <vector>

namespace arc
{
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(ArcDevice &device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<ArcGameObject> &gameObjects, const ArcCamera &camera);

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

    private:
        ArcDevice &arcDevice;
        std::unique_ptr<ArcPipeline> arcPipeline;
        VkPipelineLayout pipelineLayout;
    };
} // namespace arc
