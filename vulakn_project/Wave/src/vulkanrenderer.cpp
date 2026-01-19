#include "vulkanrenderer.h"

static inline VkDeviceSize u_aligned(VkDeviceSize v, VkDeviceSize byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}

const int MAX_INSTANCES = 16380;

VulkanRenderer::VulkanRenderer(VulkanWindow *window) 
    : m_window(window)
{
}

void VulkanRenderer::preInitResources()
{
    const QList<int> sampleCounts = m_window->supportedSampleCounts();

    if (sampleCounts.contains(4))
    {
        m_window->setSampleCount(4);
    }
}

void VulkanRenderer::initResources()
{
    QVulkanInstance *inst = m_window->vulkanInstance();
    VkDevice dev = m_window->device();
    VkResult err = VK_SUCCESS;

    m_devFuncs = inst->deviceFunctions(dev);
    vertShader = createShaderModule(":/VkShader/vert.spv");
    fragShader = createShaderModule(":/VkShader/frag.spv");

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    err = m_devFuncs->vkCreatePipelineCache(dev, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache);
    if (err != VK_SUCCESS) qFatal("创建管线缓存失败");

    VkVertexInputBindingDescription vertexBindingDesc[] = {
        { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX }
    };

    VkVertexInputAttributeDescription vertexAttrDesc[] = {
        { 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = vertexBindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = 1;
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDesc;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    err = m_devFuncs->vkCreatePipelineLayout(dev, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    if (err != VK_SUCCESS) qFatal("创建管线布局失败");

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            vertShader,
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            fragShader,
            "main",
            nullptr
        }
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr;
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.lineWidth = 2.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = m_window->sampleCountFlagBits();

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.logicOpEnable = VK_FALSE;

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState);
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = sizeof(shaderStages) / sizeof(shaderStages[0]);
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_window->defaultRenderPass();

    err = m_devFuncs->vkCreateGraphicsPipelines(dev, m_pipelineCache, 1, &pipelineInfo, nullptr, &m_pipeline);
    if (err != VK_SUCCESS) qFatal("创建图形管线失败");
    else qDebug() << "图形管线创建成功";

    this->ensureBuffers();
}

void VulkanRenderer::ensureBuffers()
{
    VkDevice dev = m_window->device();
    VkResult err = VK_SUCCESS;

    VkBufferCreateInfo bufInfo{};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = sizeof(Vertex) * MAX_INSTANCES;
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    err = m_devFuncs->vkCreateBuffer(dev, &bufInfo, nullptr, &m_vertexBuffer);
    if (err != VK_SUCCESS) qFatal("创建顶点缓冲区失败");

    VkMemoryRequirements memReqs;
    m_devFuncs->vkGetBufferMemoryRequirements(dev, m_vertexBuffer, &memReqs);

    m_vertexBufferSize = memReqs.size;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = m_window->hostVisibleMemoryIndex();

    err = m_devFuncs->vkAllocateMemory(dev, &allocInfo, nullptr, &m_vertexBufferMemory);
    if (err != VK_SUCCESS) qFatal("分配顶点缓冲区内存失败");

    err = m_devFuncs->vkBindBufferMemory(dev, m_vertexBuffer, m_vertexBufferMemory, 0);
    if (err != VK_SUCCESS) qFatal("绑定顶点缓冲区到内存失败");
}

void VulkanRenderer::createVertices()
{
    m_verticesData.clear();
    m_instanceCount = 0;
    m_vertexCount = 0;

    auto& lines = m_window->m_wavesWidget->getLines();
    if (!lines.empty())
        m_vertexCount = lines[0].size();

    for (const auto& points : lines)
    {
        if (points.empty()) continue;

        QPointF p1 = m_window->m_wavesWidget->moved(points[0], false);

        for (size_t idx = 0; idx < points.size(); ++idx)
        {
            const Point& p = points[idx];
            bool isLast = (idx == points.size() - 1);
            p1 = m_window->m_wavesWidget->moved(p, !isLast);

            Vertex v;
            v.inPos.x = p1.x();
            v.inPos.y = p1.y();
            m_verticesData.push_back(v);
        }
        m_instanceCount++;
    }

    void *data;
    VkResult err = m_devFuncs->vkMapMemory(m_window->device(), m_vertexBufferMemory, 0, m_vertexBufferSize, 0, &data);
    if (err != VK_SUCCESS) qFatal("映射顶点缓冲区内存失败");

    memcpy(data, m_verticesData.data(), m_verticesData.size() * sizeof(Vertex));
    m_devFuncs->vkUnmapMemory(m_window->device(), m_vertexBufferMemory);
}

void VulkanRenderer::initSwapChainResources()
{
    const QSize sz = m_window->swapChainImageSize();
    myPushConstants.projection = glm::ortho(0.0f, (float)sz.width(), 0.0f, (float)sz.height(), -1.0f, 1.0f);

    this->createVertices();
    m_window->m_wavesWidget->setLines(sz.width(), sz.height());
}

void VulkanRenderer::releaseSwapChainResources()
{
}

void VulkanRenderer::releaseResources()
{
    qDebug("释放 Vulkan 资源...");
    VkDevice dev = m_window->device();

    if (m_pipeline != VK_NULL_HANDLE) {
        m_devFuncs->vkDestroyPipeline(dev, m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }

    if (m_pipelineLayout != VK_NULL_HANDLE) {
        m_devFuncs->vkDestroyPipelineLayout(dev, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }

    if (m_pipelineCache != VK_NULL_HANDLE) {
        m_devFuncs->vkDestroyPipelineCache(dev, m_pipelineCache, nullptr);
        m_pipelineCache = VK_NULL_HANDLE;
    }

    if (vertShader != VK_NULL_HANDLE) {
        m_devFuncs->vkDestroyShaderModule(dev, vertShader, nullptr);
        vertShader = VK_NULL_HANDLE;
    }

    if (fragShader != VK_NULL_HANDLE) {
        m_devFuncs->vkDestroyShaderModule(dev, fragShader, nullptr);
        fragShader = VK_NULL_HANDLE;
    }

    if (m_vertexBuffer != VK_NULL_HANDLE) {
        m_devFuncs->vkDestroyBuffer(dev, m_vertexBuffer, nullptr);
        m_vertexBuffer = VK_NULL_HANDLE;
    }

    if (m_vertexBufferMemory != VK_NULL_HANDLE) {
        m_devFuncs->vkFreeMemory(dev, m_vertexBufferMemory, nullptr);
        m_vertexBufferMemory = VK_NULL_HANDLE;
    }
}

void VulkanRenderer::startNextFrame()
{
    VkCommandBuffer cmdBuf = m_window->currentCommandBuffer();
    const QSize sz = m_window->swapChainImageSize();
    uint32_t width = uint32_t(sz.width());
    uint32_t height = uint32_t(sz.height());

    VkClearColorValue clearColor = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
    VkClearDepthStencilValue clearDS = { 1.0f, 0 };

    VkClearValue clearValues[3] = {};
    clearValues[0].color = clearColor;
    clearValues[1].depthStencil = clearDS;
    clearValues[2].color = clearColor;

    const uint32_t clearValueCount = m_window->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3 : 2;

    VkRenderPassBeginInfo rpBeginInfo{};
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = m_window->defaultRenderPass();
    rpBeginInfo.framebuffer = m_window->currentFramebuffer();
    rpBeginInfo.renderArea.offset = {0, 0};
    rpBeginInfo.renderArea.extent = {width, height};
    rpBeginInfo.clearValueCount = clearValueCount;
    rpBeginInfo.pClearValues = clearValues;

    m_devFuncs->vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { 0.0f, 0.0f, float(width), float(height), 0.0f, 1.0f };
    m_devFuncs->vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

    VkRect2D scissor = { {0, 0}, {width, height} };
    m_devFuncs->vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

    this->buildDrawCommand();

    m_devFuncs->vkCmdEndRenderPass(cmdBuf);

    m_window->frameReady();
    m_window->requestUpdate();
}

void VulkanRenderer::buildDrawCommand()
{
    VkCommandBuffer cd = m_window->currentCommandBuffer();

    m_devFuncs->vkCmdBindPipeline(cd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    m_window->m_wavesWidget->tick_();
    this->createVertices();

    VkDeviceSize vbOffset[] = {0};
    m_devFuncs->vkCmdBindVertexBuffers(cd, 0, 1, &m_vertexBuffer, vbOffset);

    m_devFuncs->vkCmdPushConstants(cd, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 
                                   0, sizeof(myPushConstants), &myPushConstants);

    for (int i = 0; i < m_instanceCount; ++i)
    {
        uint32_t firstVertex = i * m_vertexCount;
        if (firstVertex + m_vertexCount <= m_verticesData.size())
        {
            m_devFuncs->vkCmdDraw(cd, m_vertexCount, 1, firstVertex, 0);
        }
    }
}

VkShaderModule VulkanRenderer::createShaderModule(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("警告：无法读取着色器 %s", qPrintable(fileName));
        return VK_NULL_HANDLE;
    }

    QByteArray blob = file.readAll();
    file.close();

    VkShaderModuleCreateInfo shaderInfo{};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = blob.size();
    shaderInfo.pCode = reinterpret_cast<const uint32_t*>(blob.constData());

    VkShaderModule shaderModule;
    VkResult err = m_devFuncs->vkCreateShaderModule(m_window->device(), &shaderInfo, nullptr, &shaderModule);
    if (err != VK_SUCCESS)
    {
        qWarning("警告：无法创建着色器模块: %d", err);
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}