/*
 * Copyright (C) 2022 Adrien ARNAUD
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

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "utils.hpp"
#include "Instance.hpp"
#include "QueueFamilies.hpp"
#include "Device.hpp"
#include "Buffer.hpp"
#include "DescriptorSetLayout.hpp"
#include "PipelineLayout.hpp"

namespace vk
{
class DescriptorPool
{
public:
  DescriptorPool(
      Device &device, PipelineLayout &pipelineLayout, VkShaderStageFlags flags);

  ~DescriptorPool();

  VkDescriptorPool &getHandle() { return descriptorPool_; }

  std::vector<VkDescriptorSet> &getDescriptors() { return descriptorSets_; }

  DescriptorPool &bindStorageBuffer(
      uint32_t setId, uint32_t bindingId, VkDescriptorBufferInfo bufferInfo,
      uint32_t offset = 0, uint32_t count = 1);

  DescriptorPool &bindStorageImage(
      uint32_t setId, uint32_t bindingId, VkDescriptorImageInfo imageInfo,
      uint32_t offset = 0, uint32_t count = 1);

  DescriptorPool &bindUniformBuffer(
      uint32_t setId, uint32_t bindingId, VkDescriptorBufferInfo bufferInfo,
      uint32_t offset = 0, uint32_t count = 1);

private:
  Device &device_;
  std::vector<VkDescriptorSet> descriptorSets_;
  VkDescriptorPool descriptorPool_;

  void allocateDescriptorSets(PipelineLayout &pipelineLayout);
};
} // namespace vk
