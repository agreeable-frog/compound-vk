#include "device.hpp"

#include <log4cplus/loggingmacros.h>
#include <map>
#include <format>
#include <optional>
#include <set>

namespace compound {
Device::Device(const Init& init, const Window& window)
    : m_physicalDevice(0),
      m_device(0),
      m_graphicsQueue(0),
      m_presentationQueue(0) {
    LOG4CPLUS_INFO(m_logger, "Creating a new vulkan device");
    selectPhysicalDevice(init, window);
    LOG4CPLUS_INFO(
        m_logger,
        std::format("Selected physical device {}",
                    std::string(m_physicalDevice.getProperties().deviceName)));
    listQueueFamilies(m_physicalDevice);

    LOG4CPLUS_INFO(m_logger, "Creating queues and device");
    createDevice(init, window);

}

int Device::scorePhysicalDevice(const vk::raii::PhysicalDevice& physicalDevice,
                                const Window& window) const noexcept {
    auto properties = physicalDevice.getProperties();
    int score = 0;
    if (properties.apiVersion < VK_API_VERSION_1_3) {
        return 0;
    }
    if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1000;
    }
    if (properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
        score += 500;
    }
    try {
        queryGraphicsFamilyQueueIndex(physicalDevice);
        queryPresentationFamilyQueueIndex(physicalDevice, window);
    } catch (std::exception& e) {
        score = 0;
    }
    if (!checkDeviceExtensionSupport(physicalDevice)) score = 0;
    if (!checkDeviceSwapchainSupport(physicalDevice, window.getSurface()))
        score = 0;
    return score;
}

void Device::selectPhysicalDevice(const Init& init, const Window& window) {
    std::vector<vk::raii::PhysicalDevice> availablePhysicalDevices =
        init.getVkInstance().enumeratePhysicalDevices();
    if (availablePhysicalDevices.size() == 0) {
        LOG4CPLUS_ERROR(m_logger,
                        "No vulkan-compatible physical device found on system");
        throw std::runtime_error(
            "No vulkan-compatible physical device found on system");
    }
    LOG4CPLUS_INFO(m_logger, "Selecting a physical device");
    int topScore = 0;
    vk::raii::PhysicalDevice& selectedPhysicalDevice =
        availablePhysicalDevices[0];
    for (auto& physicalDevice : availablePhysicalDevices) {
        int score = scorePhysicalDevice(physicalDevice, window);
        if (score < topScore) {
            continue;
        }
        topScore = score;
        selectedPhysicalDevice = physicalDevice;
    }
    if (topScore == 0) {
        LOG4CPLUS_ERROR(m_logger, "No physical device met the requirements");
        throw std::runtime_error("No physical device met the requirements");
    }
    m_physicalDevice = selectedPhysicalDevice;
}

void Device::createDevice(const Init& init, const Window& window) {
    vk::DeviceQueueCreateInfo graphicsQueueCreateInfo{};
    m_graphicsQueueFamilyIndex =
        queryGraphicsFamilyQueueIndex(m_physicalDevice);
    graphicsQueueCreateInfo.setQueueFamilyIndex(m_graphicsQueueFamilyIndex);
    graphicsQueueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    graphicsQueueCreateInfo.setQueuePriorities(queuePriority);

    vk::DeviceQueueCreateInfo presentationQueueCreateInfo{};
    m_presentationQueueFamilyIndex =
        queryPresentationFamilyQueueIndex(m_physicalDevice, window);
    presentationQueueCreateInfo.setQueueFamilyIndex(
        m_presentationQueueFamilyIndex);
    presentationQueueCreateInfo.queueCount = 1;
    presentationQueueCreateInfo.setQueuePriorities(queuePriority);

    vk::PhysicalDeviceFeatures physicalDeviceFeatures;
    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setQueueCreateInfos(graphicsQueueCreateInfo);
    deviceCreateInfo.setPEnabledFeatures(&physicalDeviceFeatures);
    deviceCreateInfo.setPEnabledExtensionNames(m_extensions);
    deviceCreateInfo.setPEnabledLayerNames(init.getLayers());
    m_device = m_physicalDevice.createDevice(deviceCreateInfo);
    m_graphicsQueue = m_device.getQueue(m_graphicsQueueFamilyIndex, 0);
    m_presentationQueue = m_device.getQueue(m_presentationQueueFamilyIndex, 0);
}

bool Device::checkDeviceExtensionSupport(
    const vk::raii::PhysicalDevice& physicalDevice) const noexcept {
    LOG4CPLUS_DEBUG(m_logger, "Checking device supports needed extensions");
    std::set<std::string> requiredExtensions(m_extensions.begin(),
                                             m_extensions.end());
    for (const auto& availableExtension :
         physicalDevice.enumerateDeviceExtensionProperties()) {
        requiredExtensions.erase(availableExtension.extensionName);
    }
    return requiredExtensions.empty();
}

