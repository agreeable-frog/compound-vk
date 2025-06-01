#include "swapchain.hpp"

#include <optional>

namespace compound {
Swapchain::Swapchain(const Device& device, const Window& window)
    : m_swapchain(0) {
    LOG4CPLUS_INFO(m_logger, "Creating swapchain");
    auto availableFormats =
        device.getPhysicalDevice().getSurfaceFormatsKHR(*window.getSurface());

    std::optional<vk::SurfaceFormatKHR> selectedFormat;
    for (const auto& format : availableFormats) {
        if ((format.format == vk::Format::eB8G8R8A8Srgb) &&
            (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)) {
            selectedFormat = format;
        }
    }
    if (!selectedFormat.has_value()) {
        LOG4CPLUS_ERROR(m_logger, "Could not find wanted surface format");
        throw std::runtime_error("Could not find wanted surface format");
    }

    vk::PresentModeKHR selectedPresentMode = vk::PresentModeKHR::eFifo;
    for (const auto& presentMode :
         device.getPhysicalDevice().getSurfacePresentModesKHR(
             *window.getSurface())) {
        if (presentMode == vk::PresentModeKHR::eMailbox) {
            selectedPresentMode = presentMode;
        }
    }

    auto surfaceCapabilities =
        device.getPhysicalDevice().getSurfaceCapabilitiesKHR(
            *window.getSurface());
    vk::Extent2D extent = surfaceCapabilities.currentExtent;
    if (surfaceCapabilities.currentExtent.width ==
        std::numeric_limits<uint32_t>::max()) {
        auto framebufferSize = window.getFramebufferSize();
        extent =
            vk::Extent2D{std::clamp(static_cast<uint32_t>(framebufferSize[0]),
                                    surfaceCapabilities.minImageExtent.width,
                                    surfaceCapabilities.maxImageExtent.width),
                         std::clamp(static_cast<uint32_t>(framebufferSize[1]),
                                    surfaceCapabilities.minImageExtent.height,
                                    surfaceCapabilities.maxImageExtent.height)};
    }

    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;

    vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.setSurface(*window.getSurface());
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = selectedFormat.value().format;
    swapchainCreateInfo.imageColorSpace = selectedFormat.value().colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    std::array<uint32_t, 2> indices = {
        device.getGraphicsFamilyQueueIndex(),
        device.getPresentationFamilyQueueIndex()};
    if (indices[0] != indices[1]) {
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchainCreateInfo.setQueueFamilyIndices(indices);
    } else {
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchainCreateInfo.presentMode = selectedPresentMode;
    swapchainCreateInfo.clipped = vk::True;
    swapchainCreateInfo.setOldSwapchain(nullptr);

    m_swapchain = device.getDevice().createSwapchainKHR(swapchainCreateInfo);
    m_swapchainExtent = extent;
    m_swapchainFormat = selectedFormat.value().format;

    auto m_images = m_swapchain.getImages();
    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.format = selectedFormat.value().format;
    imageViewCreateInfo.setComponents(vk::ComponentMapping());
    imageViewCreateInfo.setSubresourceRange(
        vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    for (size_t i = 0; i < m_images.size(); i++) {
        imageViewCreateInfo.setImage(m_images[i]);
        m_imageViews.push_back(
            device.getDevice().createImageView(imageViewCreateInfo));
    }
}

const vk::Extent2D& Swapchain::getExtent() const noexcept {
    return m_swapchainExtent;
}

const vk::Format& Swapchain::getFormat() const noexcept {
    return m_swapchainFormat;
}

const std::vector<vk::raii::ImageView>& Swapchain::getImageViews()
    const noexcept {
    return m_imageViews;
}

const vk::raii::SwapchainKHR& Swapchain::getSwapchain() const noexcept {
   return m_swapchain; 
}
} // namespace compound