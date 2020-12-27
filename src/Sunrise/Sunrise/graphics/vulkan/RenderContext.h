#pragma once

#include "srpch.h"
#include "generalAbstractions/GPUSelector.h"

namespace sunrise::gfx {

	class SUNRISE_API RenderContext
	{
	public:
		
		vk::Instance instance;


		std::vector<Window*> windows;

		// - one per each device - shared indicies
		std::vector<vk::Device         >          devices;
		std::vector<vk::PhysicalDevice >          physicalDevices;
		std::vector<Renderer*          >          renderers;
		std::vector<VmaAllocator       >          allocators;
		std::vector<GPUQueues          >          deviceQueues;
		std::vector<QueueFamilyIndices >          queueFamilyIndices;
		//

		size_t currentFrame = 0;

	};

}