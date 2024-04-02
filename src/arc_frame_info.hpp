#ifndef __ARC_FRAME_INFO_H__
#define __ARC_FRAME_INFO_H__

#include "arc_camera.hpp"
#include "arc_game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace arc
{
#define MAX_LIGHTS 10
    struct PointLight
    {
        // ignore w
        glm::vec4 position{};
        // w is intensity
        glm::vec4 color{};
    };

    struct GlobalUbo
    {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.f};
        glm::mat4 inverseView{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};
        PointLight pointLights[MAX_LIGHTS];
        float outlineWidth = 0.025f;
        int numLights;
    };

    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        ArcCamera &camera;
        VkDescriptorSet globalDescriptorSet;
        ArcGameObject::Map &gameObjects;
    };

}

#endif // __ARC_FRAME_INFO_H__