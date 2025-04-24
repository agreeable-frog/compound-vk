#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "init.hpp"
#include <vulkan/vulkan_raii.hpp>
#include <log4cplus/log4cplus.h>
#include <array>

namespace compound {
class Window {
private:
    log4cplus::Logger m_logger = log4cplus::Logger::getInstance("compound.window");
    int m_width;
    int m_height;
    GLFWwindow* m_handle;
    vk::raii::SurfaceKHR m_surface;
public:
    Window(const Init& init, int width, int height, const std::string& title);
    ~Window();
    GLFWwindow* getHandle() const noexcept;
    const vk::raii::SurfaceKHR& getSurface() const noexcept;
    std::array<int, 2> getFramebufferSize() const noexcept;
};
}