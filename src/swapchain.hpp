#pragma once

#include "device.hpp"
#include "window.hpp"
#include <log4cplus/log4cplus.h>

namespace compound {
class Swapchain {
public:
    Swapchain(const Device& device, const Window& window);
private:
    log4cplus::Logger m_logger = log4cplus::Logger::getInstance("compound.swapchain");
    vk::Format m_swapchainFormat;
    vk::Extent2D m_swapchainExtent;
    vk::raii::SwapchainKHR m_swapchain;
    std::vector<vk::raii::ImageView> m_imageViews;
public:
    const vk::Extent2D& getExtent() const noexcept;
    const vk::Format& getFormat() const noexcept;
    const std::vector<vk::raii::ImageView>& getImageViews() const noexcept;
    const vk::raii::SwapchainKHR& getSwapchain() const noexcept;
};
}