#include "srpch.h"
#include "RenderPassManager.h"

namespace sunrise::gfx {

	RenderPassManager::RenderPassManager(vk::Device device, VkFormat albedoFormat, VkFormat normalFormat, VkFormat aoFormat, VkFormat swapChainImageFormat, VkFormat depthBufferFormat)
		: albedoFormat(albedoFormat), normalFormat(normalFormat), aoFormat(aoFormat),
		swapChainImageFormat(swapChainImageFormat), depthBufferFormat(depthBufferFormat)
	{
		this->swapChainImageFormat = swapChainImageFormat;
		this->device = device;
		createMainRenderPass();
	}

	RenderPassManager::~RenderPassManager()
	{
		device.destroyRenderPass(renderPass);
	}

	void RenderPassManager::createMainRenderPass()
	{

		/* Sub Passes

			index compute pre pass = compute pre pass - if gpu

			index gbuffer = gbuffer pas - 1 if gpu-driven else 0

			lihting pass

			post passes - indicies + lightingh pass indicy

		*/


#pragma region Define Attachments


		/* Attachment indicies

			Mark: for now using multiple subpasses but in the futuree might have to change this since they do not allow for the modification of neighboring pixels

			Gbuffer:
				0 = Gbuffer albedo + metallic
				1 = Gbuffer normal + roughness
				2 = Gbuffer ao

				3 = depth atatchment

			Deferred Output:

				4 = deferred_colorAttachment


		*/

		// GBuffer

			//color attatchments

		//TODO: fix load ops and store ops
			//TODO -----------------(3,"fix load ops of textures to remove unnecicary clearing") --------------------------------------------------------------------------------------------

		VkAttachmentDescription gbuffer_albedo_metallic{};
		gbuffer_albedo_metallic.format = albedoFormat;
		gbuffer_albedo_metallic.samples = VK_SAMPLE_COUNT_1_BIT;

		gbuffer_albedo_metallic.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		gbuffer_albedo_metallic.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		gbuffer_albedo_metallic.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		gbuffer_albedo_metallic.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		gbuffer_albedo_metallic.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		gbuffer_albedo_metallic.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


		VkAttachmentDescription gbuffer_normal_roughness{};
		gbuffer_normal_roughness.format = normalFormat;
		gbuffer_normal_roughness.samples = VK_SAMPLE_COUNT_1_BIT;

		gbuffer_normal_roughness.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		gbuffer_normal_roughness.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		gbuffer_normal_roughness.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		gbuffer_normal_roughness.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		gbuffer_normal_roughness.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		gbuffer_normal_roughness.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


		VkAttachmentDescription gbuffer_ao{};
		gbuffer_ao.format = aoFormat;
		gbuffer_ao.samples = VK_SAMPLE_COUNT_1_BIT;

		gbuffer_ao.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		gbuffer_ao.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		gbuffer_ao.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		gbuffer_ao.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		gbuffer_ao.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		gbuffer_ao.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// depth

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = depthBufferFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


		// Deferred Output


		VkAttachmentDescription deferred_colorAttachment{};
		deferred_colorAttachment.format = swapChainImageFormat;
		deferred_colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		deferred_colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		deferred_colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		deferred_colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		deferred_colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		deferred_colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		deferred_colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


		// post processing



		std::array<VkAttachmentDescription, 5> attachments = {

			gbuffer_albedo_metallic, gbuffer_normal_roughness, gbuffer_ao, depthAttachment,

			deferred_colorAttachment
		};

#pragma endregion


#pragma region Subpasses

		//GBUffer

		VkSubpassDescription gbuffer{};
		{
			// the attatchment index in the refrenes her refrer to frame buffer attatchment indicies not renderpass indicies above 

			VkAttachmentReference gbuffer_albdo_metallic_AttachmentRef{};
			gbuffer_albdo_metallic_AttachmentRef.attachment = 0;
			// vulkan will transfer image to this layout at start of subpass
			gbuffer_albdo_metallic_AttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference gbuffer_normal_roughness_AttachmentRef{};
			gbuffer_normal_roughness_AttachmentRef.attachment = 1;
			// vulkan will transfer image to this layout at start of subpass
			gbuffer_normal_roughness_AttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference gbuffer_ao_AttachmentRef{};
			gbuffer_ao_AttachmentRef.attachment = 2;
			// vulkan will transfer image to this layout at start of subpass
			gbuffer_ao_AttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 3;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			/// <summary>
			/// indicies in this area are the location() in glsl shaders eg 0th element will be loation(0)
			/// </summary>
			std::array<VkAttachmentReference, 3> colorAttachments = {
				gbuffer_albdo_metallic_AttachmentRef, gbuffer_normal_roughness_AttachmentRef, gbuffer_ao_AttachmentRef
			};

			// flags are whre can enable per view attributes for multi view one gpu rendereing in the future
			gbuffer.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

			gbuffer.colorAttachmentCount = colorAttachments.size();
			gbuffer.pColorAttachments = colorAttachments.data();
			gbuffer.pDepthStencilAttachment = &depthAttachmentRef;

		}

		//Deferred

		VkSubpassDescription deferred{};
		{
			// this passes output tex
			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 4;
			// vulkan will transfer image to this layou at start of subpass
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			// read only input textures
			VkAttachmentReference gbuffer_albdo_metallic_AttachmentRef{};
			gbuffer_albdo_metallic_AttachmentRef.attachment = 0;
			// vulkan will transfer image to this layout at start of subpass
			gbuffer_albdo_metallic_AttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkAttachmentReference gbuffer_normal_roughness_AttachmentRef{};
			gbuffer_normal_roughness_AttachmentRef.attachment = 1;
			// vulkan will transfer image to this layout at start of subpass
			gbuffer_normal_roughness_AttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkAttachmentReference gbuffer_ao_AttachmentRef{};
			gbuffer_ao_AttachmentRef.attachment = 2;
			// vulkan will transfer image to this layout at start of subpass
			gbuffer_ao_AttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkAttachmentReference gbuffer_depth_AttachmentRef{};
			gbuffer_depth_AttachmentRef.attachment = 3;
			// vulkan will transfer image to this layout at start of subpass
			gbuffer_depth_AttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			std::array<VkAttachmentReference, 4> inputAttachments = {
				gbuffer_albdo_metallic_AttachmentRef, gbuffer_normal_roughness_AttachmentRef, gbuffer_ao_AttachmentRef, gbuffer_depth_AttachmentRef
			};


			// flags are whre can enable per view attributes for multi view one gpu rendereing in the future
			deferred.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

			deferred.inputAttachmentCount = inputAttachments.size();
			deferred.pInputAttachments = inputAttachments.data();

			deferred.colorAttachmentCount = 1;
			deferred.pColorAttachments = &colorAttachmentRef;
			deferred.pDepthStencilAttachment = nullptr;

		}



		std::array<VkSubpassDescription, 2> subpasses = {
			gbuffer, deferred
		};

#pragma endregion



		// dependancies for the main (0 th) pass


		VkSubpassDependency externalGBufferDependency{};
		externalGBufferDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		externalGBufferDependency.dstSubpass = 0;
		externalGBufferDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		externalGBufferDependency.srcAccessMask = 0;

		externalGBufferDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		externalGBufferDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;



		VkSubpassDependency DeferredDependencyOnGBuffer{};
		DeferredDependencyOnGBuffer.srcSubpass = 0;
		DeferredDependencyOnGBuffer.dstSubpass = 1;

		DeferredDependencyOnGBuffer.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		DeferredDependencyOnGBuffer.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


		//TODO: for now deferred lighting and will be a fullscreen quad insteasd of compute pass because this is necessary for subpasses 
		DeferredDependencyOnGBuffer.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; //VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		//TODO: add dpeth buffer as input attatchment to deferred pass  | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		DeferredDependencyOnGBuffer.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;



		std::array<VkSubpassDependency, 2> dependencies = {
			externalGBufferDependency, DeferredDependencyOnGBuffer
		};


		// render pass



		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = subpasses.size();
		renderPassInfo.pSubpasses = subpasses.data();
		renderPassInfo.dependencyCount = dependencies.size();
		renderPassInfo.pDependencies = dependencies.data();

		renderPass = device.createRenderPass(renderPassInfo);

	}

}