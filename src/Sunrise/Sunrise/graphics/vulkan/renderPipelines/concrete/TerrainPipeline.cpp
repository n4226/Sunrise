#include "srpch.h"
#include "TerrainPipeline.h"

#include "Sunrise/Sunrise/Math.h"
#include "../../resources/uniforms.h"

namespace sunrise::gfx {

    void TerrainPipeline::createPipeline()
    {
        // DescriptorSetLayout

        // global uniforms - 
        // set = 0, binding = 0 -> viewProj
        // set = 0, binding = 1 -> modelMat - per opbject/instance
        // set = 0, binding = 2 -> materialConstants - each draw call gets an index into this which has constants and texture indicies for that mat 
        // set = 0, binding = 3 -> materialImages - mat constant has a base index into this descriptor arrey which for now will have 5 contiguous indices for each texture 

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional


        // this is now one descripor to a large buffer where shaders will offset into witrh a model index

        VkDescriptorSetLayoutBinding modelUniformLayoutBinding{};
        modelUniformLayoutBinding.binding = 1;
        modelUniformLayoutBinding.descriptorType =
            //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        // the max number of elements in this varible size array
        modelUniformLayoutBinding.descriptorCount = 1;//maxModelUniformDescriptorArrayCount;
        modelUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        modelUniformLayoutBinding.pImmutableSamplers = nullptr; // Optional

        // this is now one descripor to a large buffer where shaders will offset into with a mat index

        VkDescriptorSetLayoutBinding materialUniformLayoutBinding{};
        materialUniformLayoutBinding.binding = 2;
        materialUniformLayoutBinding.descriptorType =
            //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        materialUniformLayoutBinding.descriptorCount = 1;
        materialUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        materialUniformLayoutBinding.pImmutableSamplers = nullptr; // Optional


        // this is a descritpor array of all textures that can be used by (fragment) shaders
        // i will be suing combined image samplers because that seems to be the best - might be slightly worse performance though but see https://stackoverflow.com/questions/60384671/combined-image-samplers-vs-seprate-sampled-image-and-sampler
        VkDescriptorSetLayoutBinding materialTexturesLayoutBinding{};
        materialTexturesLayoutBinding.binding = 3;
        materialTexturesLayoutBinding.descriptorType =
            //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        // the max number of elements in this varible size array
        materialTexturesLayoutBinding.descriptorCount = maxMaterialTextureDescriptorArrayCount;
        materialTexturesLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        materialTexturesLayoutBinding.pImmutableSamplers = nullptr; // Optional




        std::array< VkDescriptorSetLayoutBinding, 4> bindings = {
            uboLayoutBinding, modelUniformLayoutBinding, materialUniformLayoutBinding, materialTexturesLayoutBinding
        };

        // this allows the adding of flags for each binding in the layout
        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo;
        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlagsInfo.pNext = nullptr;

        std::array<VkDescriptorBindingFlags, 4> bindingFlags = {
            // global uniform
            VkDescriptorBindingFlags(),
            // model uniforms
            VkDescriptorBindingFlags(),
            // mat uniforms
            VkDescriptorBindingFlags(),

            // mat textures
            VkDescriptorBindingFlags(
                //vk::DescriptorBindingFlagBits::eUpdateAfterBind is not availabel on 1080 ti for uniform buffers
                vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending
            )
        };

        bindingFlagsInfo.bindingCount = bindingFlags.size();
        bindingFlagsInfo.pBindingFlags = bindingFlags.data();

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = bindings.size();
        layoutInfo.pBindings = bindings.data();
        // might not need this
        //layoutInfo.flags = VkDescriptorSetLayoutCreateFlagBits(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool);
        layoutInfo.pNext = &bindingFlagsInfo;

        descriptorSetLayouts.push_back(device.createDescriptorSetLayout({ layoutInfo }));

        // programmable stages 

        auto vertShaderCode = readFile("shaders/terrain.vert.spv");
        auto fragShaderCode = readFile("shaders/terrain.frag.spv");


        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
            GraphicsPipeline::createShaderStageInfo(device,vertShaderCode,vk::ShaderStageFlagBits::eVertex),
            GraphicsPipeline::createShaderStageInfo(device,fragShaderCode,vk::ShaderStageFlagBits::eFragment),
        };

        // vertex input 

        auto bindingDescription =    Mesh::getBindingDescription();
        auto attributeDescriptions = Mesh::getAttributeDescriptions();

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

        std::vector<VkPipelineColorBlendAttachmentState> colorBlends(RenderPassManager::gbufferAttachmentCount, colorBlendAttachment);

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
        pushConstRange.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
        pushConstRange.offset = 0;
        pushConstRange.size = sizeof(DrawPushData);


        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        //pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size(); // Optional
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
        pipelineLayoutInfo.pPushConstantRanges = &pushConstRange; // Optional

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
}