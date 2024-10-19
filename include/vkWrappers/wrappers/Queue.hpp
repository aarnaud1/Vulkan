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

#include "vkWrappers/wrappers/utils.hpp"

#include <vector>
#include <vulkan/vulkan.h>

namespace vkw
{
enum QueueUsageBits
{
    VKW_QUEUE_GRAPHICS_BIT = 0x01,
    VKW_QUEUE_COMPUTE_BIT = 0x02,
    VKW_QUEUE_TRANSFER_BIT = 0x04,
    VKW_QUEUE_PRESENT_BIT = 0x08
};
typedef uint32_t QueueUsageFlags;

class Queue
{
    // NOTE : because Device is not visible from this class, methods must use templates to designate
    // other vkw objects here.
    // NOTE: we could create "base" class objects with the name and a valid getHandle() method, if
    // compilation is too long.
  public:
    Queue() {}

    Queue(const Queue &cp) = default;
    Queue(Queue &&) = default;

    Queue &operator=(const Queue &cp) = default;
    Queue &operator=(Queue &&) = default;

    ~Queue() {}

    VkQueueFlags flags() const { return flags_; }
    uint32_t queueFamilyIndex() const { return queueFamilyIndex_; }
    uint32_t queueIndex() const { return queueIndex_; }

    VkQueue getHandle() const { return queue_; }

    template <typename CommandBuffer>
    Queue &submit(CommandBuffer &cmdBuffer)
    {
        VkSubmitInfo submitInfo
            = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
               nullptr,
               0,
               nullptr,
               nullptr,
               1,
               &(cmdBuffer.getHandle()),
               0,
               nullptr};
        vkQueueSubmit(queue_, 1, &submitInfo, nullptr);
        return *this;
    }

    template <typename CommandBuffer, typename Fence>
    Queue &submit(CommandBuffer &cmdBuffer, const Fence &fence)
    {
        VkSubmitInfo submitInfo
            = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
               nullptr,
               0,
               nullptr,
               nullptr,
               1,
               &(cmdBuffer.getHandle()),
               0,
               nullptr};
        vkQueueSubmit(queue_, 1, &submitInfo, fence.getHandle());
        return *this;
    }

    template <typename CommandBuffer, typename Semaphore>
    Queue &submit(
        CommandBuffer &cmdBuffer,
        const std::vector<Semaphore *> &waitSemaphores,
        const std::vector<VkPipelineStageFlags> &waitFlags,
        const std::vector<Semaphore *> &signalSemaphores)
    {
        std::vector<VkSemaphore> waitSemaphoreValues;
        waitSemaphoreValues.reserve(waitSemaphores.size());
        for(size_t i = 0; i < waitSemaphores.size(); ++i)
        {
            waitSemaphoreValues.emplace_back(waitSemaphores[i]->getHandle());
        }

        std::vector<VkSemaphore> signalSemaphoreValues;
        signalSemaphoreValues.reserve(signalSemaphores.size());
        for(size_t i = 0; i < signalSemaphores.size(); ++i)
        {
            signalSemaphoreValues.emplace_back(signalSemaphores[i]->getHandle());
        }

        VkSubmitInfo submitInfo
            = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
               nullptr,
               static_cast<uint32_t>(waitSemaphores.size()),
               waitSemaphoreValues.data(),
               waitFlags.data(),
               1,
               &(cmdBuffer.getHandle()),
               static_cast<uint32_t>(signalSemaphores.size()),
               signalSemaphoreValues.data()};
        vkQueueSubmit(queue_, 1, &submitInfo, VK_NULL_HANDLE);
        return *this;
    }

    template <typename CommandBuffer, typename Semaphore, typename Fence>
    Queue &submit(
        CommandBuffer &cmdBuffer,
        const std::vector<Semaphore *> &waitSemaphores,
        const std::vector<VkPipelineStageFlags> &waitFlags,
        const std::vector<Semaphore *> &signalSemaphores,
        Fence &fence)
    {
        std::vector<VkSemaphore> waitSemaphoreValues;
        waitSemaphoreValues.reserve(waitSemaphores.size());
        for(size_t i = 0; i < waitSemaphores.size(); ++i)
        {
            waitSemaphoreValues.emplace_back(waitSemaphores[i]->getHandle());
        }

        std::vector<VkSemaphore> signalSemaphoreValues;
        signalSemaphoreValues.reserve(signalSemaphores.size());
        for(size_t i = 0; i < signalSemaphores.size(); ++i)
        {
            signalSemaphoreValues.emplace_back(signalSemaphores[i]->getHandle());
        }

        VkSubmitInfo submitInfo
            = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
               nullptr,
               static_cast<uint32_t>(waitSemaphores.size()),
               waitSemaphoreValues.data(),
               waitFlags.data(),
               1,
               &(cmdBuffer.getHandle()),
               static_cast<uint32_t>(signalSemaphores.size()),
               signalSemaphoreValues.data()};
        vkQueueSubmit(queue_, 1, &submitInfo, fence.getHandle());
        return *this;
    }

    template <typename Swapchain, typename Semaphore>
    Queue &present(
        Swapchain &swapchain,
        const std::vector<Semaphore *> &waitSemaphores,
        const uint32_t imageIndex)
    {
        std::vector<VkSemaphore> waitSemaphoreValues;
        waitSemaphoreValues.reserve(waitSemaphores.size());
        for(size_t i = 0; i < waitSemaphores.size(); ++i)
        {
            waitSemaphoreValues.emplace_back(waitSemaphores[i]->getHandle());
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphoreValues.size());
        presentInfo.pWaitSemaphores = waitSemaphoreValues.data();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain.getHandle();
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        vkQueuePresentKHR(queue_, &presentInfo);
        return *this;
    }

    Queue &waitIdle()
    {
        vkQueueWaitIdle(queue_);
        return *this;
    }

  private:
    friend class Device;

    QueueUsageFlags flags_{0};

    uint32_t queueFamilyIndex_{0};
    uint32_t queueIndex_{0};

    VkQueue queue_{VK_NULL_HANDLE};
};
} // namespace vkw
