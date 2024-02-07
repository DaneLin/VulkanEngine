#ifndef __ARC_FRAME_INFO_H__
#define __ARC_FRAME_INFO_H__

#include "arc_camera.hpp"

// lib
#include <vulkan/vulkan.h>

namespace arc
{
    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        ArcCamera &camera;
        VkDescriptorSet globalDescriptorSet;
    };

}

#endif // __ARC_FRAME_INFO_H__