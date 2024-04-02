// #ifndef __IMGUI_SYSTEM_H__
// #define __IMGUI_SYSTEM_H__

// #include "arc_device.hpp"
// #include "arc_pipeline.hpp"

// // external
// #include <imgui/imgui.h>

// // std
// #include <array>
// #include <memory>

// namespace arc
// {

//     // Imgui Class
//     class ImGUI
//     {
//     public:
//         ImGUI(ArcDevice &arcDevice, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
//         ~ImGUI();

//         // Initialize styles, keys etc
//         void init(float width, float height);

//     private:
//         void setStyle(uint32_t index);
//         void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
//         void createPipeline(VkRenderPass renderPass);

//     private:
//         ArcDevice &arcDevice;
//         std::unique_ptr<ArcPipeline> arcPipeline;
//         VkPipelineLayout pipelineLayout;
//         ImGuiStyle vulkanStyle;
//         int selectedStyle = 0;
//     };
// }

// #endif // __IMGUI_SYSTEM_H__