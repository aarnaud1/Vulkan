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

#pragma once

#include "vkWrappers/wrappers/Buffer.hpp"
#include "vkWrappers/wrappers/Common.hpp"
#include "vkWrappers/wrappers/DescriptorPool.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/PipelineLayout.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <string>
#include <vector>

namespace vkw
{
class ComputePipeline
{
  public:
    ComputePipeline() {}
    ComputePipeline(Device& device, const std::string& shaderSource);

    ComputePipeline(const ComputePipeline&) = delete;
    ComputePipeline(ComputePipeline&& cp);

    ComputePipeline& operator=(const ComputePipeline&) = delete;
    ComputePipeline& operator=(ComputePipeline&& cp);

    ~ComputePipeline();

    bool init(Device& device, const std::string& shaderSource);

    void clear();

    bool isInitialized() const { return initialized_; }

    void createPipeline(PipelineLayout& pipelineLayout);

    template <typename T>
    ComputePipeline& addSpec(const T value)
    {
        static constexpr size_t size = sizeof(T);
        const char* data = (char*) &value;

        for(size_t i = 0; i < size; i++)
        {
            specData_.push_back(data[i]);
        }
        specSizes_.push_back(size);

        return *this;
    }
    template <typename T, typename... Args>
    ComputePipeline& addSpec(const T value, Args&&... args)
    {
        addSpec<T>(value);
        return addSpec(std::forward<Args>(args)...);
    }

    VkPipeline& getHandle() { return pipeline_; }
    const VkPipeline& getHandle() const { return pipeline_; }

  private:
    Device* device_{nullptr};
    std::string shaderSource_{};
    VkPipeline pipeline_{VK_NULL_HANDLE};

    bool initialized_{false};

    std::vector<char> specData_{};
    std::vector<size_t> specSizes_{};
};
} // namespace vkw