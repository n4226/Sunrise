#include "srpch.h"
#include "GraphicsPipeline.h"

#include "Sunrise/Sunrise/fileSystem/FileManager.h"

namespace sunrise::gfx {


    GraphicsPipeline::GraphicsPipeline(vk::Device device, vk::Extent2D swapChainExtent, RenderPassManager& renderPassManager)
        : renderPassManager(renderPassManager), swapChainExtent(swapChainExtent)
    {
        this->device = device;

    }

    GraphicsPipeline::GraphicsPipeline(vk::Device device, vk::Extent2D swapChainExtent, RenderPassManager& renderPassManager, GraphicsPipelineOptions& options)
        : renderPassManager(renderPassManager), swapChainExtent(swapChainExtent)
    {
        this->device = device;

        createPipeline(options);

    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        device.destroyPipelineLayout(pipelineLayout);
        device.destroyPipeline(vkItem);

        for (vk::DescriptorSetLayout& layout : descriptorSetLayouts) {
            DescriptorSetLayout::Destroy(layout,device);
        }
    }

    void GraphicsPipeline::createPipeline(const GraphicsPipelineOptions& options)
    {
        /*
          Required inputs:

              descriptos set layout bindings
              discriptor set layouts

              options {
                  shader files and uses

                  mesh binding and attribute descriptions

                  depth options -- most simplified out for now

                  push constants enabled, size, and stages



              }
      */


        descriptorSetLayouts.resize(options.descriptorSetLayouts.size());

        std::transform(options.descriptorSetLayouts.begin(), options.descriptorSetLayouts.end(), descriptorSetLayouts.begin(), [this](auto& item) {
            return DescriptorSetLayout::Create(item, device);
        });

        // programmable stages 

        SR_CORE_TRACE("going to load shader files");

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {};


        for (auto shStage : options.shaderStages)
        {
            shaderStages.push_back(static_cast<VkPipelineShaderStageCreateInfo>(GraphicsPipeline::createShaderStageInfo(device, readFile(shStage.shaderPath), shStage.shaderStage)));
        }


        // vertex input 

        auto& bindingDescription = options.bindingDescriptions;
        auto& attributeDescriptions = options.attributeDescriptions;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        //TODO: this used to be the size of attributeDescriptions but that looked wrong
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data(); // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

        // fixed function stages 

            //depth/stencil

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;

        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional

        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional



        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = static_cast<VkPrimitiveTopology>(options.primitiveType);//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;


        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = static_cast<VkExtent2D>(swapChainExtent);

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        // suful for shadow maps look at tutorial
        rasterizer.depthClampEnable = VK_FALSE;

        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = options.lineWidth;

        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        // TODO: i thought - turn this to couter clockwise for my gpu driven models to work 
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        // THIS is an empty one with no blending
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
        // below is for alpha blending above is no blending
        /*colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;*/



        std::vector<VkPipelineColorBlendAttachmentState> colorBlends;//(RenderPassManager::gbufferAttachmentCount, colorBlendAttachment);

        colorBlends.resize(renderPassManager.getColorAttatchmentCount());

        for (size_t i = 0; i < colorBlends.size(); i++)
        {
            colorBlends[i] = colorBlendAttachment;
        }

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = colorBlends.size();
        colorBlending.pAttachments = colorBlends.data();
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            //VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 1;
        dynamicState.pDynamicStates = dynamicStates;

        vk::PushConstantRange pushConstRange;
        if (options.enablePushConstants) {
            pushConstRange.stageFlags = options.pushConstantStages;
            pushConstRange.offset = options.pushConstantOffset;
            pushConstRange.size = options.pushConstantSize;
        }



        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        //pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size(); // Optional
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // Optional
        if (options.enablePushConstants) {
            pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
            pipelineLayoutInfo.pPushConstantRanges =  &pushConstRange; // Optional
        }
        else {
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr; 
        }
        pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

        // create pipeline

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr; // Optional

        pipelineInfo.layout = pipelineLayout;

        pipelineInfo.renderPass = renderPassManager.renderPass;
        pipelineInfo.subpass = 0;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        auto result = device.createGraphicsPipeline(vk::PipelineCache(nullptr), pipelineInfo, nullptr);

        vkItem = result.value;

        assert(result.result == vk::Result::eSuccess);

        // destroy transient objects 

        for (size_t i = 0; i < shaderStages.size(); i++)
        {
            device.destroyShaderModule(shaderStages[i].module);
        }
    }

