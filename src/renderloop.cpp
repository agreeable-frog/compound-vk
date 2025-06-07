#include "renderloop.hpp"

namespace compound {
Renderloop::Renderloop(const Device& a_device,
                       const std::vector<Framebuffer>& a_framebuffers)
    : m_imageAvailable(0), m_inFlight(0) {
    vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    vk::FenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
    m_inFlight = a_device.getDevice().createFence(fenceCreateInfo);
    m_imageAvailable =
        a_device.getDevice().createSemaphore(semaphoreCreateInfo);
    for (size_t i = 0; i < a_framebuffers.size(); i++) {
        m_renderFinished.push_back(
            a_device.getDevice().createSemaphore(semaphoreCreateInfo));
    }
}

void Renderloop::drawFrame(const Device& a_device,
                           const std::vector<Framebuffer>& a_framebuffers,
                           const CommandBuffer& a_commandBuffer,
                           const Swapchain& a_swapchain,
                           const Pipeline& a_pipeline) {
    [[maybe_unused]] vk::Result result1 = a_device.getDevice().waitForFences(
        *m_inFlight, vk::True, std::numeric_limits<uint64_t>::max());
    a_device.getDevice().resetFences(*m_inFlight);
    uint32_t imageIndex;
    auto result = a_swapchain.getSwapchain().acquireNextImage(
        std::numeric_limits<uint64_t>::max(), *m_imageAvailable, nullptr);
    if (result.first == vk::Result::eSuccess) {
        imageIndex = result.second;
    } else {
        throw std::runtime_error("Failed to acquire next image from swapchain");
    }
    a_commandBuffer.getBuffer().reset();
    a_commandBuffer.record(a_swapchain, a_pipeline, a_framebuffers[imageIndex]);

    vk::SubmitInfo submitInfo{};
    std::vector<vk::PipelineStageFlags> waitStages = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.setWaitSemaphores(*m_imageAvailable);
    submitInfo.setWaitDstStageMask(waitStages);
    submitInfo.setCommandBuffers(*a_commandBuffer.getBuffer());
    submitInfo.setSignalSemaphores(*m_renderFinished[imageIndex]);
    a_device.getGraphicsQueue().submit(submitInfo, *m_inFlight);

    vk::PresentInfoKHR presentInfo{};
    presentInfo.setWaitSemaphores(*m_renderFinished[imageIndex]);
    presentInfo.setSwapchains(*a_swapchain.getSwapchain());
    presentInfo.setImageIndices(imageIndex);

    [[maybe_unused]] vk::Result result2 =
        a_device.getPresentQueue().presentKHR(presentInfo);
}
} // namespace compound