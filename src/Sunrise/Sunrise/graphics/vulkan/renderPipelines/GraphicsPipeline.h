#pragma once

#include "srpch.h"
#include "../RenderPassManager.h"
//#include <GLFW/glfw3.h>
//#include "../../dataObjects/Mesh.h"

namespace sunrise::gfx {

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
	struct GraphicsPipelineOptions {
		// descriptor stuff	

		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

		struct ShaderStageOptions {
			std::string shaderPath;
			vk::ShaderStageFlagBits shaderStage;
		};

		std::vector<ShaderStageOptions> shaderStages;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		//TODO add suportfor multiple push ranges
		bool enablePushConstants = false;
		VkDeviceSize pushConstantOffset = 0;
		VkDeviceSize pushConstantSize = 0;
		vk::ShaderStageFlags pushConstantStages;


		//TODO: add depth / stencil controls

		//TODO: add many more control varibles here such as winding order and backface culling and msaa ...


	};


	class ComputePipeline;

	/// <summary>
	/// THIS CLASS DOES NOT HAVE A copy assignment operator so CANNOT BE PLACED IN A VECTOR. Pointers to it can e.g std::vector<GraphicsPipeline*>
	/// </summary>
	class SUNRISE_API GraphicsPipeline
	{
	public:

		GraphicsPipeline(vk::Device device, vk::Extent2D swapChainExtent, RenderPassManager& renderPassManager);
		GraphicsPipeline(vk::Device device, vk::Extent2D swapChainExtent, RenderPassManager& renderPassManager, GraphicsPipelineOptions& options);
		~GraphicsPipeline();

		/*GraphicsPipeline& GraphicsPipeline::operator=(const GraphicsPipeline&) {
			return *this;
		}*/

		// pipeline properties

		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
		VkPipelineLayout pipelineLayout;

		vk::Pipeline vkItem;

		RenderPassManager& renderPassManager;

		virtual void createPipeline();
		virtual void createPipeline(GraphicsPipelineOptions& options);

		static vk::PipelineShaderStageCreateInfo createShaderStageInfo(vk::Device device, const std::vector<char>& code, vk::ShaderStageFlagBits stage);

		static vk::ShaderModule createShaderModule(vk::Device device, const std::vector<char>& code);

		static std::vector<char> readFile(const std::string& filename);

	protected:


		vk::Device device;

		vk::Extent2D swapChainExtent;

		friend ComputePipeline;
	};


	class SUNRISE_API VirtualGraphicsPipeline {
	public:
		VirtualGraphicsPipeline(GraphicsPipelineOptions&& definition);

		GraphicsPipelineOptions definition;
		std::vector<GraphicsPipeline*> instances = {};
	};


}