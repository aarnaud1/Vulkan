/*
 * Copyright (C) 2024 Adrien ARNAUD
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/IMemoryObject.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vulkan/vulkan.h>

namespace vkw
{
class Image final : public IMemoryObject
{
  public:
    Image() {};

    Image(const Image &) = delete;
    Image(Image &&) = delete;

    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&) = delete;

    ~Image() { this->clear(); }

    bool isInitialized() const { return initialized_; }

    VkBufferUsageFlags getUsage() const { return usage_; }

    VkExtent3D getSize() const { return extent_; }

    bool bindResource(VkDeviceMemory mem, const size_t offset) override
    {
        VkResult res = vkBindImageMemory(device_->getHandle(), image_, mem, offset);
        if(res != VK_SUCCESS)
        {
            utils::Log::Error("vkw::Image", "Error binding memory - %s", string_VkResult(res));
            return false;
        }
        return true;
    }

    VkFormat getFormat() const { return format_; }

    VkImage &getHandle() { return image_; }
    const VkImage &getHandle() const { return image_; }

  private:
    friend class Memory;

    Device *device_{nullptr};

    VkFormat format_{};
    VkExtent3D extent_{};

    VkImageUsageFlags usage_{};
    VkImage image_{VK_NULL_HANDLE};

    bool initialized_{false};

    Image(
        Device &device,
        VkImageType imageType,
        VkFormat format,
        VkExtent3D extent,
        VkImageUsageFlags usage,
        uint32_t numLayers = 1,
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
        uint32_t mipLevels = 1,
        VkImageCreateFlags createFlags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
    {
        CHECK_BOOL_THROW(
            this->init(
                device,
                imageType,
                format,
                extent,
                usage,
                numLayers,
                tiling,
                mipLevels,
                createFlags,
                sharingMode),
            "Initializing image");
    }

    bool init(
        Device &device,
        VkImageType imageType,
        VkFormat format,
        VkExtent3D extent,
        VkImageUsageFlags usage,
        uint32_t numLayers = 1,
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
        uint32_t mipLevels = 1,
        VkImageCreateFlags flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
    {
        if(!initialized_)
        {
            this->device_ = &device;
            this->format_ = format;
            this->extent_ = extent;
            this->usage_ = usage;

            VkImageCreateInfo imgCreateInfo = {};
            imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imgCreateInfo.pNext = nullptr;
            imgCreateInfo.flags = flags;
            imgCreateInfo.imageType = imageType;
            imgCreateInfo.format = format;
            imgCreateInfo.extent = extent;
            imgCreateInfo.mipLevels = mipLevels;
            imgCreateInfo.arrayLayers = numLayers;
            imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imgCreateInfo.tiling = tiling;
            imgCreateInfo.usage = usage;
            imgCreateInfo.sharingMode = sharingMode;
            imgCreateInfo.queueFamilyIndexCount = 0;
            imgCreateInfo.pQueueFamilyIndices = nullptr;
            imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            VKW_INIT_CHECK_VK(
                vkCreateImage(device_->getHandle(), &imgCreateInfo, nullptr, &image_));

            VkMemoryRequirements memRequirements{};
            vkGetImageMemoryRequirements(device_->getHandle(), image_, &memRequirements);

            this->memAlign_ = memRequirements.alignment;
            this->memSize_ = memRequirements.size;
            this->memTypeBits_ = memRequirements.memoryTypeBits;

            initialized_ = true;
        }

        return true;
    }

    void clear()
    {
        memAlign_ = 0;
        memSize_ = 0;
        memOffset_ = 0;

        memTypeBits_ = 0;

        if(image_ != VK_NULL_HANDLE)
        {
            vkDestroyImage(device_->getHandle(), image_, nullptr);
        }

        device_ = nullptr;

        extent_ = {};
        format_ = {};
        usage_ = {};
        image_ = VK_NULL_HANDLE;

        initialized_ = false;
    }
};
} // namespace vkw
