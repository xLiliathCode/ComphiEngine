#include "cphipch.h"
#include "MemBuffer.h"

namespace Comphi::Vulkan {
    
    MemBuffer::MemBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        InitMemBuffer(size, usage, properties);
    }

    void MemBuffer::InitMemBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        bufferSize = size;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(*GraphicsHandler::get()->logicalDevice.get(), &bufferInfo, nullptr, &bufferObj) != VK_SUCCESS) {
            COMPHILOG_CORE_ERROR("failed to create buffer!");
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(*GraphicsHandler::get()->logicalDevice.get(), bufferObj, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(*GraphicsHandler::get()->logicalDevice.get(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            COMPHILOG_CORE_ERROR("failed to allocate vertex buffer memory!");
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(*GraphicsHandler::get()->logicalDevice.get(), bufferObj, bufferMemory, 0);
    }

    uint32_t MemBuffer::findMemoryType(VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        COMPHILOG_CORE_ERROR("failed to find suitable memory type!");
        throw std::runtime_error("failed to find suitable memory type!");

    }

    uint32_t MemBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        return findMemoryType(*GraphicsHandler::get()->physicalDevice.get(),typeFilter,properties);
    }

    void MemBuffer::cleanUp()
    {
        COMPHILOG_CORE_INFO("vkDestroy Destroy MemBuffer");
        vkDestroyBuffer(*GraphicsHandler::get()->logicalDevice.get(), bufferObj, nullptr);
        vkFreeMemory(*GraphicsHandler::get()->logicalDevice.get(),bufferMemory, nullptr);
    }

    void MemBuffer::copyBuffer(MemBuffer& srcBuffer, MemBuffer& dstBuffer)
    {
        srcBuffer.copyBufferTo(dstBuffer);
    }

    MemBuffer::CommandBuffer MemBuffer::beginCommandBuffer(CommandQueueOperation op)
    {
        VkCommandPool commandPool = op == MEM_TransferCommand ?
            *GraphicsHandler::get()->transferQueueFamily.commandPool.get() : *GraphicsHandler::get()->graphicsQueueFamily.commandPool.get();

        CommandBuffer commandBuffer = {op};

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1; //how many command buffers to create

        vkAllocateCommandBuffers(*GraphicsHandler::get()->logicalDevice.get(), &allocInfo, &commandBuffer.buffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer.buffer, &beginInfo);

        return commandBuffer;

    }

    void MemBuffer::endCommandBuffer(CommandBuffer& commandBuffer)
    {
        VkQueue commandQueue = commandBuffer.op == MEM_TransferCommand ?
            *GraphicsHandler::get()->transferQueueFamily.queue.get() : *GraphicsHandler::get()->graphicsQueueFamily.queue.get();

        VkCommandPool commandPool = commandBuffer.op == MEM_TransferCommand ?
            *GraphicsHandler::get()->transferQueueFamily.commandPool.get() : *GraphicsHandler::get()->graphicsQueueFamily.commandPool.get();

        vkEndCommandBuffer(commandBuffer.buffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer.buffer;

        vkQueueSubmit(commandQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(commandQueue);

        /*
        We could use a fence and wait with vkWaitForFences,
        or simply wait for the transfer queue to become idle with vkQueueWaitIdle.
        A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete, instead of executing one at a time.
        That may give the driver more opportunities to optimize.
        */

        vkFreeCommandBuffers(*GraphicsHandler::get()->logicalDevice.get(), commandPool, 1, &commandBuffer.buffer);
    }

    void MemBuffer::copyBufferTo(MemBuffer& dstBuffer)
    {
        CommandBuffer commandBuffer = beginCommandBuffer(MEM_TransferCommand);

        VkBufferCopy copyRegion{};
        //copyRegion.srcOffset = 0; // Optional
        //copyRegion.dstOffset = 0; // Optional
        copyRegion.size = bufferSize;
        vkCmdCopyBuffer(commandBuffer.buffer, bufferObj, dstBuffer.bufferObj, 1, &copyRegion);

        endCommandBuffer(commandBuffer);

    }
}