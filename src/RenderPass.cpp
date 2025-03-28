/*
 * Copyright (C) 2025 Adrien ARNAUD
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

#include "vkWrappers/wrappers/RenderPass.hpp"

#include "vkWrappers/wrappers/utils.hpp"

namespace vkw
{
RenderPass::RenderPass(Device& device)
{
    VKW_CHECK_BOOL_THROW(this->init(device), "Creating render pass");
}

RenderPass::RenderPass(RenderPass&& rhs) { *this = std::move(rhs); }

RenderPass& RenderPass::operator=(RenderPass&& rhs)
{
    this->clear();

    std::swap(device_, rhs.device_);
    std::swap(renderPass_, rhs.renderPass_);

    std::swap(attachments_, rhs.attachments_);
    std::swap(depthStencilAttachments_, rhs.depthStencilAttachments_);
    std::swap(resolveAttachments_, rhs.resolveAttachments_);
    std::swap(subPasses_, rhs.subPasses_);
    std::swap(subpassDependencies_, rhs.subpassDependencies_);

    std::swap(colorReferenceList_, rhs.colorReferenceList_);
    std::swap(depthStencilReferenceList_, rhs.depthStencilReferenceList_);
    std::swap(resolveReferenceList_, rhs.resolveReferenceList_);

    std::swap(initialized_, rhs.initialized_);

    return *this;
}

bool RenderPass::init(Device& device)
{
    if(!initialized_)
    {
        device_ = &device;
        initialized_ = true;
    }

    return true;
}

void RenderPass::clear()
{
    VKW_DELETE_VK(RenderPass, renderPass_);

    attachments_.clear();
    depthStencilAttachments_.clear();
    resolveAttachments_.clear();
    subPasses_.clear();
    subpassDependencies_.clear();

    colorReferenceList_.clear();
    depthStencilReferenceList_.clear();
    resolveReferenceList_.clear();

    device_ = nullptr;
    initialized_ = false;
}

void RenderPass::create()
{
    // Update subpass info
    const bool useDepthStencil = !depthStencilReferenceList_.empty();
    const bool useResolve = !resolveReferenceList_.empty();

    for(size_t i = 0; i < colorReferenceList_.size(); ++i)
    {
        subPasses_[i].pColorAttachments = colorReferenceList_[i].data();
        if(useDepthStencil)
        {
            subPasses_[i].pDepthStencilAttachment = depthStencilReferenceList_[i].data();
        }
        if(useResolve)
        {
            subPasses_[i].pResolveAttachments = resolveReferenceList_[i].data();
        }
    }

    std::vector<VkAttachmentDescription> attachmentList;
    attachmentList.insert(attachmentList.end(), attachments_.begin(), attachments_.end());
    if(useDepthStencil)
    {
        attachmentList.insert(
            attachmentList.end(), depthStencilAttachments_.begin(), depthStencilAttachments_.end());
    }
    if(useResolve)
    {
        attachmentList.insert(
            attachmentList.end(), depthStencilAttachments_.begin(), resolveAttachments_.end());
    }

    VkRenderPassCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.attachmentCount = static_cast<uint32_t>(attachmentList.size());
    createInfo.pAttachments = attachmentList.data();
    createInfo.subpassCount = static_cast<uint32_t>(subPasses_.size());
    createInfo.pSubpasses = subPasses_.data();
    createInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies_.size());
    createInfo.pDependencies = subpassDependencies_.data();
    VKW_CHECK_VK_THROW(
        device_->vk().vkCreateRenderPass(device_->getHandle(), &createInfo, nullptr, &renderPass_),
        "Creating render pass");
}

RenderPass& RenderPass::addColorAttachment(
    const ColorRenderTarget& attachment,
    const VkImageLayout initialLayout,
    const VkImageLayout finalLayout,
    const VkSampleCountFlagBits samples)
{
    VkAttachmentDescription descr{};
    descr.format = attachment.format();
    descr.samples = samples;
    descr.loadOp = attachment.loadOp();
    descr.storeOp = attachment.storeOp();
    descr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    descr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    descr.initialLayout = initialLayout;
    descr.finalLayout = finalLayout;

    attachments_.emplace_back(descr);
    return *this;
}

RenderPass& RenderPass::addDepthStencilAttachment(
    const DepthStencilRenderTarget& attachment,
    const VkImageLayout initialLayout,
    const VkImageLayout finalLayout,
    const VkSampleCountFlagBits samples)
{
    VkAttachmentDescription descr{};
    descr.format = attachment.format();
    descr.samples = samples;
    descr.loadOp = attachment.loadOp();
    descr.storeOp = attachment.storeOp();
    descr.stencilLoadOp = attachment.loadOp();
    descr.stencilStoreOp = attachment.storeOp();
    descr.initialLayout = initialLayout;
    descr.finalLayout = finalLayout;

    depthStencilAttachments_.emplace_back(descr);
    return *this;
}

RenderPass& RenderPass::addColorAttachment(
    const VkFormat format,
    const VkImageLayout initialLayout,
    const VkImageLayout finalLayout,
    const VkAttachmentLoadOp loadOp,
    const VkAttachmentStoreOp storeOp,
    const VkSampleCountFlagBits samples)
{
    VkAttachmentDescription descr{};
    descr.format = format;
    descr.samples = samples;
    descr.loadOp = loadOp;
    descr.storeOp = storeOp;
    descr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    descr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    descr.initialLayout = initialLayout;
    descr.finalLayout = finalLayout;

    attachments_.emplace_back(descr);
    return *this;
}

RenderPass& RenderPass::addDepthStencilAttachment(
    const VkFormat format,
    const VkImageLayout initialLayout,
    const VkImageLayout finalLayout,
    const VkAttachmentLoadOp loadOp,
    const VkAttachmentStoreOp storeOp,
    const VkAttachmentLoadOp stencilLoadOp,
    const VkAttachmentStoreOp stencilStoreOp,
    const VkSampleCountFlagBits samples)
{
    VkAttachmentDescription descr{};
    descr.format = format;
    descr.samples = samples;
    descr.loadOp = loadOp;
    descr.storeOp = storeOp;
    descr.stencilLoadOp = stencilLoadOp;
    descr.stencilStoreOp = stencilStoreOp;
    descr.initialLayout = initialLayout;
    descr.finalLayout = finalLayout;

    depthStencilAttachments_.emplace_back(descr);
    return *this;
}

RenderPass& RenderPass::addSubPass(
    const std::vector<uint32_t>& colorAttachments, const VkPipelineBindPoint bindPoint)
{
    if(renderPass_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Attempting to modify an already allocated RenderPass");
    }

    std::vector<VkAttachmentReference> colorAttachmentsList{};
    colorAttachmentsList.resize(colorAttachments.size());
    for(size_t i = 0; i < colorAttachments.size(); ++i)
    {
        VkAttachmentReference ref{};
        ref.attachment = colorAttachments[i];
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentsList[i] = ref;
    }
    colorReferenceList_.emplace_back(std::move(colorAttachmentsList));

    VkSubpassDescription subPass{};
    subPass.flags = 0;
    subPass.pipelineBindPoint = bindPoint;
    subPass.inputAttachmentCount = 0;
    subPass.pInputAttachments = nullptr;
    subPass.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
    subPass.pColorAttachments = nullptr;
    subPass.pResolveAttachments = nullptr;
    subPass.pDepthStencilAttachment = nullptr;
    subPass.preserveAttachmentCount = 0;
    subPass.pPreserveAttachments = nullptr;

    subPasses_.emplace_back(subPass);
    return *this;
}

RenderPass& RenderPass::addSubPass(
    const std::vector<uint32_t>& colorAttachments,
    const std::vector<uint32_t>& depthStencilAttachments,
    const VkPipelineBindPoint bindPoint)
{
    if(renderPass_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Attempting to modify an already allocated RenderPass");
    }

    if(colorAttachments.size() != depthStencilAttachments.size())
    {
        throw std::runtime_error("Color and depth attachment counts must be equal");
    }

    std::vector<VkAttachmentReference> colorAttachmentsList{};
    colorAttachmentsList.resize(colorAttachments.size());
    for(size_t i = 0; i < colorAttachments.size(); ++i)
    {
        VkAttachmentReference ref{};
        ref.attachment = colorAttachments[i];
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentsList[i] = ref;
    }
    colorReferenceList_.emplace_back(std::move(colorAttachmentsList));

    std::vector<VkAttachmentReference> depthStencilAttachmentsList{};
    depthStencilAttachmentsList.resize(depthStencilAttachments.size());
    for(size_t i = 0; i < depthStencilAttachments.size(); ++i)
    {
        VkAttachmentReference ref{};
        ref.attachment = depthStencilAttachments[i];
        ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachmentsList[i] = ref;
    }
    depthStencilReferenceList_.emplace_back(std::move(depthStencilAttachmentsList));

    VkSubpassDescription subPass{};
    subPass.flags = 0;
    subPass.pipelineBindPoint = bindPoint;
    subPass.inputAttachmentCount = 0;
    subPass.pInputAttachments = nullptr;
    subPass.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
    subPass.pColorAttachments = nullptr;
    subPass.pResolveAttachments = nullptr;
    subPass.pDepthStencilAttachment = nullptr;
    subPass.preserveAttachmentCount = 0;
    subPass.pPreserveAttachments = nullptr;

    subPasses_.emplace_back(subPass);
    return *this;
}

RenderPass& RenderPass::addSubPassWithResolve(
    const std::vector<uint32_t>& colorAttachments,
    const std::vector<uint32_t>& resolveAttachments,
    const VkPipelineBindPoint bindPoint)
{
    if(renderPass_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Attempting to modify an already allocated RenderPass");
    }

    if(colorAttachments.size() != resolveAttachments.size())
    {
        throw std::runtime_error("Color and resolve attachment counts must be equal");
    }

    std::vector<VkAttachmentReference> colorAttachmentsList{};
    colorAttachmentsList.resize(colorAttachments.size());
    for(size_t i = 0; i < colorAttachments.size(); ++i)
    {
        VkAttachmentReference ref{};
        ref.attachment = colorAttachments[i];
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentsList[i] = ref;
    }
    colorReferenceList_.emplace_back(std::move(colorAttachmentsList));

    std::vector<VkAttachmentReference> resolveAttachmentsList{};
    resolveAttachmentsList.resize(resolveAttachments.size());
    for(size_t i = 0; i < resolveAttachments.size(); ++i)
    {
        VkAttachmentReference ref{};
        ref.attachment = resolveAttachments[i];
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        resolveAttachmentsList[i] = ref;
    }
    resolveReferenceList_.emplace_back(resolveAttachmentsList);

    VkSubpassDescription subPass{};
    subPass.flags = 0;
    subPass.pipelineBindPoint = bindPoint;
    subPass.inputAttachmentCount = 0;
    subPass.pInputAttachments = nullptr;
    subPass.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
    subPass.pColorAttachments = nullptr;
    subPass.pResolveAttachments = nullptr;
    subPass.pDepthStencilAttachment = nullptr;
    subPass.pResolveAttachments = nullptr;
    subPass.preserveAttachmentCount = 0;
    subPass.pPreserveAttachments = nullptr;

    subPasses_.emplace_back(subPass);
    return *this;
}

RenderPass& RenderPass::addSubPassWithResolve(
    const std::vector<uint32_t>& colorAttachments,
    const std::vector<uint32_t>& depthStencilAttachments,
    const std::vector<uint32_t>& resolveAttachments,
    const VkPipelineBindPoint bindPoint)
{
    if(renderPass_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Attempting to modify an already allocated RenderPass");
    }

    if(colorAttachments.size() != depthStencilAttachments.size())
    {
        throw std::runtime_error("Color and depth attachment counts must be equal");
    }
    if(colorAttachments.size() != resolveAttachments.size())
    {
        throw std::runtime_error("Color and resolve attachment counts must be equal");
    }

    std::vector<VkAttachmentReference> colorAttachmentsList{};
    colorAttachmentsList.resize(colorAttachments.size());
    for(size_t i = 0; i < colorAttachments.size(); ++i)
    {
        VkAttachmentReference ref{};
        ref.attachment = colorAttachments[i];
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentsList[i] = ref;
    }
    colorReferenceList_.emplace_back(std::move(colorAttachmentsList));

    std::vector<VkAttachmentReference> depthStencilAttachmentsList{};
    depthStencilAttachmentsList.resize(depthStencilAttachments.size());
    for(size_t i = 0; i < depthStencilAttachments.size(); ++i)
    {
        VkAttachmentReference ref{};
        ref.attachment = depthStencilAttachments[i];
        ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachmentsList[i] = ref;
    }
    depthStencilReferenceList_.emplace_back(std::move(depthStencilAttachmentsList));

    std::vector<VkAttachmentReference> resolveAttachmentsList{};
    resolveAttachmentsList.resize(resolveAttachments.size());
    for(size_t i = 0; i < resolveAttachments.size(); ++i)
    {
        VkAttachmentReference ref{};
        ref.attachment = resolveAttachments[i];
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        resolveAttachmentsList[i] = ref;
    }
    resolveReferenceList_.emplace_back(resolveAttachmentsList);

    VkSubpassDescription subPass{};
    subPass.flags = 0;
    subPass.pipelineBindPoint = bindPoint;
    subPass.inputAttachmentCount = 0;
    subPass.pInputAttachments = nullptr;
    subPass.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
    subPass.pColorAttachments = nullptr;
    subPass.pResolveAttachments = nullptr;
    subPass.pDepthStencilAttachment = nullptr;
    subPass.pResolveAttachments = nullptr;
    subPass.preserveAttachmentCount = 0;
    subPass.pPreserveAttachments = nullptr;

    subPasses_.emplace_back(subPass);
    return *this;
}
} // namespace vkw