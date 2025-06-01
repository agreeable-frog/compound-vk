#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <log4cplus/log4cplus.h>
#include "device.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include "framebuffer.hpp"

namespace compound {
class CommandPool {
public:
    CommandPool(const Device& device, uint32_t queueFamilyIndex);
    const vk::raii::CommandPool& getCommandPool() const noexcept;
private:
    vk::raii::CommandPool m_commandPool;
};

class CommandBuffer {
public:
    CommandBuffer(const Device& device, const CommandPool& commandPool);
    void record(const Swapchain& swapchain, const Pipeline& pipeline, const Framebuffer& framebuffer) const;
    const vk::raii::CommandBuffer& getBuffer() const noexcept;
private:
    vk::raii::CommandBuffer m_buffer;
};
}