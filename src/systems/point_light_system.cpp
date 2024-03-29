#include "point_light_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <map>

namespace arc
{
    struct PointLightPushConstants
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(ArcDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : arcDevice{device}
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem()
    {
        vkDestroyPipelineLayout(arcDevice.device(), pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);

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

    void PointLightSystem::createPipeline(VkRenderPass renderPass)
    {

        assert(pipelineLayout != nullptr && "Cannot craete pipeline before pipeline layout!");
        PipelineConfigInfo pipelineConfig{};
        ArcPipeline::defaultPipelineConfigInfo(pipelineConfig);
        ArcPipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = arcDevice.getMaxUsableSampleCount();

        PipelineShaderConfigInfo shaderConfig{};
        shaderConfig.stageInfo.pSpecializationInfo = nullptr;
        // pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
        arcPipeline = std::make_unique<ArcPipeline>(
            arcDevice,
            "shaders/point_light.vert.spv",
            "shaders/point_light.frag.spv",
            pipelineConfig,
            shaderConfig);
    }

    void PointLightSystem::update(FrameInfo &frameInfo, GlobalUbo &ubo)
    {
        auto rotateLight = glm::rotate(glm::mat4(1.f), .5f * frameInfo.frameTime, {0.f, -1.f, 0.f});
        int lightIndex = 0;
        for (auto &kv : frameInfo.gameObjects)
        {
            auto &obj = kv.second;
            if (obj.pointLight == nullptr)
                continue;

            assert(lightIndex <= MAX_LIGHTS && "Point lights should be less than MAX_LIGHTS");

            // update light position
            obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));

            // copy light to ubo
            ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
            ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            lightIndex++;
        }
        ubo.numLights = lightIndex;
    }

    void PointLightSystem::render(FrameInfo &frameInfo)
    {
        // sort lights
        std::map<float, ArcGameObject::id_t> sorted;
        for (auto &kv : frameInfo.gameObjects)
        {
            auto &obj = kv.second;
            if (obj.pointLight == nullptr)
                continue;

            // calculate distance
            auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
            float disSquared = glm::dot(offset, offset);
            sorted[disSquared] = obj.getID();
        }

        arcPipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &frameInfo.globalDescriptorSet,
            0, nullptr);

        // iterate through sorted lights in reverse order
        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            // use game obj id to find light object
            auto &obj = frameInfo.gameObjects.at(it->second);

            PointLightPushConstants push{};
            push.position = glm::vec4(obj.transform.translation, 1.f);
            push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            push.radius = obj.transform.scale.x;

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstants),
                &push);
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }
}