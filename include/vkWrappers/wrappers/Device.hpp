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

#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/Queue.hpp"
#include "vkWrappers/wrappers/extensions/DeviceExtensions.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdlib>
#include <vulkan/vulkan.h>

namespace vkw
{
class Device
{
  public:
    Device() {}
    Device(
        Instance& instance,
        const std::vector<const char*>& extensions,
        const VkPhysicalDeviceFeatures& requiredFeatures,
        const std::vector<VkPhysicalDeviceType>& requiredTypes
        = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU},
        void* pCreateExt = nullptr);

    Device(const Device&) = delete;
    Device(Device&& cp);

    Device& operator=(const Device&) = delete;
    Device& operator=(Device&& cp);

    ~Device();

    bool init(
        Instance& instance,
        const std::vector<const char*>& extensions,
        const VkPhysicalDeviceFeatures& requiredFeatures,
        const std::vector<VkPhysicalDeviceType>& requiredTypes
        = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU},
        void* pCreateExt = nullptr);

    void clear();

    bool isInitialized() const { return initialized_; }

    std::vector<Queue> getQueues(const QueueUsageFlags requiredFlags) const;

    VkDevice& getHandle() { return device_; }
    const VkDevice& getHandle() const { return device_; }

    VkPhysicalDeviceFeatures getFeatures() const { return deviceFeatures_; }
    VkPhysicalDeviceProperties getProperties() const { return deviceProperties_; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice_; }

    bool hasMeshShaderSupport() const { return meshShadersSupported_; }

    void waitIdle() const { vkDeviceWaitIdle(device_); }

  private:
    Instance* instance_{nullptr};

    VkPhysicalDeviceFeatures deviceFeatures_{};
    VkPhysicalDeviceProperties deviceProperties_{};
    VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};

    static constexpr uint32_t maxQueueCount = 32;
    std::vector<float> queuePriorities_;

    bool presentSupported_{false};
    std::vector<Queue> deviceQueues_{};
    VkDevice device_{VK_NULL_HANDLE};

    bool meshShadersSupported_{false};

    bool initialized_{false};

    bool getPhysicalDevice(
        const VkPhysicalDeviceFeatures& requiredFeatures,
        const std::vector<VkPhysicalDeviceType>& requiredTypes);

    bool checkFeaturesCompatibility(
        const VkPhysicalDeviceFeatures& requiredFeatures,
        const VkPhysicalDeviceFeatures& deviceFeatures);

    std::vector<VkExtensionProperties> getDeviceExtensionProperties(
        const VkPhysicalDevice physicalDevice);

    void allocateQueues();

    std::vector<VkDeviceQueueCreateInfo> getAvailableQueuesInfo();
};
} // namespace vkw
