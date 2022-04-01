#pragma once

#include "srpch.h"
#include <GLFW/glfw3.h>

namespace sunrise {
	class Window;
}

namespace sunrise::gfx {



	struct SUNRISE_API SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	struct SUNRISE_API QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> resourceTransferFamily;

		bool isComplete();

	};

	struct SUNRISE_API GPUQueues {
		vk::Queue graphics;
		vk::Queue resourceTransfer;
		vk::Queue presentation;
	};

	class SUNRISE_API GPUSelector
	{
	public:
		static vk::PhysicalDevice primaryGPU(vk::Instance instance, const Window* window);

		static QueueFamilyIndices gpuQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);

		static SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface);

		static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

		static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

		static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

		static vk::Format findSupportedFormat(vk::PhysicalDevice phdevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	private:
		static bool gpuSutable(const vk::PhysicalDevice device,const Window* window);

		static bool WindowsVarifiedCorrectGPUForMonitor(vk::PhysicalDevice device, const Window* window);
	};



}
