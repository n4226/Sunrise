#include "srpch.h"
#include "GraphicsPipeline.h"

namespace sunrise::gfx {


    GraphicsPipeline::GraphicsPipeline(vk::Device device, vk::Extent2D swapChainExtent, RenderPassManager& renderPassManager)
        : renderPassManager(renderPassManager), swapChainExtent(swapChainExtent)
    {
        this->device = device;

    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        device.destroyPipelineLayout(pipelineLayout);
        device.destroyPipeline(vkItem);

        for (vk::DescriptorSetLayout& layout : descriptorSetLayouts) {
            device.destroyDescriptorSetLayout(layout);
        }
    }



    /// <summary>
    /// wrapes a shader binary data into a PipelineShaderStageCreateInfo struct
    /// </summary>
    /// <param name="device"></param>
    /// <param name="code"></param>
    /// <param name="stage"></param>
    /// <returns>WARNING the returned value needs to be deinitilized when it is done being used with device.destroyShaderModule </returns>
    vk::PipelineShaderStageCreateInfo GraphicsPipeline::createShaderStageInfo(vk::Device device, const std::vector<char>& code, vk::ShaderStageFlagBits stage)
    {
        auto shaderModule = GraphicsPipeline::createShaderModule(device, code);

        vk::PipelineShaderStageCreateInfo shaderStageInfo{};

        shaderStageInfo.stage = stage;//vk::ShaderStageFlagBits::eVertex;
        shaderStageInfo.module = shaderModule;
        shaderStageInfo.pName = "main";


        return shaderStageInfo;
    }

    vk::ShaderModule GraphicsPipeline::createShaderModule(vk::Device device, const std::vector<char>& code)
    {
        vk::ShaderModuleCreateInfo createInfo{};

        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        auto shaderModule = device.createShaderModule(createInfo, nullptr);

        return shaderModule;
    }

    std::vector<char> GraphicsPipeline::readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

}