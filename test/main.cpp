#include <iostream>
#include "init.hpp"
#include <log4cplus/configurator.h>
#include "device.hpp"
#include "window.hpp"
#include "swapchain.hpp"
#include "pipeline.hpp"
#include "framebuffer.hpp"
#include "commandstructs.hpp"
#include "renderloop.hpp"

int main() {
    log4cplus::BasicConfigurator::doConfigure();
    compound::Init::setAppName("compound-test");
    const compound::Init& init = compound::Init::get();
    compound::Window window(init, 800, 450, "test");
    compound::Device device(init, window.getSurface());
    compound::Swapchain swapchain(device, window);
    compound::Pipeline pipeline(
        device, std::string(TEST_DIR) + "shaders/basic.vert.spv",
        std::string(TEST_DIR) + "shaders/basic.frag.spv",
        swapchain.getFormat());
    auto framebuffers = compound::Framebuffer::create(
        device, swapchain.getImageViews(), swapchain.getExtent(),
        pipeline.getRenderpass());
    compound::CommandPool graphicsCommandPool(
        device, device.getGraphicsFamilyQueueIndex());
    compound::CommandBuffer graphicsCommandBuffer(device, graphicsCommandPool);
    compound::Renderloop renderloop(device, framebuffers);
    while (!glfwWindowShouldClose(window.getHandle())) {
        glfwPollEvents();
        renderloop.drawFrame(device, framebuffers, graphicsCommandBuffer,
                             swapchain, pipeline);
    }
    device.getDevice().waitIdle();
    return 0;
}