#include "pipeline.hpp"

#include "utils.hpp"

namespace compound {
Pipeline::Pipeline(const Device& device, const std::string& vertShaderPath,
                   const std::string& fragShaderPath, vk::Format format)
    : m_vertShaderModule(0),
      m_fragShaderModule(0),
      m_pipelineLayout(0),
      m_renderpass(0),
      m_pipeline(0) {
    LOG4CPLUS_INFO(m_logger, "Creating pipeline");
    {
        auto vertShader = utils::readFile(vertShaderPath);
        vk::ShaderModuleCreateInfo shaderModuleCreateInfo{};
        const uint32_t* arr =
            reinterpret_cast<const uint32_t*>(vertShader.data());
        shaderModuleCreateInfo.setCode(*arr);
        shaderModuleCreateInfo.setCodeSize(vertShader.size());
        m_vertShaderModule =
            device.getDevice().createShaderModule(shaderModuleCreateInfo);
    }
    {
        auto fragShader = utils::readFile(fragShaderPath);
        vk::ShaderModuleCreateInfo shaderModuleCreateInfo{};
        const uint32_t* arr =
            reinterpret_cast<const uint32_t*>(fragShader.data());
        shaderModuleCreateInfo.setCode(*arr);
        shaderModuleCreateInfo.setCodeSize(fragShader.size());
        m_fragShaderModule =
            device.getDevice().createShaderModule(shaderModuleCreateInfo);
    }

    vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
    vertShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eVertex);
    vertShaderStageCreateInfo.setModule(*m_vertShaderModule);
    vertShaderStageCreateInfo.setPName("main");

    vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
    fragShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eFragment);
    fragShaderStageCreateInfo.setModule(*m_fragShaderModule);
    fragShaderStageCreateInfo.setPName("main");

    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};

    vk::PipelineVertexInputDivisorStateCreateInfoKHR
        vertexInputDivisorStateCreateInfo{};

    vk::PipelineInputAssemblyStateCreateInfo
        vertexInputAssemblyStateCreateInfo{};
    vertexInputAssemblyStateCreateInfo.setTopology(
        vk::PrimitiveTopology::eTriangleList);
    vertexInputAssemblyStateCreateInfo.setPrimitiveRestartEnable(vk::False);

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    auto dynamicStates = {vk::DynamicState::eViewport,
                          vk::DynamicState::eScissor};
    dynamicStateCreateInfo.setDynamicStates(dynamicStates);

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.setScissorCount(1);
    viewportStateCreateInfo.setViewportCount(1);

    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.setDepthClampEnable(vk::False);
    rasterizationStateCreateInfo.setRasterizerDiscardEnable(vk::False);
    rasterizationStateCreateInfo.setPolygonMode(vk::PolygonMode::eFill);
    rasterizationStateCreateInfo.setLineWidth(1.0f);
    rasterizationStateCreateInfo.setCullMode(vk::CullModeFlagBits::eBack);
    rasterizationStateCreateInfo.setFrontFace(vk::FrontFace::eClockwise);
    rasterizationStateCreateInfo.setDepthBiasEnable(vk::False);
    rasterizationStateCreateInfo.setDepthBiasConstantFactor(0.0f);
    rasterizationStateCreateInfo.setDepthBiasClamp(0.0f);
    rasterizationStateCreateInfo.setDepthBiasSlopeFactor(0.0f);

    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.setSampleShadingEnable(vk::False);
    multisampleStateCreateInfo.setRasterizationSamples(
        vk::SampleCountFlagBits::e1);
    multisampleStateCreateInfo.setMinSampleShading(1.0f);
    multisampleStateCreateInfo.setPSampleMask(nullptr);
    multisampleStateCreateInfo.setAlphaToCoverageEnable(vk::False);
    multisampleStateCreateInfo.setAlphaToOneEnable(vk::False);

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    {
        using enum vk::ColorComponentFlagBits;
        colorBlendAttachment.setColorWriteMask(eR | eG | eB | eA);
    }
    colorBlendAttachment.setBlendEnable(vk::False);

    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
    colorBlendStateCreateInfo.setLogicOpEnable(vk::False);
    colorBlendStateCreateInfo.setLogicOp(vk::LogicOp::eCopy);
    colorBlendStateCreateInfo.setAttachments(colorBlendAttachment);

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    // pipelineLayoutCreateInfo.setSetLayouts(0);
    // pipelineLayoutCreateInfo.setPushConstantRanges(0);

    m_pipelineLayout =
        device.getDevice().createPipelineLayout(pipelineLayoutCreateInfo);

    vk::AttachmentDescription colorAttachment{};
    colorAttachment.setFormat(format);
    colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
    colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
    colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference attachmentReference{};
    attachmentReference.setAttachment(0);
    attachmentReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpassDescription{};
    subpassDescription.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpassDescription.setColorAttachments(attachmentReference);

    vk::SubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = vk::SubpassExternal;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    subpassDependency.srcAccessMask = vk::AccessFlagBits::eNone;
    subpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    subpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    vk::RenderPassCreateInfo renderpassCreateInfo{};
    renderpassCreateInfo.setAttachments(colorAttachment);
    renderpassCreateInfo.setSubpasses(subpassDescription);
    renderpassCreateInfo.setDependencies(subpassDependency);
    m_renderpass = device.getDevice().createRenderPass(renderpassCreateInfo);

    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    auto stages = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};
    graphicsPipelineCreateInfo.setStages(stages);
    graphicsPipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo);
    graphicsPipelineCreateInfo.setPInputAssemblyState(&vertexInputAssemblyStateCreateInfo);
    graphicsPipelineCreateInfo.setPViewportState(&viewportStateCreateInfo);
    graphicsPipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo);
    graphicsPipelineCreateInfo.setPMultisampleState(&multisampleStateCreateInfo);
    graphicsPipelineCreateInfo.setPDepthStencilState(nullptr);
    graphicsPipelineCreateInfo.setPColorBlendState(&colorBlendStateCreateInfo);
    graphicsPipelineCreateInfo.setPDynamicState(&dynamicStateCreateInfo);
    graphicsPipelineCreateInfo.setLayout(*m_pipelineLayout);
    graphicsPipelineCreateInfo.setRenderPass(*m_renderpass);
    graphicsPipelineCreateInfo.setSubpass(0);
    graphicsPipelineCreateInfo.setBasePipelineHandle(nullptr);
    graphicsPipelineCreateInfo.setBasePipelineIndex(-1);
    m_pipeline = device.getDevice().createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
}

const vk::raii::RenderPass& Pipeline::getRenderpass() const noexcept {
    return m_renderpass;
}

const vk::raii::Pipeline& Pipeline::getPipeline() const noexcept {
    return m_pipeline;
}
} // namespace compound