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

#include <cstdint>
#include <cstdio>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>

#define CHECK_VK(f, msg)                                                                           \
    {                                                                                              \
        VkResult err = f;                                                                          \
        if(err != VK_SUCCESS)                                                                      \
        {                                                                                          \
            fprintf(stderr, "%s : %s\n", msg, string_VkResult(err));                               \
            exit(1);                                                                               \
        }                                                                                          \
    }

namespace vkw
{
inline uint32_t divUp(const uint32_t n, const uint32_t val) { return (n + val - 1) / val; }

namespace utils
{
    VkShaderModule createShaderModule(const VkDevice device, const std::vector<char> &src);

    std::vector<char> readShader(const std::string &filename);
} // namespace utils
} // namespace vkw
