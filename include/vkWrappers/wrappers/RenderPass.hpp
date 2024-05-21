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

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

#include "vkWrappers/wrappers/Device.hpp"

namespace vk
{
class RenderPass
{
  public:
    RenderPass(Device &device) : device_{&device} {}

    RenderPass(const RenderPass &) = delete;
    RenderPass(RenderPass &&) = delete;

    RenderPass &operator=(const RenderPass &) = delete;
    RenderPass &operator=(RenderPass &&) = delete;

    ~RenderPass() { release(); }

    VkRenderPass &getHandle() { return renderPass_; }
    const VkRenderPass &getHandle() const { return renderPass_; }

    RenderPass &addColorAttachment(
        const VkFormat format, const VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    RenderPass &addDepthAttachment(
        const VkFormat format, const VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

    RenderPass &addSubPass(
        const std::vector<uint32_t> &colorAttachments,
        const VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    RenderPass &addSubPass(
        const std::vector<uint32_t> &colorAttachments,
        const std::vector<uint32_t> &depthStencilAttachments,
        const VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    RenderPass &addSubpassDependency(
        const uint32_t srcSubpass,
        const uint32_t dstSubpass,
        const VkPipelineStageFlags srcStageMask,
        const VkPipelineStageFlags dstStageMask,
        const VkAccessFlags srcAccessMask,
        const VkAccessFlags dstAccessFlags,
        const VkDependencyFlags flags = 0)
    {
        if(renderPass_ != VK_NULL_HANDLE)
        {
            throw std::runtime_error("Attempting to modify an already allocated RenderPass");
        }

        VkSubpassDependency dependency{};
        dependency.srcSubpass = srcSubpass;
        dependency.dstSubpass = dstSubpass;
        dependency.srcStageMask = srcStageMask;
        dependency.dstStageMask = dstStageMask;
        dependency.srcAccessMask = srcAccessMask;
        dependency.dstAccessMask = dstAccessFlags;
        dependency.dependencyFlags = flags;

        subpassDependencies_.emplace_back(dependency);
        return *this;
    }

    RenderPass &create();
    RenderPass &release();

  private:
    Device *device_{nullptr};
    VkRenderPass renderPass_{VK_NULL_HANDLE};

    std::vector<VkAttachmentDescription> attachments_{};
    std::vector<VkAttachmentDescription> depthStencilAttachments_{};
    std::vector<VkSubpassDescription> subPasses_{};
    std::vector<VkSubpassDependency> subpassDependencies_{};

    std::vector<std::vector<VkAttachmentReference>> colorReferenceList_{};
    std::vector<std::vector<VkAttachmentReference>> depthStencilReferenceList_{};
};
} // namespace vk