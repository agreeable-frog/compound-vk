#include "init.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <format>

static inline void logSeverity(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    const std::string& msg) noexcept {
    static log4cplus::Logger logger =
        log4cplus::Logger::getInstance("compound.vkdebug");
    switch (messageSeverity) {
        case VkDebugUtilsMessageSeverityFlagBitsEXT::
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG4CPLUS_ERROR(logger, msg);
            break;
        case VkDebugUtilsMessageSeverityFlagBitsEXT::
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG4CPLUS_WARN(logger, msg);
            break;
        case VkDebugUtilsMessageSeverityFlagBitsEXT::
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG4CPLUS_INFO(logger, msg);
            break;
        case VkDebugUtilsMessageSeverityFlagBitsEXT::
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LOG4CPLUS_DEBUG(logger, msg);
            break;
        default:
            LOG4CPLUS_DEBUG(logger, "not supposed to be here");
    }
}

static inline VkBool32 debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    [[maybe_unused]] void* pUserData) noexcept {
    std::string msg = "";
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        msg += "GENERAL ";
    }
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        msg += "VALIDATION ";
    }
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        msg += "PERFORMANCE ";
    }
    msg += pCallbackData->pMessage;
    logSeverity(messageSeverity, msg);
    return VK_FALSE;
}

namespace compound {
void Init::setAppName(const std::string& appName) noexcept {
    m_appName = appName;
}

Init& Init::get() {
    static Init instance;
    return instance;
}

Init::Init() : m_context(), m_instance(0), m_debugMessenger(0) {
    LOG4CPLUS_INFO(m_logger, "Instancing init");
    glfwInit();
    glfwSetErrorCallback([](int error, const char* msg) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("compound.init"),
                        std::to_string(error) + " : " + msg);
    });
    vk::ApplicationInfo applicationInfo{};
    applicationInfo.pApplicationName = m_appName.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "compound";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    vk::InstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    for (const auto& validationLayer : m_validationLayers) {
        if (!isLayerAvailable(validationLayer)) {
            throw std::runtime_error("Validation layer is not available.");
        }
    }
    instanceCreateInfo.setPEnabledLayerNames(m_validationLayers);
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions,
                                        glfwExtensions + glfwExtensionCount);
    if (m_validationLayers.size() > 0) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    for (const auto& extension : extensions) {
        LOG4CPLUS_DEBUG(m_logger, "Checking for extension support for " +
                                      std::string(extension));
        if (!isExtensionAvailable(extension)) {
            throw std::runtime_error("Extension is not available.");
        }
    }
    m_extensions = extensions;
    instanceCreateInfo.setPEnabledExtensionNames(extensions);

    LOG4CPLUS_INFO(m_logger, "Creating vulkan instance");
    try {
        m_instance = vk::raii::Instance(m_context, instanceCreateInfo);
    } catch (std::exception& e) {
        LOG4CPLUS_ERROR(m_logger, e.what());
        throw e;
    }
    if (m_validationLayers.size() > 0) {
        LOG4CPLUS_DEBUG(m_logger, "Setting up debug messenger");
        setupDebugMessenger();
    }
}

void Init::logInstanceProperties() const noexcept {
    auto version = m_context.enumerateInstanceVersion();
    auto extensions = m_context.enumerateInstanceExtensionProperties();
    auto layers = m_context.enumerateInstanceLayerProperties();
    LOG4CPLUS_INFO(m_logger, std::format("Instance version : {}",
                                         std::to_string(version)));
    LOG4CPLUS_INFO(m_logger, "Available extensions :");
    for (const auto& extension : extensions) {
        LOG4CPLUS_INFO(
            m_logger,
            std::format("\t{} {}", std::string(extension.extensionName),
                        std::to_string(extension.specVersion)));
    }
    LOG4CPLUS_INFO(m_logger, "Available layers :");
    for (const auto& layer : layers) {
        LOG4CPLUS_INFO(
            m_logger, std::format("\t{} {} {} {}", std::string(layer.layerName),
                                  std::to_string(layer.specVersion),
                                  std::to_string(layer.implementationVersion),
                                  std::string(layer.description)));
    }
}

bool Init::isExtensionAvailable(
    const std::string& extensionName) const noexcept {
    auto extensions = m_context.enumerateInstanceExtensionProperties();
    for (const auto& extension : extensions) {
        if (std::string(extension.extensionName) == extensionName) {
            LOG4CPLUS_DEBUG(m_logger,
                            std::format("Extension {} is available",
                                        std::string(extension.extensionName)));
            return true;
        }
    }
    LOG4CPLUS_WARN(m_logger, std::format("Extension {} is not available",
                                         std::string(extensionName)));
    return false;
}

bool Init::isLayerAvailable(const std::string& layerName) const noexcept {
    auto layers = m_context.enumerateInstanceLayerProperties();
    for (const auto& layer : layers) {
        if (std::string(layer.layerName) == layerName) {
            LOG4CPLUS_DEBUG(m_logger,
                            std::format("Layer {} is available",
                                        std::string(layer.layerName)));
            return true;
        }
    }
    LOG4CPLUS_WARN(m_logger, std::format("Layer {} is not available",
                                         std::string(layerName)));
    return false;
}

void Init::setupDebugMessenger() {
    vk::DebugUtilsMessengerCreateInfoEXT info{};
    {
        using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
        info.messageSeverity = eError | eWarning | eInfo | eVerbose;
    }
    {
        using enum vk::DebugUtilsMessageTypeFlagBitsEXT;
        info.messageType = eGeneral | eValidation | ePerformance;
    }
    info.setPfnUserCallback(debugCallback);
    info.pUserData = nullptr;
    m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(info);
}

Init::~Init() {
    glfwTerminate();
}

const vk::raii::Instance& Init::getVkInstance() const noexcept {
    return m_instance;
}

const std::vector<const char*>& Init::getExtensions() const noexcept {
    return m_extensions;
}

const std::vector<const char*>& Init::getLayers() const noexcept {
    return m_validationLayers;
}
} // namespace compound
