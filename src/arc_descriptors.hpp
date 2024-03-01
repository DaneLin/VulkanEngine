#pragma once

#include "arc_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace arc
{
    // The blueprint of a descriptor set
    // tells how the bindings format
    class ArcDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder(ArcDevice &arcDevice) : arcDevice{arcDevice} {}

            Builder &addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<ArcDescriptorSetLayout> build() const;

        private:
            ArcDevice &arcDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        ArcDescriptorSetLayout(
            ArcDevice &arcDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~ArcDescriptorSetLayout();
        ArcDescriptorSetLayout(const ArcDescriptorSetLayout &) = delete;
        ArcDescriptorSetLayout &operator=(const ArcDescriptorSetLayout &) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        ArcDevice &arcDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class ArcDescriptorWriter;
    };

    class ArcDescriptorPool
    {
    public:
        class Builder
        {
        public:
            Builder(ArcDevice &arcDevice) : arcDevice{arcDevice} {}

            Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count);
            std::unique_ptr<ArcDescriptorPool> build() const;

        private:
            ArcDevice &arcDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        ArcDescriptorPool(
            ArcDevice &arcDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize> &poolSizes);
        ~ArcDescriptorPool();
        ArcDescriptorPool(const ArcDescriptorPool &) = delete;
        ArcDescriptorPool &operator=(const ArcDescriptorPool &) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

        void resetPool();

    private:
        ArcDevice &arcDevice;
        VkDescriptorPool descriptorPool;

        friend class ArcDescriptorWriter;
    };

    class ArcDescriptorWriter
    {
    public:
        ArcDescriptorWriter(ArcDescriptorSetLayout &setLayout, ArcDescriptorPool &pool);

        ArcDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        ArcDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

    private:
        ArcDescriptorSetLayout &setLayout;
        ArcDescriptorPool &pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

} // namespace Arc