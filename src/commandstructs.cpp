#include "commandstructs.hpp"

namespace compound {
CommandPool::CommandPool(const Device& device, uint32_t queueFamilyIndex)
    : m_commandPool(0) {
    vk::CommandPoolCreateInfo createInfo{};
    createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    createInfo.setQueueFamilyIndex(queueFamilyIndex);
    m_commandPool = device.getDevice().createCommandPool(createInfo);
}

const vk::raii::CommandPool& CommandPool::getCommandPool() const noexcept {
    return m_commandPool;
}

CommandBuffer::CommandBuffer(const Device& device,
                             const CommandPool& commandPool)
    : m_buffer(0) {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.setCommandPool(commandPool.getCommandPool());
    allocInfo.setCommandBufferCount(1);
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    m_buffer =
        std::move(device.getDevice().allocateCommandBuffers(allocInfo)[0]);
}

void CommandBuffer::record(const Swapchain& swapchain, const Pipeline& pipeline,
                           const Framebuffer& framebuffer) const {
    vk::CommandBufferBeginInfo beginInfo{};
    m_buffer.begin(beginInfo);
    vk::RenderPassBeginInfo renderpassBeginInfo{};
    renderpassBeginInfo.setRenderPass(pipeline.getRenderpass());
    renderpassBeginInfo.setFramebuffer(framebuffer.getFramebuffer());
    renderpassBeginInfo.setRenderArea(vk::Rect2D({0, 0}, swapchain.getExtent()));
    auto clearValue = vk::ClearValue({0.0f, 0.0f, 0.0f, 1.0f});
    renderpassBeginInfo.setClearValues(clearValue);
    m_buffer.beginRenderPass(renderpassBeginInfo, vk::SubpassContents::eInline);

    m_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.getPipeline());

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = swapchain.getExtent().width;
    viewport.height = swapchain.getExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    m_buffer.setViewport(0, viewport);

    vk::Rect2D scissor{};
    scissor.setOffset({0, 0});
    scissor.setExtent(swapchain.getExtent());
    m_buffer.setScissor(0, scissor);

    m_buffer.draw(3, 1, 0, 0);
    m_buffer.endRenderPass();
    m_buffer.end();
}

const vk::raii::CommandBuffer& CommandBuffer::getBuffer() const noexcept {
    return m_buffer;
}
} // namespace compound