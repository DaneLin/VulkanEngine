#ifndef __ARC_PIPELINE_H__
#define __ARC_PIPELINE_H__

#include "arc_device.hpp"

// std
#include <string>
#include <vector>

namespace arc
{
    struct PipelineConfigInfo
    {
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo &) = delete;
        PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    struct PipelineShaderConfigInfo
    {
        PipelineShaderConfigInfo() = default;
        PipelineShaderConfigInfo(const PipelineShaderConfigInfo &) = delete;
        PipelineShaderConfigInfo &operator=(const PipelineShaderConfigInfo &) = delete;

        VkPipelineShaderStageCreateInfo stageInfo{};
    };

    class ArcPipeline
    {
    public:
        ArcPipeline(ArcDevice &device,
                    const std::string &vertFilePath,
                    const std::string &fragFilePath,
                    const PipelineConfigInfo &configInfo,
                    const PipelineShaderConfigInfo &shaderConfigInfo);
        ~ArcPipeline();

        ArcPipeline(const ArcPipeline &) = delete;
        ArcPipeline operator=(const ArcPipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);
        static void enableAlphaBlending(PipelineConfigInfo &configInfo);
        static void enableMultisampling(PipelineConfigInfo &configInfo);

    private:
        static std::vector<char> readFile(const std::string &filepath);

        void createGraphicsPipeline(const std::string &vertFilePath,
                                    const std::string &fragFilePath,
                                    const PipelineConfigInfo &configInfo,
                                    const PipelineShaderConfigInfo &shaderConfigInfo);

        void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);
        VkPipelineShaderStageCreateInfo loadShader(const std::string &shaderPath, VkShaderStageFlagBits stage);

        ArcDevice &arcDevice;
        VkPipeline graphicsPipeline;
        std::vector<VkShaderModule> shaderModules;
    };
}
#endif // __ARC_PIPELINE_H__