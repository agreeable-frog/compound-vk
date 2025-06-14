#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "device.hpp"
#include "swapchain.hpp"
#include "pipeline.hpp"
#include <log4cplus/log4cplus.h>
#include <vector>

namespace compound {
class Framebuffer {
public:
    static std::vector<Framebuffer> create(
        const Device& device, const std::vector<vk::raii::ImageView>& imageViews,
        const vk::Extent2D& extent, const vk::raii::RenderPass& renderpass);
    const vk::raii::Framebuffer& getFramebuffer() const noexcept;

private:
    log4cplus::Logger m_logger =
        log4cplus::Logger::getInstance("compound.framebuffer");
    vk::raii::Framebuffer m_framebuffer;
    Framebuffer(vk::raii::Framebuffer&& framebuffer);
};
} // namespace compound