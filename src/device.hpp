#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "init.hpp"
#include <log4cplus/logger.h>
#include "window.hpp"
#include <vector>

namespace compound {
class Device {
private:
    log4cplus::Logger m_logger = log4cplus::Logger::getInstance("compound.device");
    vk::raii::PhysicalDevice m_physicalDevice;
    vk::raii::Device m_device;
    uint32_t m_graphicsQueueFamilyIndex = 0;
    uint32_t m_graphicsQueueCount = 0;
    vk::raii::Queue m_graphicsQueue;
    uint32_t m_presentationQueueFamilyIndex = 0;
    uint32_t m_presentationQueueCount = 0;
    vk::raii::Queue m_presentationQueue;
    int scorePhysicalDevice(const vk::raii::PhysicalDevice&, const Window&) const noexcept;
    void selectPhysicalDevice(const Init& init, const Window& window);
    void createDevice(const Init& init, const Window& window);
    std::vector<const char*> m_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice&) const noexcept;
    bool checkDeviceSwapchainSupport(const vk::raii::PhysicalDevice&, const vk::raii::SurfaceKHR&) const noexcept;
public:
    Device(const Init&, const Window&);
    void listQueueFamilies(const vk::raii::PhysicalDevice&) const noexcept;
    [[maybe_unused]] uint32_t queryGraphicsFamilyQueueIndex(const vk::raii::PhysicalDevice&) const;
    [[maybe_unused]] uint32_t queryPresentationFamilyQueueIndex(const vk::raii::PhysicalDevice&, const Window&) const;
    [[maybe_unused]] uint32_t queryTransferFamilyQueueIndex(const vk::raii::PhysicalDevice&) const;
    const vk::raii::PhysicalDevice& getPhysicalDevice() const noexcept;
    const vk::raii::Device& getDevice() const noexcept;
    uint32_t getGraphicsFamilyQueueIndex() const noexcept;
    const vk::raii::Queue& getGraphicsQueue() const noexcept;
    uint32_t getPresentationFamilyQueueIndex() const noexcept;
    const vk::raii::Queue& getPresentQueue() const noexcept;
};
}