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

#include "vkWrappers/wrappers/DebugMessenger.hpp"

static void printDebug(FILE *fp, const char *info, const char *msg, const char *pUserData)
{
    if(pUserData != nullptr)
    {
        fprintf(fp, "%s : %s from %s\n\n", info, msg, pUserData);
    }
    else
    {
        fprintf(fp, "%s : %s -\n\n", info, msg);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    switch(messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
#if(LOG_FILTER == LOG_LEVEL_VERBOSE)
            printDebug(
                stderr,
                "[Verbose] Validation layer",
                pCallbackData->pMessage,
                reinterpret_cast<const char *>(pUserData));
#endif
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
#if(LOG_FILTER <= LOG_LEVEL_INFO)
            printDebug(
                stderr,
                "[Info] Validation layer",
                pCallbackData->pMessage,
                reinterpret_cast<const char *>(pUserData));
#endif
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
#if(LOG_FILTER <= LOG_LEVEL_WARNING)
            printDebug(
                stderr,
                "[Warning] Validation layer",
                pCallbackData->pMessage,
                reinterpret_cast<const char *>(pUserData));
#endif
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            printDebug(
                stderr,
                "[Error] Validation layer",
                pCallbackData->pMessage,
                reinterpret_cast<const char *>(pUserData));
            break;
        default:
            break;
    }

    return VK_FALSE;
}

namespace vkw
{
DebugMessenger::DebugMessenger(Instance &instance)
{
    CHECK_BOOL_THROW(this->init(instance), "Initializing debug messenger");
}

DebugMessenger::DebugMessenger(DebugMessenger &&cp) { *this = std::move(cp); }

DebugMessenger &DebugMessenger::operator=(DebugMessenger &&cp)
{
    this->clear();

    std::swap(instance_, cp.instance_);
    std::swap(messenger_, cp.messenger_);

    return *this;
}

DebugMessenger::~DebugMessenger() { this->clear(); }

bool DebugMessenger::init(Instance &instance)
{
    if(!initialized_)
    {
        instance_ = &instance;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.pNext = nullptr;
        debugCreateInfo.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                          | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT*/
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                                      | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        debugCreateInfo.pUserData = nullptr;

        CHECK_VK_RETURN_FALSE(
            ext::vkCreateDebugUtilsMessengerEXT(
                instance_->getInstance(), &debugCreateInfo, nullptr, &messenger_),
            "Creating debug utils messenger");

        initialized_ = true;
    }
    return true;
}

void DebugMessenger::clear()
{
    if(initialized_)
    {
        ext::vkDestroyDebugUtilsMessengerEXT(instance_->getInstance(), messenger_, nullptr);
        initialized_ = false;
    }
}
} // namespace vkw