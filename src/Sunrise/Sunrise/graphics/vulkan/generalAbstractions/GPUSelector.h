#pragma once

#include "pch.h"
#include <GLFW/glfw3.h>
#include <optional>

namespace sunrise::gfx {


	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> resourceTransferFamily;

		bool isComplete();

	};

	struct GPUQueues {
		vk::Queue graphics;
		vk::Queue resourceTransfer;
		vk::Queue presentation;
	};

	class GPUSelector
	{
	public:
		static vk::PhysicalDevice primaryGPU(vk::Instance instance, vk::SurfaceKHR surface);

		static QueueFamilyIndices gpuQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);

		static SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface);

		static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

		static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

		static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

		static vk::Format findSupportedFormat(vk::PhysicalDevice phdevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	private:
		static bool gpuSutable(const vk::PhysicalDevice device, vk::SurfaceKHR surface);

	};



}
