#include "srpch.h"
#include "GPUGenCommandsPipeline.h"
//

#include "../../GraphicsPipeline.h"
#include "Sunrise/Sunrise/core/Application.h"
#include "Sunrise/Sunrise/math/mesh/Mesh.h"
#include "Sunrise/Sunrise/graphics/vulkan/resources/uniforms.h"

#include <vulkan/vulkan.hpp>
#include <array>


namespace sunrise::gfx {

	GPUGenCommandsPipeline::GPUGenCommandsPipeline(Application& app, vk::Device device, GraphicsPipeline& terrainPipeline)
		: app(app), terrainPipeline(terrainPipeline),
		ComputePipeline::ComputePipeline(device, createDesLayouts(device), "shaders/gpuGenCommands.comp.spv")
	{
		//std::vector<vk::DescriptorSetLayout> layouts = ;


		vk::DynamicLoader dl;

		// This dispatch class will fetch function pointers for the passed device if possible, else for the passed instance
		dldid = vk::DispatchLoaderDynamic(app.instance, dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"), device);

		this->device = device;

		pfn_vkCreateIndirectCommandsLayoutNV = (PFN_vkCreateIndirectCommandsLayoutNV)dldid.vkGetDeviceProcAddr(device, "vkCreateIndirectCommandsLayoutNV");
		pfn_vkGetGeneratedCommandsMemoryRequirementsNV = (PFN_vkGetGeneratedCommandsMemoryRequirementsNV)dldid.vkGetDeviceProcAddr(device, "vkGetGeneratedCommandsMemoryRequirementsNV");
		
		

		setup();
	}

	void GPUGenCommandsPipeline::exicuteIndirectCommands(vk::CommandBuffer cmdBuff, uint32_t drawCount, BindlessMeshBuffer* meshBuff)
	{


		// TODO: find better way to store fn pointers pfn (potential future nonsense)
		static PFN_vkCmdExecuteGeneratedCommandsNV vkCmdExecuteGeneratedCommandsNV = (PFN_vkCmdExecuteGeneratedCommandsNV)dldid.vkGetDeviceProcAddr(device, "vkCmdExecuteGeneratedCommandsNV");

		const auto optimized = VK_FALSE;

		vk::GeneratedCommandsInfoNV info{};
		//info.sType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_NV;
		info.pNext = nullptr;
		info.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		info.pipeline = terrainPipeline.vkItem;
		info.indirectCommandsLayout = commandsLayout;

		// returns a pointer to the streams that should be deleted when the vkGeneratedCommandsInfoNV struct is no longer neaded
		auto streams = meshBuff->bindVerticiesIntoIndirectCommandsNV(info);


		//TODO: this is until i get a allow the kernal to cull and change the number of objects to draw
		info.sequencesCount = drawCount;

		// the buffer to execute from, if isPreprocessed is true than they will not be proccesed again but simply executed. 
		//  if isPreprocessed is false than they will be pre proccesed before execuited 
		info.preprocessBuffer = commandsBuffer->vkItem;
		info.preprocessOffset = 0;
		info.preprocessSize = commandsBuffer->size;

		info.sequencesCountBuffer = static_cast<VkBuffer>(VK_NULL_HANDLE);
		info.sequencesCountOffset = 0;
		info.sequencesIndexBuffer = static_cast<VkBuffer>(VK_NULL_HANDLE);
		info.sequencesIndexOffset = 0;

		VkGeneratedCommandsInfoNV c_info = info;

		vkCmdExecuteGeneratedCommandsNV(cmdBuff, optimized, &c_info);


		delete streams;
	}

	void GPUGenCommandsPipeline::setup()
	{

		// Create VkIndirectCommandsLayoutNV
		createCommandsLayout();

		//10.211.55.3
		// genrerate memory requirements
		getMemRequirements();

	}

	void GPUGenCommandsPipeline::createCommandsLayout()
	{
		std::array<vk::IndirectCommandsLayoutTokenNV, 4> tokenLayouts{};
		//VkIndirectCommandsLayoutTokenNV
		// define stream here
		tokenLayouts[0].tokenType = vk::IndirectCommandsTokenTypeNV::eDrawIndexed;
		tokenLayouts[0].stream = 0;
		tokenLayouts[0].offset = 0;
		auto types = vk::IndexType::eUint32;
		tokenLayouts[0].pIndexTypes = &types;
		tokenLayouts[0].indirectStateFlags = {};


		tokenLayouts[1].tokenType = vk::IndirectCommandsTokenTypeNV::eVertexBuffer;
		tokenLayouts[1].stream = 0;
		tokenLayouts[1].offset = 0;
		tokenLayouts[1].vertexBindingUnit = 0;
		tokenLayouts[1].vertexDynamicStride = VK_FALSE;

		tokenLayouts[2].tokenType = vk::IndirectCommandsTokenTypeNV::eIndexBuffer;
		tokenLayouts[2].stream = 0;
		tokenLayouts[2].offset = 0;

		tokenLayouts[3].tokenType = vk::IndirectCommandsTokenTypeNV::ePushConstant;
		tokenLayouts[3].stream = 0;
		tokenLayouts[3].offset = 0;
		tokenLayouts[3].pushconstantPipelineLayout = terrainPipeline.pipelineLayout;
		tokenLayouts[3].pushconstantShaderStageFlags = vk::ShaderStageFlagBits::eFragment;
		tokenLayouts[3].pushconstantOffset = 0;
		tokenLayouts[3].pushconstantSize = sizeof(DrawPushData);



		vk::IndirectCommandsLayoutCreateInfoNV info{};

		info.pNext = nullptr;
		info.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;




		// will be providoing a custom subset see below
		// also see docs: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkIndirectCommandsLayoutUsageFlagBitsNV.html
		//VK_INDIRECT_COMMANDS_LAYOUT_USAGE_INDEXED_SEQUENCES_BIT_NV specifies that the input data for the sequences is not implicitly indexed from 0..sequencesUsed but a user provided VkBuffer encoding the index is provided.

		// i can use this feature to frustrum cull objects in the gpu shader by removing the indicies from a seperate buffer but than adding them back when the objects com back into view instead of reencoding the command
		// also every camera can just have its own buffer which says which indicies to draw for that monitor and the actuall comands can just be encoded once until terrain changes

		// this can be done by having 2 compute shaders,
		//		one which will encode all the commands which can be exicuted on a async compute queue when chunks change (very simular to the way cpu2 pipe works but when it will reencode on a seperate thread. also this approch can have frustrum culling)
		//		and another which selectevly puts those comands into the indicies buffer to do culling 
		//		we will have to see if this is more efficent with testing

		// the eIndexedSequences allows implimentation to draw the calls in a non linear order
		info.flags = vk::IndirectCommandsLayoutUsageFlagBitsNV::eIndexedSequences;//vk::IndirectCommandsLayoutUsageFlagBitsNV::eIndexedSequences;
		info.setTokens(tokenLayouts);


		auto bindingDescription = Mesh::getBindingDescription();
		auto attributeDescriptions = Mesh::getAttributeDescriptions();

		std::vector<uint32_t> strides(bindingDescription.size());

		auto i = 0;
		for (size_t i = 0; i < bindingDescription.size(); i++)
		{
			strides[i] = bindingDescription[i].stride;
			i++;
		}

		info.streamCount = bindingDescription.size();
		info.setStreamStrides(strides);


		VkIndirectCommandsLayoutCreateInfoNV c_info = info;

		//vkCreateIndirectCommandsLayoutNV(device, &info_c, nullptr, result);



		pfn_vkCreateIndirectCommandsLayoutNV(device, &c_info, nullptr, &commandsLayout);

		//auto res = device.createIndirectCommandsLayoutNV(info,nullptr,dl);
	}

	void GPUGenCommandsPipeline::getMemRequirements()
	{
		VkGeneratedCommandsMemoryRequirementsInfoNV info{};

		info.pipelineBindPoint = (VkPipelineBindPoint)vk::PipelineBindPoint::eGraphics;
		info.sType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_NV;
		info.pipeline = terrainPipeline.vkItem;
		info.indirectCommandsLayout = commandsLayout;
		info.maxSequencesCount = 10'000;

		VkMemoryRequirements2 result{};
		result.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		pfn_vkGetGeneratedCommandsMemoryRequirementsNV(device, &info, &result);

		BufferCreationOptions options =
		{ ResourceStorageType::gpu, { vk::BufferUsageFlagBits::eIndirectBuffer }, vk::SharingMode::eExclusive, {} };
		options.memoryTypeBits = result.memoryRequirements.memoryTypeBits;
		commandsBuffer = new Buffer(device, app.allocators[0], result.memoryRequirements.size, options);
	}

	std::vector<vk::DescriptorSetLayout> GPUGenCommandsPipeline::createDesLayouts(vk::Device device)
	{
		return { 
			// layout 1
			DescriptorSetLayout::Create({DescriptorSetLayoutBinding::createWholeSet({
				// binding 1 - the main command stream
				{vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute},
				// binding 2 =- nothing yet
				//{}, 
			})}, device)
		};
	}

}