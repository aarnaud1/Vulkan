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

#include "vkWrappers/wrappers/GraphicsPipeline.hpp"

#include <stdexcept>

namespace vkw
{
GraphicsPipeline::GraphicsPipeline(Device& device) { this->init(device); }

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& cp) { *this = std::move(cp); }

GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& cp)
{
    this->clear();

    std::swap(device_, cp.device_);
    std::swap(pipeline_, cp.pipeline_);
    std::swap(bindingDescriptions_, cp.bindingDescriptions_);
    std::swap(attributeDescriptions_, cp.attributeDescriptions_);

    std::swap(viewports_, cp.viewports_);
    std::swap(scissors_, cp.scissors_);
    std::swap(colorBlendAttachmentStates_, cp.colorBlendAttachmentStates_);

    std::swap(vertexInputStateInfo_, cp.vertexInputStateInfo_);
    std::swap(inputAssemblyStateInfo_, cp.inputAssemblyStateInfo_);
    std::swap(tesselationStateInfo_, cp.tesselationStateInfo_);
    std::swap(viewportStateInfo_, cp.viewportStateInfo_);
    std::swap(rasterizationStateInfo_, cp.rasterizationStateInfo_);
    std::swap(multisamplingStateInfo_, cp.multisamplingStateInfo_);
    std::swap(depthStencilStateInfo_, cp.depthStencilStateInfo_);
    std::swap(colorBlendStateInfo_, cp.colorBlendStateInfo_);
    std::swap(dynamicStateInfo_, cp.dynamicStateInfo_);

    std::swap(moduleInfo_, cp.moduleInfo_);
    std::swap(initialized_, cp.initialized_);

    return *this;
}

GraphicsPipeline::~GraphicsPipeline() { this->clear(); }

