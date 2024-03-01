
#include "arc_texture.hpp"
#include "arc_buffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// stb
#include <stdexcept>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace arc
{
    ArcTexture::ArcTexture(ArcDevice &arcDevice, const std::string &imagepath)
        : arcDevice{arcDevice}
    {
        createTextureImage(imagepath);
        createTextureImageView();
        createTextureSampler();
    }

    ArcTexture::~ArcTexture()
    {
        // the order matters
        vkDestroySampler(arcDevice.device(), textureSampler, nullptr);
        vkDestroyImageView(arcDevice.device(), textureImageView, nullptr);

        vkDestroyImage(arcDevice.device(), textureImage, nullptr);
        vkFreeMemory(arcDevice.device(), textureImageMemory, nullptr);
    }

    void ArcTexture::createTextureImage(const std::string &imagepath)
    {
        int texWidth, texHeight, texChannels;
        std::string enginePath = ENGINE_DIR + imagepath;
        stbi_uc *pixels = stbi_load(enginePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels)
        {
            throw std::runtime_error("failed to load image resource from " + imagepath);
        }

        VkDeviceSize imageSize = texWidth * texHeight * 4;
        ArcBuffer stagingBuffer{
            arcDevice,
            imageSize,
            1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)pixels);

        // clean up pixel
        stbi_image_free(pixels);

        createImage(texWidth, texHeight,
                    VK_FORMAT_R8G8B8A8_SRGB,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    textureImage, textureImageMemory);

        transitionImageLayout(textureImage,
                              VK_FORMAT_R8G8B8A8_SRGB,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        stagingBuffer.writeToImage(textureImage,
                                   static_cast<uint32_t>(texWidth),
                                   static_cast<uint32_t>(texHeight));
        transitionImageLayout(textureImage,
                              VK_FORMAT_R8G8B8A8_SRGB,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void ArcTexture::createImage(uint32_t width, uint32_t height, VkFormat format,
                                 VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                 VkImage &image, VkDeviceMemory &imageMemory)
    {
        // VkImageCreateInfo struct for creating an image
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // related to multisampling
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(arcDevice.device(), &imageInfo, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image!");
        }

        // allocating memory for an image
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(arcDevice.device(), textureImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = arcDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(arcDevice.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(arcDevice.device(), image, imageMemory, 0);
    }

    void ArcTexture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = arcDevice.beginSingleTimeCommands();

        // use an image memory barrier to perform layout transitions
        // transition image layouts and transfer queue family ownership
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;

        // if we are using the barrier to transfer queue family ownership
        // these two fields should be the indices of the queue families
        // they must be set to VK_QUEUE_FAMILY_IGNORED
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        // TODO: make this field adaptive
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        //  there are two transitions we need to handle:
        // 1. Undefined->transfer destination: transfer writes that don't need to wait on anything
        // 2. Transfer destination -> shader reading: shader reads should wait on transfer writes,
        //    specifically the shader reads in the fragment shader
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::runtime_error("unsupported layout transition");
        }

        vkCmdPipelineBarrier(commandBuffer,
                             sourceStage, destinationStage,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        arcDevice.endSingleTimeCommands(commandBuffer);
    }

    VkImageView ArcTexture::createImageView(VkImage image, VkFormat format)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(arcDevice.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }
        return imageView;
    }

    void ArcTexture::createTextureImageView()
    {
        textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
    }

    void ArcTexture::createTextureSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        // currently, we go for maximum quality
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = arcDevice.properties.limits.maxSamplerAnisotropy;
        // when sampling beyond the image with clamp to border addressing mode, which color is returned
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        // we use standard 0-1 texture coordinate
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        // if a comparison function is enabled, then texels will first be compared to a value
        // the result of that comparison is used in filtering operations
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        // midmapping
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.f;
        samplerInfo.minLod = 0.f;
        samplerInfo.maxLod = 0.f;
        // create sampler
        if (vkCreateSampler(arcDevice.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    VkDescriptorImageInfo ArcTexture::descriptorInfo()
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;
        return imageInfo;
    }

}
