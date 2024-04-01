#include "imgui_system.hpp"

namespace arc
{
    ImGUI::ImGUI(ArcDevice &arcDevice, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout)
        : arcDevice{arcDevice}
    {
        ImGui::CreateContext();
        createPipelineLayout(descriptorSetLayout);
        createPipeline(renderPass);
    }

    ImGUI::~ImGUI()
    {
        ImGui::DestroyContext();
        vkDestroyPipelineLayout(arcDevice.device(), pipelineLayout, nullptr);
    }

    void ImGUI::init(float width, float height)
    {
        // Color scheme
        vulkanStyle = ImGui::GetStyle();
        vulkanStyle.Colors[ImGuiCol_TitleBg] = ImVec4(1.f, 0.f, 0.f, 0.6f);
        vulkanStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.f, 0.f, 0.f, 0.8f);
        vulkanStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.f, 0.f, 0.f, 0.4f);
        vulkanStyle.Colors[ImGuiCol_Header] = ImVec4(1.f, 0.f, 0.f, 0.4f);
        vulkanStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.f, 1.f, 0.f, 1.f);
        // default setting
        setStyle(0);

        // Dimensions
        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2(width, height);
        io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
#if defined(_WIN32)
        // If we directly work with os specific key codes
        // we need to map special key types like tab

#endif
    }

    void ImGUI::setStyle(uint32_t index)
    {
        switch (index)
        {
        case 0:
        {
            ImGuiStyle &style = ImGui::GetStyle();
            style = vulkanStyle;
            break;
        }
        case 1:
        {
            ImGui::StyleColorsClassic();
            break;
        }
        case 2:
        {
            ImGui::StyleColorsDark();
            break;
        }
        case 3:
        {
            ImGui::StyleColorsLight();
            break;
        }
        }
    }

    void ImGUI::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
    }

    void ImGUI::createPipeline(VkRenderPass renderPass)
    {
    }
}