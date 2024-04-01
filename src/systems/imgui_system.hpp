#ifndef __IMGUI_SYSTEM_H__
#define __IMGUI_SYSTEM_H__

#include "arc_device.hpp"
#include "arc_pipeline.hpp"

// external
#include <imgui/imgui.h>

// std
#include <array>

namespace arc
{
    // options and values to display/toggle from the ui
    struct UISettings
    {
        bool displayModels = true;
        bool displayLogos = true;
        bool displayBackground = true;
        bool animateLight = false;
        float lightSpeed = 0.25f;
        std::array<float, 50> frameTimes{};
        float frameTimeMin = 9999.0f, frameTimeMax = 0.0f;
        float lightTimer = 0.f;
    } uiSettings;

    // Imgui Class
    class ImGUI
    {
    public:
        ImGUI(ArcDevice &arcDevice, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
        ~ImGUI();

        // Initialize styles, keys etc
        void init(float width, float height);

    private:
        void setStyle(uint32_t index);
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

    private:
        ArcDevice &arcDevice;
        std::unique_ptr<ArcPipeline> arcPipeline;
        VkPipelineLayout pipelineLayout;
        ImGuiStyle vulkanStyle;
        int selectedStyle = 0;
    };
}

#endif // __IMGUI_SYSTEM_H__