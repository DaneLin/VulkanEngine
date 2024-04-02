#include "stencil_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>

namespace arc
{
    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };

    StencilSystem::StencilSystem(ArcDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : arcDevice(device)
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    void StencilSystem::render(FrameInfo &frameInfo)
    {
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &frameInfo.globalDescriptorSet,
            0, nullptr);

        stencil->bind(frameInfo.commandBuffer);
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

        outline->bind(frameInfo.commandBuffer);
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

    void StencilSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        layoutInfo.pSetLayouts = descriptorSetLayouts.data();
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(arcDevice.device(), &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void StencilSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(pipelineLayout != nullptr && "Cannot create a pipeline without pipeline layout");
        PipelineConfigInfo pipelineConfigInfo{};
        ArcPipeline::defaultPipelineConfigInfo(pipelineConfigInfo);
        pipelineConfigInfo.renderPass = renderPass;
        pipelineConfigInfo.pipelineLayout = pipelineLayout;
        pipelineConfigInfo.multisampleInfo.rasterizationSamples = arcDevice.getMaxUsableSampleCount();

        // In default settings, we won't enbale stencil test

        pipelineConfigInfo.depthStencilInfo.stencilTestEnable = VK_TRUE;
        pipelineConfigInfo.depthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
        pipelineConfigInfo.depthStencilInfo.back.failOp = VK_STENCIL_OP_REPLACE;
        pipelineConfigInfo.depthStencilInfo.back.depthFailOp = VK_STENCIL_OP_REPLACE;
        pipelineConfigInfo.depthStencilInfo.back.passOp = VK_STENCIL_OP_REPLACE;
        pipelineConfigInfo.depthStencilInfo.back.compareMask = 0xff;
        pipelineConfigInfo.depthStencilInfo.back.writeMask = 0xff;
        pipelineConfigInfo.depthStencilInfo.back.reference = 1;
        pipelineConfigInfo.depthStencilInfo.front = pipelineConfigInfo.depthStencilInfo.back;

        PipelineShaderConfigInfo shaderConfig{};
        shaderConfig.stageInfo.pSpecializationInfo = nullptr;

        stencil = std::make_unique<ArcPipeline>(
            arcDevice,
            "shaders/toon.vert.spv",
            "shaders/toon.frag.spv",
            pipelineConfigInfo,
            shaderConfig);

        pipelineConfigInfo.depthStencilInfo.back.compareOp = VK_COMPARE_OP_NOT_EQUAL;
        pipelineConfigInfo.depthStencilInfo.back.failOp = VK_STENCIL_OP_KEEP;
        pipelineConfigInfo.depthStencilInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
        pipelineConfigInfo.depthStencilInfo.back.passOp = VK_STENCIL_OP_REPLACE;
        pipelineConfigInfo.depthStencilInfo.front = pipelineConfigInfo.depthStencilInfo.back;
        pipelineConfigInfo.depthStencilInfo.depthTestEnable = VK_FALSE;

        outline = std::make_unique<ArcPipeline>(
            arcDevice,
            "shaders/outline.vert.spv",
            "shaders/outline.frag.spv",
            pipelineConfigInfo,
            shaderConfig);
    }

    StencilSystem::~StencilSystem()
    {
        vkDestroyPipelineLayout(arcDevice.device(), pipelineLayout, nullptr);
    }
}