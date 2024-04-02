#ifndef __STENCIL_SYSTEM_H__
#define __STENCIL_SYSTEM_H__

#include "arc_device.hpp"
#include "arc_pipeline.hpp"
#include "arc_frame_info.hpp"

// std
#include <memory>

namespace arc
{
    class StencilSystem
    {
    public:
        StencilSystem(ArcDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~StencilSystem();

        StencilSystem(const StencilSystem &) = delete;
        StencilSystem operator=(const StencilSystem &) = delete;

        void render(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

    private:
        ArcDevice &arcDevice;
        std::unique_ptr<ArcPipeline> stencil;
        std::unique_ptr<ArcPipeline> outline;
        VkPipelineLayout pipelineLayout;
    };
}

#endif // __STENCIL_SYSTEM_H__