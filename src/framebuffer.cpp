#include "framebuffer.hpp"
#include <iostream>

namespace compound {
Framebuffer::Framebuffer(vk::raii::Framebuffer&& framebuffer)
    : m_framebuffer(std::move(framebuffer)) {
    LOG4CPLUS_DEBUG(m_logger, "Creating framebuffer");
}
std::vector<Framebuffer> Framebuffer::create(const Device& device,
                                             const Swapchain& swapchain,
                                             const Pipeline& pipeline) {
    LOG4CPLUS_INFO(log4cplus::Logger::getInstance("compound.framebuffer"),
                   "Creating framebuffers from swapchain's imageviews and "
                   "pipeline's renderpass");
    std::vector<Framebuffer> framebuffers;
    const auto& imageViews = swapchain.getImageViews();
    vk::FramebufferCreateInfo createInfo{};
    createInfo.setRenderPass(*pipeline.getRenderpass());
    const auto& extent = swapchain.getExtent();
    createInfo.width = extent.width;
    createInfo.height = extent.height;
    createInfo.setLayers(1);
    for (const auto& imageView : imageViews) {
        createInfo.setAttachments(*imageView);
        framebuffers.push_back(
            Framebuffer(device.getDevice().createFramebuffer(createInfo)));
    }
    return framebuffers;
}

const vk::raii::Framebuffer& Framebuffer::getFramebuffer() const noexcept {
    return m_framebuffer;
}
} // namespace compound