#include "VulkanPipeline.h"


vk::PipelineShaderStageCreateInfo pipeline_shader_stage_create_info(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule) {
    return vk::PipelineShaderStageCreateInfo({}, stage, shaderModule, "main");
}


vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info(VertexInputDescription& vertexDescription) {
    vk::PipelineVertexInputStateCreateInfo info = {};

    //connect the pipeline builder vertex input info to the one we get from Vertex
    info.setVertexAttributeDescriptions(vertexDescription.attributes);
    info.setVertexBindingDescriptions(vertexDescription.bindings);

    return info;
}


vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info(vk::PrimitiveTopology topology) {
    vk::PipelineInputAssemblyStateCreateInfo info = {};

    info.topology = topology;
    info.primitiveRestartEnable = VK_FALSE; //we are not going to use primitive restart on the entire tutorial so leave it on false
    
    return info;
}


vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info(vk::PolygonMode polygonMode)
{
    vk::PipelineRasterizationStateCreateInfo info = vk::PipelineRasterizationStateCreateInfo(
        {}, 
        VK_FALSE, 
        VK_FALSE, //discards all primitives before the rasterization stage if enabled which we don't want
        polygonMode, 
        vk::CullModeFlagBits::eNone, 
        vk::FrontFace::eClockwise
    );

    return info;
}


vk::PipelineMultisampleStateCreateInfo multisampling_state_create_info()
{
    vk::PipelineMultisampleStateCreateInfo info = {};

    info.sampleShadingEnable = VK_FALSE;
    info.rasterizationSamples = vk::SampleCountFlagBits::e1; //multisampling defaulted to no multisampling (1 sample per pixel)
    info.minSampleShading = 1.0f;
    info.pSampleMask = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;

    return info;
}


vk::PipelineColorBlendAttachmentState color_blend_attachment_state() {
    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;
    return colorBlendAttachment;
}


vk::PipelineLayoutCreateInfo pipeline_layout_create_info() {
    vk::PipelineLayoutCreateInfo info{};
    return info;
}

vk::PipelineDepthStencilStateCreateInfo depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, vk::CompareOp compareOp) {
    vk::PipelineDepthStencilStateCreateInfo info = {};

    info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
    info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
    info.depthCompareOp = bDepthTest ? compareOp : vk::CompareOp::eAlways;
    info.depthBoundsTestEnable = VK_FALSE;
    info.minDepthBounds = 0.0f; // Optional
    info.maxDepthBounds = 1.0f; // Optional
    info.stencilTestEnable = VK_FALSE;

    return info;
}

/*
void createGraphicsPipeline() {
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}
*/


PipelineBuilder::PipelineBuilder(VkExtent2D windowExtent) {
    vertexDescription = Vertex::get_vertex_description();
    _vertexInputInfo = vertex_input_state_create_info(vertexDescription);
    _inputAssembly = input_assembly_create_info(vk::PrimitiveTopology::eTriangleList);
    _rasterizer = rasterization_state_create_info(vk::PolygonMode::eFill);
    _colorBlendAttachment = color_blend_attachment_state();
    _multisampling = multisampling_state_create_info();
    _depthStencil = depth_stencil_create_info(true, true, vk::CompareOp::eLessOrEqual);

    _viewport.x = 0.0f;
    _viewport.y = 0.0f;
    _viewport.width = (float)windowExtent.width;
    _viewport.height = (float)windowExtent.height;
    _viewport.minDepth = 0.0f;
    _viewport.maxDepth = 1.0f;

    _scissor.offset = vk::Offset2D{ 0, 0 };
    _scissor.extent = windowExtent;
}

void PipelineBuilder::addShader(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule) {
    auto shader = pipeline_shader_stage_create_info(stage, shaderModule);
    _shaderStages.push_back(shader);
}

void PipelineBuilder::addPushConstant(vk::PushConstantRange pushConstant) {
    _pushConstantRange.push_back(pushConstant);
}

vk::Pipeline PipelineBuilder::build_pipeline(vk::Device device, vk::RenderPass pass) {
    //make viewport state from our stored viewport and scissor.
    //at the moment we won't support multiple viewports or scissors
    vk::PipelineViewportStateCreateInfo viewportState = {};

    viewportState.setViewports(_viewport);
    viewportState.setScissors(_scissor);

    //setup dummy color blending. We aren't using transparent objects yet
    //the blending is just "no blend", but we do write to the color attachment
    vk::PipelineColorBlendStateCreateInfo colorBlending = {};

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.setAttachments(_colorBlendAttachment);

    // setup pipeline layout
    auto layoutInfo = pipeline_layout_create_info();
    layoutInfo.setPushConstantRanges(_pushConstantRange);

    _pipelineLayout = device.createPipelineLayout(layoutInfo);


    //build the actual pipeline
    //we now use all of the info structs we have been writing into into this one to create the pipeline
    vk::GraphicsPipelineCreateInfo pipelineInfo = {};

    pipelineInfo.setStages(_shaderStages);

    pipelineInfo.pVertexInputState = &_vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &_inputAssembly;
    
    pipelineInfo.pViewportState = &viewportState;
    
    pipelineInfo.pRasterizationState = &_rasterizer;
    pipelineInfo.pMultisampleState = &_multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    
    pipelineInfo.layout = _pipelineLayout;
    
    pipelineInfo.renderPass = pass;
    pipelineInfo.subpass = 0;
    
    pipelineInfo.pDepthStencilState = &_depthStencil;
    
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    //it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
    auto newPipeline = device.createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo);
    if (newPipeline.result != vk::Result::eSuccess) {
        std::cout << "failed to create pipeline\n";
        return VK_NULL_HANDLE; // failed to create graphics pipeline
    }
    else
    {
        return newPipeline.value;
    }
}
