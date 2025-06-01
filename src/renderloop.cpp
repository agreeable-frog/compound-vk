#include "renderloop.hpp"

namespace compound {
Renderloop::Renderloop(const Device& a_device,
                       const std::vector<Framebuffer>& framebuffers)
    : m_device(a_device),
      m_framebuffers(framebuffers),
      m_imageAvailable(0),
      m_inFlight(0) {
    vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    vk::FenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
    m_inFlight = m_device.getDevice().createFence(fenceCreateInfo);
    m_imageAvailable =
        m_device.getDevice().createSemaphore(semaphoreCreateInfo);
    for (size_t i = 0; i < m_framebuffers.size(); i++) {
        m_renderFinished.push_back(
            m_device.getDevice().createSemaphore(semaphoreCreateInfo));
    }
}

void Renderloop::drawFrame(const CommandBuffer& a_commandBuffer,
                           const Swapchain& a_swapchain,
                           const Pipeline& a_pipeline) {
    [[maybe_unused]] vk::Result result1 = m_device.getDevice().waitForFences(
        *m_inFlight, vk::True, std::numeric_limits<uint64_t>::max());
    m_device.getDevice().resetFences(*m_inFlight);
    uint32_t imageIndex;
    auto result = a_swapchain.getSwapchain().acquireNextImage(
        std::numeric_limits<uint64_t>::max(), *m_imageAvailable, nullptr);
    if (result.first == vk::Result::eSuccess) {
        imageIndex = result.second;
    } else {
        throw std::runtime_error("Failed to acquire next image from swapchain");
    }
    a_commandBuffer.getBuffer().reset();
    a_commandBuffer.record(a_swapchain, a_pipeline, m_framebuffers[imageIndex]);

    vk::SubmitInfo submitInfo{};
    std::vector<vk::PipelineStageFlags> waitStages = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.setWaitSemaphores(*m_imageAvailable);
    submitInfo.setWaitDstStageMask(waitStages);
    submitInfo.setCommandBuffers(*a_commandBuffer.getBuffer());
    submitInfo.setSignalSemaphores(*m_renderFinished[imageIndex]);
    m_device.getGraphicsQueue().submit(submitInfo, *m_inFlight);

    vk::PresentInfoKHR presentInfo{};
    presentInfo.setWaitSemaphores(*m_renderFinished[imageIndex]);
    presentInfo.setSwapchains(*a_swapchain.getSwapchain());
    presentInfo.setImageIndices(imageIndex);

    [[maybe_unused]] vk::Result result2 =
        m_device.getPresentQueue().presentKHR(presentInfo);
}
} // namespace compound