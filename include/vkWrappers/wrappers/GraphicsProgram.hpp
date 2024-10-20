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

#include "vkWrappers/wrappers/Buffer.hpp"
#include "vkWrappers/wrappers/GraphicsPipeline.hpp"
#include "vkWrappers/wrappers/ImageView.hpp"

#include <type_traits>

namespace vkw
{
/// Helper class to create simple graphics programs
template <typename Params>
class GraphicsProgram
{
  public:
    GraphicsProgram() {}
    GraphicsProgram(Device& device, const char* vertexShader, const char* fragmentShader)
    {
        this->init(device, vertexShader, fragmentShader);
    }

    GraphicsProgram(const GraphicsProgram&) = delete;
    GraphicsProgram(GraphicsProgram&& cp) { *this = std::move(cp); }

    GraphicsProgram& operator=(const GraphicsProgram&) = delete;
    GraphicsProgram& operator=(GraphicsProgram&& cp)
    {
        this->clear();
        std::swap(device_, cp.device_);

        std::swap(viewport_, cp.viewport_);
        std::swap(scissor_, cp.scissor_);
        std::swap(cullMode_, cp.cullMode_);

        std::swap(storageBufferBindingPoint_, cp.storageBufferBindingPoint_);
        std::swap(uniformBufferBindingPoint_, cp.uniformBufferBindingPoint_);
        std::swap(storageImageBindingPoint_, cp.storageImageBindingPoint_);
        std::swap(vertexBufferBindingPoint_, cp.vertexBufferBindingPoint_);

        graphicsPipeline_ = std::move(cp.graphicsPipeline_);
        pipelineLayout_ = std::move(cp.pipelineLayout_);
        return *this;
    }

    ~GraphicsProgram() { this->clear(); }

    bool isInitialized() const { return initialized_; }

    void init(Device& device, const char* vertexShader, const char* fragmentShader)
    {
        if(!initialized_)
        {
            device_ = &device;

            graphicsPipeline_.init(device);
            graphicsPipeline_.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertexShader);
            graphicsPipeline_.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader);
            pipelineLayout_.init(device, 1);

