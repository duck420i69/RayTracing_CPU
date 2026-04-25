#pragma once

#include "vulkan.h"
#include "VulkanObject.h"


struct MatrixPushConstant {
    glm::mat4 transformMatrix;
};


class PipelineBuilder {
public:
    vk::Viewport _viewport;
    vk::Rect2D _scissor;
    std::vector<vk::PushConstantRange> _pushConstantRange;
    std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages;
    vk::PipelineVertexInputStateCreateInfo _vertexInputInfo;
    vk::PipelineInputAssemblyStateCreateInfo _inputAssembly;
    vk::PipelineRasterizationStateCreateInfo _rasterizer;
    vk::PipelineColorBlendAttachmentState _colorBlendAttachment;
    vk::PipelineMultisampleStateCreateInfo _multisampling;
    vk::PipelineDepthStencilStateCreateInfo _depthStencil;
    vk::PipelineLayoutCreateInfo _pipelineLayoutInfo;
    vk::PipelineLayout _pipelineLayout;
    
    VertexInputDescription vertexDescription;

    PipelineBuilder(VkExtent2D windowExtent);

    void addShader(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule);

    void addPushConstant(vk::PushConstantRange pushConstant);

    vk::Pipeline build_pipeline(vk::Device device, vk::RenderPass pass);
};