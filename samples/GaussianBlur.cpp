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

#include "Common.hpp"
#include "ImgUtils.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>

// -----------------------------------------------------------------------------

// clang-format off
static const float gaussianKernel[] =
{
    1.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    2.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    1.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    2.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    4.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    2.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    1.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    2.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    1.0f / 16.0f, 0.0f, 0.0f, 0.0f
};

static const float laplacianKernel[] =
{
     0.0f, 0.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, 0.0f,
     4.0f, 0.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f, 0.0f
};
// clang-format on

// -----------------------------------------------------------------------------

static void updateUBO(
    vkw::Device &device, vkw::Buffer<float> &uboBuf, float *data, const size_t size);

// -----------------------------------------------------------------------------

int main(int, char **)
{
    const std::vector<const char *> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<vkw::InstanceExtension> instanceExts = {vkw::DebugUtilsExt};
    vkw::Instance instance(instanceLayers, instanceExts);

    const std::vector<VkPhysicalDeviceType> compatibleDeviceTypes
        = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};
    vkw::Device device(instance, {}, {}, compatibleDeviceTypes);

    auto deviceQueues = device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_COMPUTE_BIT);
    if(deviceQueues.empty())
    {
        throw std::runtime_error("No available device queues");
    }
    vkw::Queue computeQueue = deviceQueues[0];

    int width;
    int height;
    uint8_t *imgData = utils::imgLoad("samples/data/img.png", &width, &height, 4);
    fprintf(stdout, "Image loaded : w = %d, h = %d\n", width, height);

    const uint32_t res = width * height;

    vkw::Memory stagingMem(device, hostStagingFlags.memoryFlags);
    auto &stagingBuf = stagingMem.createBuffer<float>(hostStagingFlags.usage, 4 * res);
    stagingMem.allocate();

    vkw::Memory uboMem(device, uniformDeviceFlags.memoryFlags);
    auto &uboBuf = uboMem.createBuffer<float>(uniformDeviceFlags.usage, 9 * 4);
    uboMem.allocate();

    vkw::Memory imgMem(device, imgDeviceFlags.memoryFlags);
    auto &inImage = imgMem.createImage(
        VK_IMAGE_TYPE_2D,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        imgDeviceFlags.usage);
    auto &outImage = imgMem.createImage(
        VK_IMAGE_TYPE_2D,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        imgDeviceFlags.usage);
    imgMem.allocate();

    updateUBO(device, uboBuf, const_cast<float *>(gaussianKernel), 9 * 4);

    struct PushConstants
    {
        uint32_t width;
        uint32_t height;
    } pushConstants;
    pushConstants.width = static_cast<uint32_t>(width);
    pushConstants.height = static_cast<uint32_t>(height);

    vkw::PipelineLayout pipelineLayout(device, 1);
    pipelineLayout.getDescriptorSetlayoutInfo(0)
        .addStorageImageBinding(VK_SHADER_STAGE_COMPUTE_BIT, 0, 1)
        .addStorageImageBinding(VK_SHADER_STAGE_COMPUTE_BIT, 1, 1)
        .addUniformBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 2, 1);

    uint32_t compPushConstantsOffset
        = pipelineLayout.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(PushConstants));

    pipelineLayout.create();

    vkw::ImageView inImageView(
        device,
        inImage,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
    vkw::ImageView outImageView(
        device,
        outImage,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    vkw::DescriptorPool descriptorPool(device, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT);
    descriptorPool
        .bindStorageImage(0, 0, {VK_NULL_HANDLE, inImageView.getHandle(), VK_IMAGE_LAYOUT_GENERAL})
        .bindStorageImage(0, 1, {VK_NULL_HANDLE, outImageView.getHandle(), VK_IMAGE_LAYOUT_GENERAL})
        .bindUniformBuffer(0, 2, {uboBuf.getHandle(), 0, VK_WHOLE_SIZE});

    vkw::ComputePipeline pipeline(device, "output/spv/img_gaussian_comp.spv");
    pipeline.addSpec<uint32_t>(16).addSpec<uint32_t>(16);
    pipeline.createPipeline(pipelineLayout);

    vkw::CommandPool cmdPool(device, computeQueue);
    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
        .imageMemoryBarriers(
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            vkw::createImageMemoryBarrier(
                inImage,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
            vkw::createImageMemoryBarrier(
                outImage,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_GENERAL))
        .copyBufferToImage(
            stagingBuf,
            inImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            {0,
             0,
             0,
             {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
             {0, 0, 0},
             {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1}})
        .imageMemoryBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            vkw::createImageMemoryBarrier(
                inImage,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_GENERAL))
        .bindComputePipeline(pipeline)
        .bindComputeDescriptorSets(pipelineLayout, descriptorPool)
        .pushConstants(
            pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, compPushConstantsOffset, pushConstants)
        .dispatch(vkw::utils::divUp(width, 16), vkw::utils::divUp(height, 16), 1)
        .imageMemoryBarrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            vkw::createImageMemoryBarrier(
                outImage,
                VK_ACCESS_SHADER_WRITE_BIT,
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL))
        .copyImageToBuffer(
            outImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            stagingBuf,
            {0,
             0,
             0,
             {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
             {0, 0, 0},
             {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1}})
        .end();

    // Execute
    std::vector<float> inData(width * height * 4);
    std::vector<float> outData(width * height * 4);
    for(int i = 0; i < width * height * 4; i++)
    {
        inData[i] = (float) imgData[i] / 255.0f;
    }

    stagingMem.copyFromHost<float>(inData.data(), 0, inData.size());
    computeQueue.submit(cmdBuffer).waitIdle();
    stagingMem.copyFromDevice<float>(outData.data(), 0, outData.size());

    for(int i = 0; i < width * height * 4; i++)
    {
        imgData[i] = (unsigned char) (outData[i] * 255.0f);
    }

    utils::imgStorePNG("main/data/output.png", imgData, width, height, 4);
    utils::imgFree(imgData);
    return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------

static void updateUBO(
    vkw::Device &device, vkw::Buffer<float> &uboBuf, float *data, const size_t size)
{
    vkw::Memory stagingMem(device, hostStagingFlags.memoryFlags);
    auto &stagingBuf = stagingMem.createBuffer<float>(hostStagingFlags.usage, size);
    stagingMem.allocate();
    stagingMem.copyFromHost<float>(data, stagingBuf.getMemOffset(), size);

    auto deviceQueues = device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_TRANSFER_BIT);
    if(deviceQueues.empty())
    {
        throw std::runtime_error("No available device queues");
    }
    vkw::Queue transferQueue = deviceQueues[0];

    vkw::CommandPool cmdPool(device, transferQueue);
    std::array<VkBufferCopy, 1> c0 = {{0, 0, size * sizeof(float)}};

    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
        .copyBuffer(stagingBuf, uboBuf, c0)
        .end();

    transferQueue.submit(cmdBuffer).waitIdle();
}
