#include "specializationConstants.hpp"

#include <stdexcept>

namespace arc
{

    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };

    SpecializationConstantSystem::SpecializationConstantSystem(ArcDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : arcDevice(device)
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    void SpecializationConstantSystem::renderGameObjects(FrameInfo &frameInfo)
    {
        pipelines.toon->bind(frameInfo.commandBuffer);
        // pipelines.phong->bind(frameInfo.commandBuffer);
        // pipelines.textured->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &frameInfo.globalDescriptorSet,
            0, nullptr);

        for (auto &kv : frameInfo.gameObjects)
        {
            auto &obj = kv.second;
            if (obj.model == nullptr)
                continue;

            SimplePushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(frameInfo.commandBuffer,
                               pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(SimplePushConstantData),
                               &push);

            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

    void SpecializationConstantSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        // std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        // VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        // pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        // pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(arcDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void SpecializationConstantSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(pipelineLayout != nullptr && "Cannot create pipeline without creating pipeline layout first!");
        PipelineConfigInfo pipelineConfig{};
        ArcPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = arcDevice.getMaxUsableSampleCount();

        // Host data to take specialization constants from
        struct SpecializationData
        {
            // Sets the lighting model used in the fragment 'uber' shader
            uint32_t lightingModel{0};
            // Parameter for the toon shading part of the fragment shader
            float toonDesaturationFactor{0.5f};
        } specializationData;

        // Each shader constant of a shader stage corresponds to one map entry
        std::array<VkSpecializationMapEntry, 2> specializationMapEntries;
        // Shader bindings based on specialization constants are marked by the new "constant_id" layout qualifier:
        //      layout (constant_id = 0) const int LIGHTING_MODEL = 0;
        //      layout (constant_id = 1) const float PARAM_TOON_DESATURATION = 0.0f;

        // Map entry for the lighting model to be used by the fragment shader
        specializationMapEntries[0].constantID = 0;
        specializationMapEntries[0].size = sizeof(specializationData.lightingModel);
        specializationMapEntries[0].offset = 0;

        // Map entry for the toon shader parameter
        specializationMapEntries[1].constantID = 1;
        specializationMapEntries[1].size = sizeof(specializationData.lightingModel);
        specializationMapEntries[1].offset = offsetof(SpecializationData, SpecializationData::toonDesaturationFactor);

        // Prepare specialization info block for the shader stage
        VkSpecializationInfo specializationInfo{};
        specializationInfo.dataSize = sizeof(specializationData);
        specializationInfo.mapEntryCount = static_cast<uint32_t>(specializationMapEntries.size());
        specializationInfo.pMapEntries = specializationMapEntries.data();
        specializationInfo.pData = &specializationData;

        PipelineShaderConfigInfo shaderConfig{};
        shaderConfig.stageInfo.pSpecializationInfo = &specializationInfo;

        specializationData.lightingModel = 0;
        pipelines.phong = std::make_unique<ArcPipeline>(arcDevice,
                                                        "shaders/specialization.vert.spv",
                                                        "shaders/specialization.frag.spv",
                                                        pipelineConfig,
                                                        shaderConfig);

        specializationData.lightingModel = 1;
        pipelines.toon = std::make_unique<ArcPipeline>(arcDevice,
                                                       "shaders/specialization.vert.spv",
                                                       "shaders/specialization.frag.spv",
                                                       pipelineConfig,
                                                       shaderConfig);
        specializationData.lightingModel = 2;
        pipelines.textured = std::make_unique<ArcPipeline>(arcDevice,
                                                           "shaders/specialization.vert.spv",
                                                           "shaders/specialization.frag.spv",
                                                           pipelineConfig,
                                                           shaderConfig);
    }

    SpecializationConstantSystem::~SpecializationConstantSystem()
    {
        vkDestroyPipelineLayout(arcDevice.device(), pipelineLayout, nullptr);
    }
}