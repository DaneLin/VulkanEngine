#ifndef __OFFSCREEN_SYTEM_H__
#define __OFFSCREEN_SYTEM_H__

#include "arc_device.hpp"
#include "arc_pipeline.hpp"
#include "arc_image.hpp"
// std
#include <memory>
#include <vector>

namespace arc
{
    struct FrameBufferAttachment
    {
        ArcImage image;
        VkFormat format;
    };

    struct Attachments
    {
        FrameBufferAttachment color, depth;
    };

    class OffscreenSystem
    {
    public:
        OffscreenSystem(ArcDevice &device, VkRenderPass &renderPass, VkDescriptorSetLayout &globalDesc);
        ~OffscreenSystem();

        OffscreenSystem(const OffscreenSystem &) = delete;
        OffscreenSystem operator=(const OffscreenSystem &) = delete;

    private:
        void createPipelineLayout(VkDescriptorSetLayout &globalDesc);
        void createPipeline(VkRenderPass &renderPass);

    private:
        ArcDevice &arcDevice;
        std::unique_ptr<ArcPipeline> pipelinetWrite;
        std::unique_ptr<ArcPipeline> pipelineRead;
        VkPipelineLayout attachmentWrite;
        VkPipelineLayout attachmentRead;
    };

    std::vector<Attachments> attachments;
}

#endif // __OFFSCREEN_SYTEM_H__