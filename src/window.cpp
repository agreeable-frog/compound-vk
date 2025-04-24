#include "window.hpp"

namespace compound {
Window::Window(const Init& init, int width, int height,
               const std::string& title)
    : m_width(width), m_height(height), m_surface(0) {
    LOG4CPLUS_INFO(m_logger, "Creating window");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if ((m_handle = glfwCreateWindow(width, height, title.c_str(), 0, 0)) ==
        nullptr) {
        LOG4CPLUS_ERROR(m_logger, "Failed to create window");
        throw std::runtime_error("Failed to create window");
    }
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(*init.getVkInstance(), m_handle, 0, &surface) !=
        VK_SUCCESS) {
        LOG4CPLUS_ERROR(m_logger, "Failed to create surface");
        throw std::runtime_error("Failed to create surface");
    }
    m_surface = vk::raii::SurfaceKHR(init.getVkInstance(), surface);
}

Window::~Window() {
    glfwDestroyWindow(m_handle);
}

GLFWwindow* Window::getHandle() const noexcept {
    return m_handle;
}

const vk::raii::SurfaceKHR& Window::getSurface() const noexcept {
    return m_surface;
}

std::array<int, 2> Window::getFramebufferSize() const noexcept {
    int width, height;
    glfwGetFramebufferSize(m_handle, &width, &height);
    return {width, height};
}
} // namespace compound