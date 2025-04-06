#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "init.hpp"
#include <log4cplus/logger.h>

namespace compound {
class Device {
private:
    log4cplus::Logger m_logger = log4cplus::Logger::getInstance("compound.device");
    vk::raii::PhysicalDevice m_physicalDevice;
    vk::raii::Device m_device;
    int scorePhysicalDevice(const vk::raii::PhysicalDevice&) const noexcept;
public:
    Device(const Init&);
    void listQueueFamilies(const vk::raii::PhysicalDevice&) const noexcept;
};
}