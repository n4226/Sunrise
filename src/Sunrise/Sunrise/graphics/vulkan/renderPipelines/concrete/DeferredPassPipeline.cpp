#include "srpch.h"
#include "DeferredPassPipeline.h"
#include "../../generalAbstractions/AttributeHelpers.h"
#include "../../resources/uniforms.h"

//DeferredPass::DeferredPass(vk::Device device)
//{

	//std::array<VkDescriptorSetLayoutBinding, 3> setLayoutBindings{};

	//// thses have same indicies as frame buffer and render pass attatchmetns

	//setLayoutBindings[0].binding = 0;
	//setLayoutBindings[0].descriptorCount = 1;
	//setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	//setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//setLayoutBindings[1].binding = 1;
	//setLayoutBindings[1].descriptorCount = 1;
	//setLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	//setLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//setLayoutBindings[1].binding = 2;
	//setLayoutBindings[1].descriptorCount = 1;
	//setLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	//setLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//VkDescriptorSetLayoutCreateInfo layoutInfo{};
	//layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	//layoutInfo.bindingCount = setLayoutBindings.size();
	//layoutInfo.pBindings = setLayoutBindings.data();

	//std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = { device.createDescriptorSetLayout({ layoutInfo }) };


	//pipeline = new ComputePipeline(device, descriptorSetLayouts, "shaders/LightingDeferred.compute.sprv");

//}

//GBufferPass::~GBufferPass()
//{
//	//delete pipeline;
//}

namespace sunrise::gfx {


    void DeferredPass::createPipeline()
    {

        std::array<VkDescriptorSetLayoutBinding, 5> setLayoutBindings{};

        // thses have same indicies as frame buffer and render pass attatchmetns

        setLayoutBindings[0].binding = 0;
        setLayoutBindings[0].descriptorCount = 1;
        setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        setLayoutBindings[1].binding = 1;
        setLayoutBindings[1].descriptorCount = 1;
        setLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        setLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        setLayoutBindings[2].binding = 2;
        setLayoutBindings[2].descriptorCount = 1;
        setLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        setLayoutBindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        setLayoutBindings[3].binding = 3;
        setLayoutBindings[3].descriptorCount = 1;
        setLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        setLayoutBindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


        setLayoutBindings[4].binding = 4;
        setLayoutBindings[4].descriptorCount = 1;
        setLayoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        setLayoutBindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = setLayoutBindings.size();
        layoutInfo.pBindings = setLayoutBindings.data();

        descriptorSetLayouts = { device.createDescriptorSetLayout({ layoutInfo }) };


        // programmable stages 

        auto vertShaderCode = readFile("shaders/GBuffer.vert.spv");
        auto fragShaderCode = readFile("shaders/GBuffer.frag.spv");


        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
            GraphicsPipeline::createShaderStageInfo(device,vertShaderCode,vk::ShaderStageFlagBits::eVertex),
            GraphicsPipeline::createShaderStageInfo(device,fragShaderCode,vk::ShaderStageFlagBits::eFragment),
        };

        // vertex input 

        auto bindingDescription =  makeVertBinding(0, sizeof(glm::vec2));
        auto attributeDescription = makeVertAttribute(0, 0, VertexAttributeFormat::vec2, 0);

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 1;
        vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription; // Optional

        // fixed function stages 

            //depth/stencil

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.depthWriteEnable = VK_FALSE;

        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional

        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional



        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
        scissor.extent = swapChainExtent;

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
        rasterizer.lineWidth = 1.0f;

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


        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
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
        pushConstRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
        pushConstRange.offset = 0;
        pushConstRange.size = sizeof(DrawPushData);


        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        //pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size(); // Optional
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

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
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr; // Optional

        pipelineInfo.layout = pipelineLayout;

        pipelineInfo.renderPass = renderPassManager.renderPass;
        pipelineInfo.subpass = 1;

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

}