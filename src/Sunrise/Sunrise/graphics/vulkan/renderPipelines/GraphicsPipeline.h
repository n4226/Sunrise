#pragma once

#include "srpch.h"
#include "../RenderPassManager.h"
//#include <GLFW/glfw3.h>
//#include "../../dataObjects/Mesh.h"

namespace sunrise::gfx {

	class ComputePipeline;

	class SUNRISE_API GraphicsPipeline
	{
	public:

		GraphicsPipeline(vk::Device device, vk::Extent2D swapChainExtent, RenderPassManager& renderPassManager);
		~GraphicsPipeline();

		// pipeline properties

		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
		VkPipelineLayout pipelineLayout;

		vk::Pipeline vkItem;

		RenderPassManager& renderPassManager;

		virtual void createPipeline();

		static vk::PipelineShaderStageCreateInfo createShaderStageInfo(vk::Device device, const std::vector<char>& code, vk::ShaderStageFlagBits stage);

		static vk::ShaderModule createShaderModule(vk::Device device, const std::vector<char>& code);

		static std::vector<char> readFile(const std::string& filename);

	protected:


		vk::Device device;

		vk::Extent2D swapChainExtent;

		friend ComputePipeline;
	};

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
		

		struct ShaderStageOptions {
			std::string shaderPath;
			vk::ShaderStageFlagBits shaderStage;
		};

		std::vector<ShaderStageOptions> shaderStages;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		bool enablePushConstants = false;
		VkDeviceSize pushConstantSize = 0;
		vk::StencilFaceFlags pushConstantStages;




	};

}