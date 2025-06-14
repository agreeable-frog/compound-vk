#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include "commandstructs.hpp"
#include "framebuffer.hpp"
#include <vector>

namespace compound {
class Renderloop {
public:
    Renderloop(const Device&, const std::vector<Framebuffer>&);
    void drawFrame(const Device&, const std::vector<Framebuffer>&, const CommandBuffer&, const Swapchain&, const Pipeline&);
private:
    vk::raii::Semaphore m_imageAvailable;
    std::vector<vk::raii::Semaphore> m_renderFinished;
    vk::raii::Fence m_inFlight;
};
}