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

#include "vkWrappers/wrappers/CommandBuffer.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/Queue.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw
{
class CommandPool
{
  public:
    CommandPool() {}
    CommandPool(
        Device &device,
        Queue &queue,
        VkCommandPoolCreateFlags flags
        = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
    {
        this->init(device, queue, flags);
    }

    CommandPool(const CommandPool &) = delete;
    CommandPool(CommandPool &&cp) { *this = std::move(cp); }

    CommandPool &operator=(const CommandPool &) = delete;
    CommandPool &operator=(CommandPool &&cp)
    {
        this->clear();

        std::swap(device_, cp.device_);
        std::swap(commandPool_, cp.commandPool_);
        std::swap(initialized_, cp.initialized_);

        return *this;
    }

    ~CommandPool() { this->clear(); }

    void init(
        Device &device,
        Queue &queue,
        VkCommandPoolCreateFlags flags
        = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
    {
        if(!initialized_)
        {
            device_ = &device;

            VkCommandPoolCreateInfo createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = flags;
            createInfo.queueFamilyIndex = queue.queueFamilyIndex();

            CHECK_VK(
                vkCreateCommandPool(device_->getHandle(), &createInfo, nullptr, &commandPool_),
                "Creating command pool");

            initialized_ = true;
        }
    }

    void clear()
    {
        if(commandPool_ != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device_->getHandle(), commandPool_, nullptr);
        }
        device_ = nullptr;
        commandPool_ = VK_NULL_HANDLE;
        initialized_ = false;
    }

    bool isInitialized() const { return initialized_; }

    CommandBuffer createCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
    {
        return CommandBuffer(*device_, commandPool_, level);
    }

    std::vector<CommandBuffer> createCommandBuffers(
        const size_t n, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
    {
        std::vector<CommandBuffer> ret;
        for(size_t i = 0; i < n; ++i)
        {
            ret.emplace_back(CommandBuffer(*device_, commandPool_, level));
        }
        return ret;
    }

    VkCommandPool &getHandle() { return commandPool_; }
    const VkCommandPool &getHandle() const { return commandPool_; }

  private:
    Device *device_{nullptr};
    VkCommandPool commandPool_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw
