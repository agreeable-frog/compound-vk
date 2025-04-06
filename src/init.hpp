#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include <log4cplus/logger.h>
#include <vector>

namespace compound {
class Init {
private:
    log4cplus::Logger m_logger = log4cplus::Logger::getInstance("compound.init");
    inline static std::string m_appName;
    vk::raii::Context m_context;
    vk::raii::Instance m_instance;
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger;
#ifndef NDEBUG
    inline static std::vector<const char*> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
#else
    inline static std::vector<const char*> m_validationLayers {""};
#endif
    Init();
    void logInstanceProperties() const noexcept;
    bool isExtensionAvailable(const std::string&) const noexcept;
    bool isLayerAvailable(const std::string&) const noexcept;
    void setupDebugMessenger();
public:
    static void setAppName(const std::string&) noexcept;
    static Init& get();
    const vk::raii::Instance& getVkInstance() const noexcept;
    ~Init();
};
} // namespace compound