    void GraphicsPipeline::createPipeline()
    {
        /*
            Required inputs:

                descriptos set layout bindings
                discriptor set layouts

                options {
                    shader files and uses
                    
                    mesh binding and attribute descriptions

                    depth options -- most simplified out for now

                    push constants enabled, size, and stages



                }
        */
        //TODO: all this
        //std::array< VkDescriptorSetLayoutBinding, 4> bindings = {
        //    uboLayoutBinding, modelUniformLayoutBinding, materialUniformLayoutBinding, materialTexturesLayoutBinding
        //};

      

        //VkDescriptorSetLayoutCreateInfo layoutInfo{};
        //layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        //layoutInfo.bindingCount = bindings.size();
        //layoutInfo.pBindings = bindings.data();
        //// might not need this
        ////layoutInfo.flags = VkDescriptorSetLayoutCreateFlagBits(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool);
        //layoutInfo.pNext = &bindingFlagsInfo;

        //descriptorSetLayouts.push_back(device.createDescriptorSetLayout({ layoutInfo }));

        //// programmable stages 

        //auto vertShaderCode = readFile("shaders/terrain.vert.spv");
        //auto fragShaderCode = readFile("shaders/terrain.frag.spv");


        //std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
        //    GraphicsPipeline::createShaderStageInfo(device,vertShaderCode,vk::ShaderStageFlagBits::eVertex),
        //    GraphicsPipeline::createShaderStageInfo(device,fragShaderCode,vk::ShaderStageFlagBits::eFragment),
        //};

        //// vertex input 

        //auto bindingDescription =    Mesh::getBindingDescription();
        //auto attributeDescriptions = Mesh::getAttributeDescriptions();

        //VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        //vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        ////TODO: this used to be the size of attributeDescriptions but that looked wrong
        //vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
        //vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data(); // Optional
        //vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        //vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

        //// fixed function stages 

        //    //depth/stencil

        //VkPipelineDepthStencilStateCreateInfo depthStencil{};
        //depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        //depthStencil.depthTestEnable = VK_TRUE;
        //depthStencil.depthWriteEnable = VK_TRUE;

        //depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

        //depthStencil.depthBoundsTestEnable = VK_FALSE;
        //depthStencil.minDepthBounds = 0.0f; // Optional
        //depthStencil.maxDepthBounds = 1.0f; // Optional

        //depthStencil.stencilTestEnable = VK_FALSE;
        //depthStencil.front = {}; // Optional
        //depthStencil.back = {}; // Optional



        //VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        //inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        //inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        //inputAssembly.primitiveRestartEnable = VK_FALSE;

        //VkViewport viewport{};
        //viewport.x = 0.0f;
        //viewport.y = 0.0f;
        //viewport.width = (float)swapChainExtent.width;
        //viewport.height = (float)swapChainExtent.height;
        //viewport.minDepth = 0.0f;
        //viewport.maxDepth = 1.0f;


        //VkRect2D scissor{};
        //scissor.offset = { 0, 0 };
        //scissor.extent = swapChainExtent;

        //VkPipelineViewportStateCreateInfo viewportState{};
        //viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        //viewportState.viewportCount = 1;
        //viewportState.pViewports = &viewport;
        //viewportState.scissorCount = 1;
        //viewportState.pScissors = &scissor;

        //VkPipelineRasterizationStateCreateInfo rasterizer{};
        //rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        //// suful for shadow maps look at tutorial
        //rasterizer.depthClampEnable = VK_FALSE;

        //rasterizer.rasterizerDiscardEnable = VK_FALSE;
        //rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        //rasterizer.lineWidth = 1.0f;

        //rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        //// TODO: i thought - turn this to couter clockwise for my gpu driven models to work 
        //rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

        //rasterizer.depthBiasEnable = VK_FALSE;
        //rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        //rasterizer.depthBiasClamp = 0.0f; // Optional
        //rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


        //VkPipelineMultisampleStateCreateInfo multisampling{};
        //multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        //multisampling.sampleShadingEnable = VK_FALSE;
        //multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        //multisampling.minSampleShading = 1.0f; // Optional
        //multisampling.pSampleMask = nullptr; // Optional
        //multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        //multisampling.alphaToOneEnable = VK_FALSE; // Optional


        //VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        //colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        //colorBlendAttachment.blendEnable = VK_FALSE;
        //colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        //colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        //colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        //colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        //colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        //colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
        //// below is for alpha blending above is no blending
        ///*colorBlendAttachment.blendEnable = VK_TRUE;
        //colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        //colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        //colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        //colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        //colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        //colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;*/

        //std::vector<VkPipelineColorBlendAttachmentState> colorBlends(RenderPassManager::gbufferAttachmentCount, colorBlendAttachment);

        //VkPipelineColorBlendStateCreateInfo colorBlending{};
        //colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        //colorBlending.logicOpEnable = VK_FALSE;
        //colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        //colorBlending.attachmentCount = colorBlends.size();
        //colorBlending.pAttachments = colorBlends.data();
        //colorBlending.blendConstants[0] = 0.0f; // Optional
        //colorBlending.blendConstants[1] = 0.0f; // Optional
        //colorBlending.blendConstants[2] = 0.0f; // Optional
        //colorBlending.blendConstants[3] = 0.0f; // Optional

        //VkDynamicState dynamicStates[] = {
        //    VK_DYNAMIC_STATE_VIEWPORT,
        //    //VK_DYNAMIC_STATE_LINE_WIDTH
        //};

        //VkPipelineDynamicStateCreateInfo dynamicState{};
        //dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        //dynamicState.dynamicStateCount = 1;
        //dynamicState.pDynamicStates = dynamicStates;

        //vk::PushConstantRange pushConstRange;
        //pushConstRange.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
        //pushConstRange.offset = 0;
        //pushConstRange.size = sizeof(DrawPushData);


        //vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        ////pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        //pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size(); // Optional
        //pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // Optional
        //pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
        //pipelineLayoutInfo.pPushConstantRanges = &pushConstRange; // Optional

        //pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

        //// create pipeline

        //VkGraphicsPipelineCreateInfo pipelineInfo{};
        //pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        //pipelineInfo.stageCount = shaderStages.size();
        //pipelineInfo.pStages = shaderStages.data();
        //pipelineInfo.pVertexInputState = &vertexInputInfo;
        //pipelineInfo.pInputAssemblyState = &inputAssembly;
        //pipelineInfo.pViewportState = &viewportState;
        //pipelineInfo.pRasterizationState = &rasterizer;
        //pipelineInfo.pMultisampleState = &multisampling;
        //pipelineInfo.pDepthStencilState = &depthStencil; // Optional
        //pipelineInfo.pColorBlendState = &colorBlending;
        //pipelineInfo.pDynamicState = nullptr; // Optional

        //pipelineInfo.layout = pipelineLayout;

        //pipelineInfo.renderPass = renderPassManager.renderPass;
        //pipelineInfo.subpass = 0;

        //pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        //pipelineInfo.basePipelineIndex = -1; // Optional

        //auto result = device.createGraphicsPipeline(vk::PipelineCache(nullptr), pipelineInfo, nullptr);

        //vkItem = result.value;

        //assert(result.result == vk::Result::eSuccess);

        //// destroy transient objects 

        //for (size_t i = 0; i < shaderStages.size(); i++)
        //{
        //    device.destroyShaderModule(shaderStages[i].module);
        //}
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

#if SR_ENABLE_PRECONDITION_CHECKS
        if (!FileManager::exists(filename)) {
            SR_CORE_ERROR("Going to open a shader file which does not exist: {}",filename);
            SR_CORE_INFO("this might be beacuse the working directory is incorect. It currently is: {}", FileManager::appWokringDir());
        }
#endif

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


    VirtualGraphicsPipeline::VirtualGraphicsPipeline()
    {
        definition = makeDeff();
    }


	VirtualGraphicsPipeline::~VirtualGraphicsPipeline()
	{

	}

    void VirtualGraphicsPipeline::create()
    {
        definition = makeDeff();
    }

    GraphicsPipelineOptions VirtualGraphicsPipeline::makeDeff()
    {
        return {};
    }


}