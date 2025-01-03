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

#include "vkWrappers/wrappers/Device.hpp"

#include "vkWrappers/wrappers/utils.hpp"

#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>

namespace vkw
{
Device::Device(
    Instance& instance,
    const std::vector<const char*>& extensions,
    const VkPhysicalDeviceFeatures& requiredFeatures,
    const std::vector<VkPhysicalDeviceType>& requiredTypes,
    void* pCreateExt)
{
    VKW_CHECK_BOOL_THROW(
        this->init(instance, extensions, requiredFeatures, requiredTypes, pCreateExt),
        "Creating device");
}

Device::Device(Device&& cp) { *this = std::move(cp); }

Device& Device::operator=(Device&& cp)
{
    this->clear();

    std::swap(instance_, cp.instance_);

    std::swap(deviceFeatures_, cp.deviceFeatures_);
    std::swap(deviceProperties_, cp.deviceProperties_);
    std::swap(physicalDevice_, cp.physicalDevice_);

    std::swap(device_, cp.device_);

    std::swap(initialized_, cp.initialized_);

    return *this;
}

Device::~Device() { this->clear(); }

bool Device::init(
    Instance& instance,
    const std::vector<const char*>& extensions,
    const VkPhysicalDeviceFeatures& requiredFeatures,
    const std::vector<VkPhysicalDeviceType>& requiredTypes,
    void* pCreateNext)
{
    if(!initialized_)
    {
        instance_ = &instance;

        queuePriorities_.resize(maxQueueCount);
        std::fill(queuePriorities_.begin(), queuePriorities_.end(), 1.0f);

        VKW_INIT_CHECK_BOOL(getPhysicalDevice(requiredFeatures, requiredTypes));

        vkGetPhysicalDeviceFeatures(physicalDevice_, &deviceFeatures_);

        // Create logical device
        auto queueCreateInfoList = getAvailableQueuesInfo();

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfoList.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfoList.data();
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = extensions.data();
        deviceCreateInfo.pEnabledFeatures = &requiredFeatures;
        deviceCreateInfo.pNext = pCreateNext;
        VKW_INIT_CHECK_VK(vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &device_));

        // Load required extensions
        for(auto extName : extensions)
        {
            VKW_INIT_CHECK_BOOL(loadDeviceExtension(device_, extName));
        }

        // Get queue handles
        allocateQueues();

        initialized_ = true;
    }

    utils::Log::Info("wkw", "Logical device created");
    return true;
}

void Device::clear()
{
    if(physicalDevice_ != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device_, nullptr);
    }

    instance_ = nullptr;

    deviceFeatures_ = {};
    deviceProperties_ = {};
    physicalDevice_ = VK_NULL_HANDLE;

    deviceQueues_.clear();
    device_ = VK_NULL_HANDLE;

    initialized_ = false;
}

std::vector<Queue> Device::getQueues(const QueueUsageFlags requiredFlags) const
{
    std::vector<Queue> ret = {};

    for(const auto& queue : deviceQueues_)
    {
        if((queue.flags() & requiredFlags) == requiredFlags)
        {
            ret.emplace_back(Queue(queue));
        }
    }

    return ret;
}

bool Device::getPhysicalDevice(
    const VkPhysicalDeviceFeatures& requiredFeatures,
    const std::vector<VkPhysicalDeviceType>& requiredTypes)
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance_->getHandle(), &physicalDeviceCount, nullptr);

    if(physicalDeviceCount == 0)
    {
        return false;
    }

    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(physicalDeviceCount);
    vkEnumeratePhysicalDevices(
        instance_->getHandle(), &physicalDeviceCount, physicalDevices.data());

    for(const auto deviceType : requiredTypes)
    {
        for(const auto physicalDevice : physicalDevices)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            VkPhysicalDeviceFeatures features{};
            vkGetPhysicalDeviceFeatures(physicalDevice, &features);

            if(properties.deviceType == deviceType
               && checkFeaturesCompatibility(requiredFeatures, features))
            {
                utils::Log::Info("vkw", "Device found : %s", properties.deviceName);
                utils::Log::Info(
                    "vkw", "Device type : %s", string_VkPhysicalDeviceType(deviceType));

                deviceFeatures_ = features;
                deviceProperties_ = properties;
                physicalDevice_ = physicalDevice;

                return true;
            }
        }
    }

    return false;
}

bool Device::checkFeaturesCompatibility(
    const VkPhysicalDeviceFeatures& requiredFeatures,
    const VkPhysicalDeviceFeatures& deviceFeatures)
{
    static constexpr uint32_t featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);

    const auto* reqFeaturesPtr = reinterpret_cast<const VkBool32*>(&requiredFeatures);
    const auto* featuresPtr = reinterpret_cast<const VkBool32*>(&deviceFeatures);

    for(uint32_t i = 0; i < featureCount; ++i)
    {
        if(reqFeaturesPtr[i] == VK_TRUE && featuresPtr[i] == VK_FALSE)
        {
            return false;
        }
    }

    return true;
}

std::vector<VkExtensionProperties> Device::getDeviceExtensionProperties(
    const VkPhysicalDevice physicalDevice)
{
    uint32_t nExtensions;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &nExtensions, nullptr);
    std::vector<VkExtensionProperties> ret(nExtensions);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &nExtensions, ret.data());
    return ret;
}

std::vector<VkDeviceQueueCreateInfo> Device::getAvailableQueuesInfo()
{
    deviceQueues_.clear();

    std::vector<VkDeviceQueueCreateInfo> ret{};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> properties;
    properties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, properties.data());

    for(size_t i = 0; i < properties.size(); ++i)
    {
        const auto& props = properties[i];
        VkBool32 presentSupport = 0;
        if(instance_->getSurface() != VK_NULL_HANDLE)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(
                physicalDevice_,
                static_cast<uint32_t>(i),
                instance_->getSurface(),
                &presentSupport);
        }

        const uint32_t queueCount = props.queueCount;
        const VkQueueFlags queueFlags = props.queueFlags;

        QueueUsageFlags flags = 0;
        if(queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            flags |= uint32_t(QueueUsageBits::VKW_QUEUE_GRAPHICS_BIT);
        }
        if(queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            flags |= uint32_t(QueueUsageBits::VKW_QUEUE_COMPUTE_BIT);
        }
        if(queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            flags |= uint32_t(QueueUsageBits::VKW_QUEUE_TRANSFER_BIT);
        }
        if(presentSupport)
        {
            flags |= uint32_t(QueueUsageBits::VKW_QUEUE_PRESENT_BIT);
        }

        for(uint32_t ii = 0; ii < std::min(queueCount, maxQueueCount); ++ii)
        {
            Queue queue{};
            queue.flags_ = flags;
            queue.queueFamilyIndex_ = static_cast<uint32_t>(i);
            queue.queueIndex_ = ii;
            deviceQueues_.emplace_back(std::move(queue));
        }

        VkDeviceQueueCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.pQueuePriorities = queuePriorities_.data();
        createInfo.queueFamilyIndex = static_cast<uint32_t>(i);
        createInfo.queueCount = std::min(queueCount, maxQueueCount);
        ret.emplace_back(createInfo);
    }

    return ret;
}

void Device::allocateQueues()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> properties;
    properties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, properties.data());

    size_t index = 0;
    for(uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        const auto& props = properties[i];
        const uint32_t queueCount = props.queueCount;
        for(uint32_t ii = 0; ii < std::min(queueCount, maxQueueCount); ++ii)
        {
            vkGetDeviceQueue(device_, i, ii, &(deviceQueues_[index].queue_));
            index++;
        }
    }
}
} // namespace vkw
