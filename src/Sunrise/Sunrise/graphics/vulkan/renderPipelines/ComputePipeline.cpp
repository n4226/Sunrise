#include "srpch.h"
#include "ComputePipeline.h"
#include "GraphicsPipeline.h"

namespace sunrise::gfx {


    ComputePipeline::ComputePipeline(vk::Device device, std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts, const std::string& shaderFilePath)
    {

        VkPipelineShaderStageCreateInfo shaderStage =
            GraphicsPipeline::createShaderStageInfo(device, GraphicsPipeline::readFile(shaderFilePath), vk::ShaderStageFlagBits::eCompute);


        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        //pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size(); // Optional
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);


        vk::ComputePipelineCreateInfo pipelineCreatInfo;

        pipelineCreatInfo.layout = pipelineLayout;
        pipelineCreatInfo.stage = shaderStage;

        auto result = device.createComputePipeline(vk::PipelineCache(nullptr), pipelineCreatInfo);

        pipeline = result.value;

        device.destroyShaderModule(shaderStage.module);

    }

}