void GraphicsPipeline::init(Device& device)
{
    if(!initialized_)
    {
        device_ = &device;

        // Add one color blend attachment by default
        colorBlendAttachmentStates_.resize(1);
        colorBlendAttachmentStates_[0].colorWriteMask
            = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
              | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachmentStates_[0].blendEnable = VK_FALSE;
        colorBlendAttachmentStates_[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachmentStates_[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachmentStates_[0].colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachmentStates_[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachmentStates_[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachmentStates_[0].alphaBlendOp = VK_BLEND_OP_ADD;

        viewports_.resize(1);
        scissors_.resize(1);

        // Input assembly
        inputAssemblyStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateInfo_.pNext = nullptr;
        inputAssemblyStateInfo_.flags = 0;
        inputAssemblyStateInfo_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateInfo_.primitiveRestartEnable = false;

        // Vertex input
        vertexInputStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateInfo_.pNext = nullptr;

        // Tesselation
        tesselationStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tesselationStateInfo_.pNext = nullptr;

        // Viewport
        viewportStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateInfo_.pNext = nullptr;

        // Rasterization
        rasterizationStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateInfo_.depthClampEnable = VK_FALSE;
        rasterizationStateInfo_.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateInfo_.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateInfo_.lineWidth = 1.0f;
        rasterizationStateInfo_.cullMode = VK_CULL_MODE_FRONT_BIT;
        rasterizationStateInfo_.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateInfo_.depthBiasEnable = VK_FALSE;
        rasterizationStateInfo_.depthBiasConstantFactor = 0.0f;
        rasterizationStateInfo_.depthBiasClamp = 0.0f;
        rasterizationStateInfo_.depthBiasSlopeFactor = 0.0f;

        // MultiSample
        multisamplingStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisamplingStateInfo_.sampleShadingEnable = VK_FALSE;
        multisamplingStateInfo_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisamplingStateInfo_.minSampleShading = 1.0f;
        multisamplingStateInfo_.pSampleMask = nullptr;
        multisamplingStateInfo_.alphaToCoverageEnable = VK_FALSE;
        multisamplingStateInfo_.alphaToOneEnable = VK_FALSE;

        // Depth stencil
        depthStencilStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateInfo_.pNext = nullptr;
        depthStencilStateInfo_.depthTestEnable = VK_TRUE;
        depthStencilStateInfo_.depthWriteEnable = VK_TRUE;
        depthStencilStateInfo_.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilStateInfo_.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateInfo_.minDepthBounds = 0.0f;
        depthStencilStateInfo_.maxDepthBounds = 1.0f;
        depthStencilStateInfo_.stencilTestEnable = VK_FALSE;
        depthStencilStateInfo_.front = {};
        depthStencilStateInfo_.back = {};

        // Color blend
        colorBlendStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateInfo_.logicOpEnable = VK_FALSE;
        colorBlendStateInfo_.logicOp = VK_LOGIC_OP_COPY;
        colorBlendStateInfo_.attachmentCount = 0;
        colorBlendStateInfo_.pAttachments = nullptr;
        colorBlendStateInfo_.blendConstants[0] = 0.0f;
        colorBlendStateInfo_.blendConstants[1] = 0.0f;
        colorBlendStateInfo_.blendConstants[2] = 0.0f;
        colorBlendStateInfo_.blendConstants[3] = 0.0f;

        // Dynamic state
        dynamicStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo_.dynamicStateCount = static_cast<uint32_t>(dynamicStates_.size());
        dynamicStateInfo_.pDynamicStates = dynamicStates_.data();

        initialized_ = true;
    }
}

void GraphicsPipeline::clear()
{
    if(pipeline_ != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device_->getHandle(), pipeline_, nullptr);
    }

    device_ = nullptr;
    pipeline_ = VK_NULL_HANDLE;

    bindingDescriptions_.clear();
    attributeDescriptions_.clear();

    for(auto& info : moduleInfo_)
    {
        info = {};
    }

    initialized_ = false;
}

GraphicsPipeline& GraphicsPipeline::addShaderStage(
    const VkShaderStageFlagBits stage, const std::string& shaderSource)
{
    if(pipeline_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Adding shaders to a created pipeline");
    }

    const int id = getStageIndex(stage);
    if(id < 0)
    {
        throw std::runtime_error("Unsupported shader stage for graphics pipeline");
    }

    auto& info = moduleInfo_[id];
    info.shaderSource = shaderSource;

    return *this;
}

GraphicsPipeline& GraphicsPipeline::addVertexBinding(
    const uint32_t binding, const uint32_t stride, const VkVertexInputRate inputRate)
{
    if(pipeline_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Adding vertex binding to a created pipeline");
    }
    bindingDescriptions_.emplace_back(VkVertexInputBindingDescription{binding, stride, inputRate});
    return *this;
}

GraphicsPipeline& GraphicsPipeline::addVertexAttribute(
    const uint32_t location, const uint32_t binding, const VkFormat format, const uint32_t offset)
{
    if(pipeline_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Adding vertex attribute to a created pipeline");
    }
    attributeDescriptions_.emplace_back(
        VkVertexInputAttributeDescription{location, binding, format, offset});
    return *this;
}

void GraphicsPipeline::createPipeline(
    RenderPass& renderPass, PipelineLayout& pipelineLayout, const uint32_t subPass)
{
    for(size_t id = 0; id < maxStageCount; ++id)
    {
        auto& info = moduleInfo_[id];
        if(!info.shaderSource.empty())
        {
            info.shaderModule = utils::createShaderModule(
                device_->getHandle(), utils::readShader(info.shaderSource));
        }
    }

    std::array<std::vector<VkSpecializationMapEntry>, maxStageCount> specMaps;
    for(size_t id = 0; id < maxStageCount; ++id)
    {
        size_t offset = 0;
        auto& specMap = specMaps[id];
        for(size_t i = 0; i < moduleInfo_[id].specSizes.size(); i++)
        {
            VkSpecializationMapEntry mapEntry
                = {static_cast<uint32_t>(i),
                   static_cast<uint32_t>(offset),
                   moduleInfo_[id].specSizes[i]};
            specMap.push_back(mapEntry);
            offset += moduleInfo_[id].specSizes[i];
        }
    }

    std::vector<VkSpecializationInfo> specInfoList;
    std::vector<VkPipelineShaderStageCreateInfo> stageCreateInfoList;

    // Important : pre allocate data to avoid reallocation
    specInfoList.reserve(maxStageCount);
    stageCreateInfoList.reserve(maxStageCount);

    auto addShaderSpecInfo = [&](const int id, const auto stage) {
        if(moduleInfo_[id].shaderModule == VK_NULL_HANDLE)
        {
            return;
        }
        auto& specMap = specMaps[id];
        auto& specData = moduleInfo_[id].specData;

        VkSpecializationInfo specInfo{};
        specInfo.mapEntryCount = specMap.size();
        specInfo.pMapEntries = specMap.data();
        specInfo.pData = specData.data();
        specInfo.dataSize = specData.size();

        specInfoList.emplace_back(specInfo);

        auto& specSizes = moduleInfo_[id].specSizes;

        VkPipelineShaderStageCreateInfo stageCreateInfo{};
        stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageCreateInfo.pNext = nullptr;
        stageCreateInfo.flags = 0;
        stageCreateInfo.stage = stage;
        stageCreateInfo.module = moduleInfo_[id].shaderModule;
        stageCreateInfo.pName = "main";
        stageCreateInfo.pSpecializationInfo = specSizes.size() > 0 ? &specInfo : nullptr;

        stageCreateInfoList.emplace_back(stageCreateInfo);
    };
    addShaderSpecInfo(0, VK_SHADER_STAGE_VERTEX_BIT);
    addShaderSpecInfo(1, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    addShaderSpecInfo(2, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    addShaderSpecInfo(3, VK_SHADER_STAGE_GEOMETRY_BIT);
    addShaderSpecInfo(4, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Viewport
    viewportStateInfo_.viewportCount = static_cast<uint32_t>(viewports_.size());
    viewportStateInfo_.pViewports = viewports_.data();
    viewportStateInfo_.scissorCount = static_cast<uint32_t>(scissors_.size());
    viewportStateInfo_.pScissors = scissors_.data();

    // Vertex input
    vertexInputStateInfo_.flags = 0;
    vertexInputStateInfo_.vertexBindingDescriptionCount
        = static_cast<uint32_t>(bindingDescriptions_.size());
    vertexInputStateInfo_.pVertexBindingDescriptions = bindingDescriptions_.data();
    vertexInputStateInfo_.vertexAttributeDescriptionCount
        = static_cast<uint32_t>(attributeDescriptions_.size());
    vertexInputStateInfo_.pVertexAttributeDescriptions = attributeDescriptions_.data();

    colorBlendStateInfo_.attachmentCount
        = static_cast<uint32_t>(colorBlendAttachmentStates_.size());
    colorBlendStateInfo_.pAttachments = colorBlendAttachmentStates_.data();

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.stageCount = static_cast<uint32_t>(stageCreateInfoList.size());
    createInfo.pStages = stageCreateInfoList.data();
    createInfo.pVertexInputState = &vertexInputStateInfo_;
    createInfo.pInputAssemblyState = &inputAssemblyStateInfo_;
    createInfo.pTessellationState = nullptr; // TODO : not supported yet
    createInfo.pViewportState = &viewportStateInfo_;
    createInfo.pRasterizationState = &rasterizationStateInfo_;
    createInfo.pMultisampleState = &multisamplingStateInfo_;
    createInfo.pDepthStencilState = &depthStencilStateInfo_;
    createInfo.pColorBlendState = &colorBlendStateInfo_;
    createInfo.pDynamicState = &dynamicStateInfo_;
    createInfo.layout = pipelineLayout.getHandle();
    createInfo.renderPass = renderPass.getHandle();
    createInfo.subpass = subPass;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = 0;

    CHECK_VK(
        vkCreateGraphicsPipelines(
            device_->getHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline_),
        "Creating graphics pipeline");

    // Destroy shader modules
    for(size_t id = 0; id < maxStageCount; ++id)
    {
        if(moduleInfo_[id].shaderModule != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(device_->getHandle(), moduleInfo_[id].shaderModule, nullptr);
            moduleInfo_[id].shaderModule = VK_NULL_HANDLE;
        }
    }
}
} // namespace vkw