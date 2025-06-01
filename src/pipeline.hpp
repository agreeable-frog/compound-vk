#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "device.hpp"
#include "swapchain.hpp"
#include <log4cplus/log4cplus.h>

namespace compound {
class Pipeline {
public:
    Pipeline(const Device& device, const std::string& vertShaderPath,
             const std::string& fragShaderPath, vk::Format format);
    const vk::raii::RenderPass& getRenderpass() const noexcept;
    const vk::raii::Pipeline& getPipeline() const noexcept;

private:
    log4cplus::Logger m_logger =
        log4cplus::Logger::getInstance("compound.pipeline");
    vk::raii::ShaderModule m_vertShaderModule;
    vk::raii::ShaderModule m_fragShaderModule;
    vk::raii::PipelineLayout m_pipelineLayout;
    vk::raii::RenderPass m_renderpass;
    vk::raii::Pipeline m_pipeline;
};
} // namespace compound