            initialized_ = true;
        }
    }

    void clear()
    {
        if(initialized_)
        {
            viewport_ = {};
            scissor_ = {};
            cullMode_ = VK_CULL_MODE_BACK_BIT;

            storageBufferBindingPoint_ = 0;
            uniformBufferBindingPoint_ = 0;
            storageImageBindingPoint_ = 0;
            vertexBufferBindingPoint_ = 0;

            graphicsPipeline_ = {};
            pipelineLayout_ = {};
            descriptorPool_ = {};

            initialized_ = false;
        }
        device_ = nullptr;
    }

    // Dynamic states
    inline GraphicsProgram& setViewport(const float x, const float y, const float w, const float h)
    {
        viewport_ = {x, y, w, h, 0.0f, 0.0f};
        return *this;
    }
    inline GraphicsProgram& setScissor(const int x, const int y, const uint32_t w, const uint32_t h)
    {
        scissor_ = {x, y, w, h};
        return *this;
    }
    inline GraphicsProgram& setCullMode(const VkCullModeFlags mode)
    {
        cullMode_ = mode;
        return *this;
    }

    // Shader stages
    inline GraphicsProgram& addShaderStage(
        const VkShaderStageFlagBits stage, const char* shaderSource)
    {
        graphicsPipeline_.addShaderStage(stage, shaderSource);
        return *this;
    }

    void create(RenderPass& renderpass)
    {
        if constexpr(std::is_empty<Params>::value == false)
        {
            pushConstantOffset_
                = pipelineLayout_.addPushConstantRange(VK_SHADER_STAGE_ALL, sizeof(Params));
        }

        pipelineLayout_.create();
        graphicsPipeline_.createPipeline(renderpass, pipelineLayout_);
        descriptorPool_.init(*device_, pipelineLayout_);
        for(const auto& bindingInfo : storageBufferBindings_)
        {
            descriptorPool_.bindStorageBuffer(0, bindingInfo.bindingPoint, bindingInfo.bufferInfo);
        }
        for(const auto& bindingInfo : uniformBufferBindings_)
        {
            descriptorPool_.bindUniformBuffer(0, bindingInfo.bindingPoint, bindingInfo.bufferInfo);
        }
        for(const auto& bindingInfo : storageImageBindings_)
        {
            descriptorPool_.bindStorageImage(0, bindingInfo.bindingPoint, bindingInfo.imageInfo);
        }
    }

    auto& graphicsPipeline() { return graphicsPipeline_; }
    const auto& graphicsPipeline() const { return graphicsPipeline_; }

    template <typename T>
    inline GraphicsProgram& bindVertexBuffer(const Buffer<T>& buffer)
    {
        graphicsPipeline_.addVertexBinding(vertexBufferBindingPoint_, sizeof(T));
        vertexBufferBindings_.emplace_back(
            BufferBinding{vertexBufferBindingPoint_, buffer.getFullSizeInfo()});
        vertexBufferBindingPoint_++;
        return *this;
    }

    inline GraphicsProgram& vertexAttribute(
        const uint32_t location, const VkFormat format, const uint32_t offset)
    {
        const uint32_t bindingPoint = vertexBufferBindingPoint_ - 1;
        graphicsPipeline_.addVertexAttribute(location, bindingPoint, format, offset);
        return *this;
    }

    template <typename T>
    inline GraphicsProgram& bindStorageBuffers(
        const VkShaderStageFlagBits flags, const Buffer<T>& buffer)
    {
        pipelineLayout_.getDescriptorSetlayoutInfo(0).addStorageBufferBinding(
            flags, storageBufferBindingPoint_, 1);
        storageBufferBindings_.emplace_back(
            BufferBinding{storageBufferBindingPoint_, buffer.getFullSizeInfo()});
        storageBufferBindingPoint_++;
        return *this;
    }
    template <typename T, typename... Args>
    inline GraphicsProgram& bindStorageBuffers(
        const VkShaderStageFlagBits flags, const Buffer<T>& buffer, Args&&... args)
    {
        bindStorageBuffers(flags, buffer);
        return bindStorageBuffers(flags, std::forward<Args>(args)...);
    }

    template <typename T>
    inline GraphicsProgram& bindUniformBuffers(
        const VkShaderStageFlagBits flags, const Buffer<T>& buffer)
    {
        pipelineLayout_.getDescriptorSetlayoutInfo(0).addUniformBufferBinding(
            flags, uniformBufferBindingPoint_, 1);
        uniformBufferBindings_.emplace_back(
            BufferBinding{uniformBufferBindingPoint_, buffer.getFullSizeInfo()});
        uniformBufferBindingPoint_++;
        return *this;
    }
    template <typename T, typename... Args>
    inline GraphicsProgram& bindUniformBuffers(
        const VkShaderStageFlagBits flags, const Buffer<T>& buffer, Args&&... args)
    {
        bindUniformBuffers(flags, buffer);
        return bindUniformBuffers(flags, std::forward<Args>(args)...);
    }

    inline GraphicsProgram& bindStorageImages(
        const VkShaderStageFlagBits flags, const ImageView& image)
    {
        pipelineLayout_.getDescriptorSetlayoutInfo(0).addStorageImageBinding(
            flags, storageImageBindingPoint_, 1);
        storageImageBindings_.emplace_back(ImageBinding{
            storageImageBindingPoint_, {nullptr, image.getHandle(), VK_IMAGE_LAYOUT_GENERAL}});
        storageImageBindingPoint_++;
        return *this;
    }
    template <typename... Args>
    inline GraphicsProgram& bindStorageImages(
        const VkShaderStageFlagBits flags, const ImageView& image, Args&&... args)
    {
        bindStorageImages(flags, image);
        return bindStorageImages(flags, std::forward<Args>(args)...);
    }

    template <typename T>
    inline GraphicsProgram& spec(const T val)
    {
        graphicsPipeline_.addSpec<T>(val);
        return *this;
    }
    template <typename T, typename... Args>
    inline GraphicsProgram& spec(const T val, Args&&... args)
    {
        graphicsPipeline_.addSpec<T>(val);
        return spec(std::forward<Args>(args)...);
    }

  private:
    Device* device_{nullptr};

    bool initialized_{false};

    // Dynamic states
    VkViewport viewport_{};
    VkRect2D scissor_{};
    VkCullModeFlags cullMode_{VK_CULL_MODE_BACK_BIT};

    uint32_t storageBufferBindingPoint_{0};
    uint32_t uniformBufferBindingPoint_{0};
    uint32_t storageImageBindingPoint_{0};
    uint32_t vertexBufferBindingPoint_{0};

    GraphicsPipeline graphicsPipeline_{};
    PipelineLayout pipelineLayout_{};
    DescriptorPool descriptorPool_{};

    struct BufferBinding
    {
        uint32_t bindingPoint;
        VkDescriptorBufferInfo bufferInfo;
    };
    std::vector<BufferBinding> storageBufferBindings_{};
    std::vector<BufferBinding> uniformBufferBindings_{};
    std::vector<BufferBinding> vertexBufferBindings_{};

    struct ImageBinding
    {
        uint32_t bindingPoint;
        VkDescriptorImageInfo imageInfo;
    };
    std::vector<ImageBinding> storageImageBindings_{};

    uint32_t pushConstantOffset_{0};

    friend class CommandBuffer;
};
} // namespace vkw