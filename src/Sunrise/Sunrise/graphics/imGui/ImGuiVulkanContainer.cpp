#include "srpch.h"
#include "ImGuiVulkanContainer.h"

#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#include "Sunrise/core/Application.h"
#include "Sunrise/core/Window.h"

#include "Sunrise/graphics/vulkan/resources/ResourceTransferTask.h"

namespace sunrise::gfx {

	void ImGuiVulkanContainer::generateIMGUIResources(Application& app)
	{
		// see: https://vkguide.dev/docs/extra-chapter/implementing_imgui/
		// 
		// also see https://frguthmann.github.io/posts/vulkan_imgui/
		

		//TODO: pick better size counts
		//1: create descriptor pool for IMGUI
		// the size of the pool is very oversize, but it's copied from imgui demo itself.
		std::vector<DescriptorPool::CreateOptions::DescriptorTypeAllocOptions> pool_sizes =
		{
			(DescriptorPool::CreateOptions::DescriptorTypeAllocOptions)vk::DescriptorPoolSize({ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 }),
			(DescriptorPool::CreateOptions::DescriptorTypeAllocOptions)vk::DescriptorPoolSize({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 }),
			(DescriptorPool::CreateOptions::DescriptorTypeAllocOptions)vk::DescriptorPoolSize({ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 }),
			(DescriptorPool::CreateOptions::DescriptorTypeAllocOptions)vk::DescriptorPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 }),
			(DescriptorPool::CreateOptions::DescriptorTypeAllocOptions)vk::DescriptorPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 }),
			(DescriptorPool::CreateOptions::DescriptorTypeAllocOptions)vk::DescriptorPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 }),
			(DescriptorPool::CreateOptions::DescriptorTypeAllocOptions)vk::DescriptorPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 }),
			(DescriptorPool::CreateOptions::DescriptorTypeAllocOptions)vk::DescriptorPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 }),
			(DescriptorPool::CreateOptions::DescriptorTypeAllocOptions)vk::DescriptorPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 }),
			(DescriptorPool::CreateOptions::DescriptorTypeAllocOptions)vk::DescriptorPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 }),
			(DescriptorPool::CreateOptions::DescriptorTypeAllocOptions)vk::DescriptorPoolSize({ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 })
		};

		descriptorPool = new gfx::DescriptorPool(app.renderers[0]->device, { 1000, pool_sizes });

	
	}


	void ImGuiVulkanContainer::destroyIMGUIResources(Application& app)
	{
		delete descriptorPool;
	}

	void ImGuiVulkanContainer::setupIMGUIForWindow(Window* window, Application& app)
	{
		ImGui_ImplGlfw_InitForVulkan(window->window,true);


		// create spacific render pass

		VkAttachmentDescription attachment = {};
		attachment.format = window->swapchainImageFormat;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;

		renderPass = window->renderer->device.createRenderPass(info);

		//TODO: move to renderer spacififc to support multi monitor

		auto renderer = window->renderer;
		// 2: initialize imgui library

		//this initializes the core structures of imgui
		ImGui::CreateContext();


		//this initializes imgui for Vulkan
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = app.instance;
		init_info.PhysicalDevice = renderer->physicalDevice;
		init_info.Device = renderer->device;
		init_info.Queue = renderer->deviceQueues.graphics;
		init_info.DescriptorPool = descriptorPool->vkItem;
		init_info.MinImageCount = renderer->physicalWindows[0]->swapChainImages.size();
		init_info.ImageCount = init_info.MinImageCount;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&init_info, renderPass);



		//TODO: fix for multi window
		auto task = ResourceTransferer::Task{};
		task.type = ResourceTransferer::TaskType::imguiFontGen;

		app.renderers[0]->resouceTransferer->newTask({ {task} }, []() {

			}, true, true);

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void ImGuiVulkanContainer::cleanupIMGUIForWindow(Window* window, Application& app)
	{
	}

	void ImGuiVulkanContainer::setupIMGUIForRenderer(Renderer* renderer, Application& app)
	{
		//TODO: WARN: imGUi appears to only work with single gpu

		SR_CORE_CRITICAL("{} Not Implimented", SR_FUNC_SIG);
		SR_CORE_ASSERT(false);
		
	}



}
