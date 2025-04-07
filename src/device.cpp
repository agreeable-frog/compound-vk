#include "device.hpp"

#include <log4cplus/loggingmacros.h>
#include <map>
#include <format>
#include <optional>

namespace compound {
Device::Device(const Init& init) : m_physicalDevice(0), m_device(0) {
    LOG4CPLUS_INFO(m_logger, "Creating a new vulkan device");
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
        int score = scorePhysicalDevice(physicalDevice);
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
    vk::raii::PhysicalDevice m_physicalDevice = selectedPhysicalDevice;
    availablePhysicalDevices.clear();
    LOG4CPLUS_INFO(
        m_logger,
        std::format("Selected physical device {}",
                    std::string(m_physicalDevice.getProperties().deviceName)));
    listQueueFamilies(m_physicalDevice);
}

int Device::scorePhysicalDevice(
    const vk::raii::PhysicalDevice& physicalDevice) const noexcept {
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
        getGraphicsFamilyQueue(physicalDevice);
    } catch (std::exception& e) {
        score = 0;
    }
    return score;
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

[[maybe_unused]] uint32_t Device::getGraphicsFamilyQueue(const vk::raii::PhysicalDevice& physicalDevice) const {
    auto queuesProperties = physicalDevice.getQueueFamilyProperties();
    uint32_t selectedQueueFamilyIndex;
    int selectedQueueFamilyScore = 0;
    for (uint32_t i = 0; i < static_cast<uint32_t>(queuesProperties.size()); i++) {
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
} // namespace compound