#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "init.hpp"
#include <log4cplus/logger.h>
#include "window.hpp"

namespace compound {
class Device {
private:
    log4cplus::Logger m_logger = log4cplus::Logger::getInstance("compound.device");
    vk::raii::PhysicalDevice m_physicalDevice;
    vk::raii::Device m_device;
    vk::raii::Queue m_graphicsQueue;
    vk::raii::Queue m_presentationQueue;
    int scorePhysicalDevice(const vk::raii::PhysicalDevice&, const Window&) const noexcept;
public:
    Device(const Init&, const Window&);
    void listQueueFamilies(const vk::raii::PhysicalDevice&) const noexcept;
    [[maybe_unused]] uint32_t getGraphicsFamilyQueue(const vk::raii::PhysicalDevice&) const;
    [[maybe_unused]] uint32_t getPresentationFamilyQueue(const vk::raii::PhysicalDevice&, const Window&) const;
    [[maybe_unused]] uint32_t getTransferFamilyQueue(const vk::raii::PhysicalDevice&) const;
};
}