bool Device::checkDeviceSwapchainSupport(
    const vk::raii::PhysicalDevice& physicalDevice,
    const vk::raii::SurfaceKHR& surface) const noexcept {
    LOG4CPLUS_DEBUG(m_logger, "Checking device supports swapchain");
    auto physicalDeviceSurfaceFormats =
        physicalDevice.getSurfaceFormatsKHR(*surface);
    auto physicalDeviceSurfacePresentModes =
        physicalDevice.getSurfacePresentModesKHR(*surface);
    return physicalDeviceSurfaceFormats.size() > 0 &&
           physicalDeviceSurfacePresentModes.size();
}

void Device::listQueueFamilies(
    const vk::raii::PhysicalDevice& physicalDevice) const noexcept {
    auto queuesProperties = physicalDevice.getQueueFamilyProperties();
    for (size_t i = 0; i < queuesProperties.size(); i++) {
        const auto& queueProperties = queuesProperties[i];
        auto flags = queueProperties.queueFlags;
        using enum vk::QueueFlagBits;
        bool graphics = (flags & eGraphics) == eGraphics;
        bool transfer = (flags & eTransfer) == eTransfer;
        bool compute = (flags & eCompute) == eCompute;
        LOG4CPLUS_INFO(
            m_logger,
            std::format(
                "Queue family {} : count {} graphics {} transfer {} compute {}",
                std::to_string(i), std::to_string(queueProperties.queueCount),
                graphics ? "yes" : "no", transfer ? "yes" : "no",
                compute ? "yes" : "no"));
    }
}

[[maybe_unused]] uint32_t Device::queryGraphicsFamilyQueueIndex(
    const vk::raii::PhysicalDevice& physicalDevice) const {
    LOG4CPLUS_DEBUG(m_logger, "Getting a queue family for graphics");
    auto queuesProperties = physicalDevice.getQueueFamilyProperties();
    uint32_t selectedQueueFamilyIndex;
    int selectedQueueFamilyScore = 0;
    for (uint32_t i = 0; i < static_cast<uint32_t>(queuesProperties.size());
         i++) {
        int score = 0;
        auto& queueProperties = queuesProperties[i];
        auto flags = queueProperties.queueFlags;
        using enum vk::QueueFlagBits;
        bool graphics = (flags & eGraphics) == eGraphics;
        bool transfer = (flags & eTransfer) == eTransfer;
        bool compute = (flags & eCompute) == eCompute;
        if (graphics) score += 500;
        if (!transfer) score += 500;
        if (!compute) score += 500;
        if (score > selectedQueueFamilyScore) {
            selectedQueueFamilyIndex = i;
            selectedQueueFamilyScore = score;
        }
    }
    if (selectedQueueFamilyScore == 0) {
        LOG4CPLUS_ERROR(m_logger, "No graphics-able family queue was found");
        throw std::runtime_error("No graphics-able family queue was found");
    }
    return selectedQueueFamilyIndex;
}

[[maybe_unused]] uint32_t Device::queryPresentationFamilyQueueIndex(
    const vk::raii::PhysicalDevice& physicalDevice,
    const Window& window) const {
    LOG4CPLUS_DEBUG(m_logger, "Getting a queue family for presentation");
    auto queuesProperties = physicalDevice.getQueueFamilyProperties();
    uint32_t selectedQueueFamilyIndex;
    int selectedQueueFamilyScore = 0;
    for (uint32_t i = 0; i < static_cast<uint32_t>(queuesProperties.size());
         i++) {
        int score = 0;
        auto& queueProperties = queuesProperties[i];
        auto flags = queueProperties.queueFlags;
        using enum vk::QueueFlagBits;
        bool graphics = (flags & eGraphics) == eGraphics;
        bool transfer = (flags & eTransfer) == eTransfer;
        bool compute = (flags & eCompute) == eCompute;
        if (graphics) score += 500;
        if (!transfer) score += 500;
        if (!compute) score += 500;
        if (score > selectedQueueFamilyScore) {
            selectedQueueFamilyIndex = i;
            selectedQueueFamilyScore = score;
        }
        if (physicalDevice.getSurfaceSupportKHR(i, *window.getSurface()) ==
            VK_FALSE) {
            score = 0;
        }
    }
    if (selectedQueueFamilyScore == 0) {
        LOG4CPLUS_ERROR(m_logger, "No graphics-able family queue was found");
        throw std::runtime_error("No graphics-able family queue was found");
    }
    return selectedQueueFamilyIndex;
}

const vk::raii::Device& Device::getDevice() const noexcept {
    return m_device;
}

const vk::raii::PhysicalDevice& Device::getPhysicalDevice() const noexcept {
    return m_physicalDevice;
}

uint32_t Device::getGraphicsFamilyQueueIndex() const noexcept {
    return m_graphicsQueueFamilyIndex;
}

uint32_t Device::getPresentationFamilyQueueIndex() const noexcept {
    return m_presentationQueueFamilyIndex;
}

const vk::raii::Queue& Device::getGraphicsQueue() const noexcept {
    return m_graphicsQueue;
}

const vk::raii::Queue& Device::getPresentQueue() const noexcept {
    return m_presentationQueue;
}
} // namespace compound