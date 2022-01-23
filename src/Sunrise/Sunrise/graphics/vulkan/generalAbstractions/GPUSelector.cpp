#include "srpch.h"
#include "GPUSelector.h"

namespace sunrise::gfx {


	bool QueueFamilyIndices::isComplete() {
		// resource transfer faqmily is not currently optional
		return graphicsFamily.has_value() && presentFamily.has_value() && resourceTransferFamily.has_value();
	}

	vk::PhysicalDevice GPUSelector::primaryGPU(vk::Instance instance, vk::SurfaceKHR surface)
	{
		PROFILE_FUNCTION;
		uint32_t deviceCount = 0;

		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		std::vector<VkPhysicalDevice> devices;
		devices.resize(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());


		SR_CORE_TRACE("Finding right gpu for a window, {} gpus available",deviceCount);

		for (uint32_t i = 0; i < deviceCount; i++)
		{
			auto device = vk::PhysicalDevice(devices.at(i));

			if (gpuSutable(device, surface)) {
				SR_CORE_TRACE("this gpu ({}) is sutable", device);
				return device;
			}
		}
		return vk::PhysicalDevice(devices[0]);
	}

	bool GPUSelector::gpuSutable(const vk::PhysicalDevice device, vk::SurfaceKHR surface)
	{
		PROFILE_FUNCTION;

		SR_CORE_TRACE("determining if this gpu ({}) is sutable",device);
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		// add check for required extensions here later

		auto families = gpuQueueFamilies(device, surface);

		bool swapChainAdequate = false;
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

		bool sutable = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader && families.isComplete() && swapChainAdequate;

		return sutable;
	}

	QueueFamilyIndices GPUSelector::gpuQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface)
	{
		PROFILE_FUNCTION;
			uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());



		QueueFamilyIndices indices;
        std::vector<uint32_t> backupTransferIndices;

        
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {


			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			// this shold be fixxed in the future but my graphcs cards allow presenting from the main graphics queue
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport) {
				indices.graphicsFamily = i;

				indices.presentFamily = i;
                
                if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                    backupTransferIndices.push_back(i);
                }
			}
			else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
				indices.resourceTransferFamily = i;
			}

            
            
			if (presentSupport) {
			}

			i++;
		}

        //TODO: make more robost and maybe chose didferent gueew from graphics
        if (!indices.resourceTransferFamily.has_value()) {
            indices.resourceTransferFamily = backupTransferIndices[0];
        }
        
		return indices;
	}

	SwapChainSupportDetails GPUSelector::querySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
		PROFILE_FUNCTION;
			SwapChainSupportDetails details;

		details.capabilities = device.getSurfaceCapabilitiesKHR(surface);

		uint32_t formatCount;
		device.getSurfaceFormatsKHR(surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			device.getSurfaceFormatsKHR(surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		device.getSurfacePresentModesKHR(surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			device.getSurfacePresentModesKHR(surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	vk::SurfaceFormatKHR GPUSelector::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
		PROFILE_FUNCTION;
			for (const auto& availableFormat : availableFormats) {
				if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
					return availableFormat;
				}
			}

		return availableFormats[0];
	}

	vk::PresentModeKHR GPUSelector::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
		PROFILE_FUNCTION;
			for (const auto& availablePresentMode : availablePresentModes) {
				if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
					return availablePresentMode;
				}
			}

		return vk::PresentModeKHR::eFifo;
	}

	VkExtent2D GPUSelector::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
		PROFILE_FUNCTION;
			if (capabilities.currentExtent.width != UINT32_MAX && capabilities.currentExtent.width != 0) {
				return capabilities.currentExtent;
			}
			else {
				int width, height;
				glfwGetFramebufferSize(window, &width, &height);

				VkExtent2D actualExtent = {
					static_cast<uint32_t>(width),
					static_cast<uint32_t>(height)
				};

				actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

				return actualExtent;
			}
	}

	vk::Format GPUSelector::findSupportedFormat(vk::PhysicalDevice phdevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
	{
		for (vk::Format format : candidates) {
			vk::FormatProperties props;
			phdevice.getFormatProperties(format, &props);
			//vkGetPhysicalDeviceFormatProperties(phdevice, format, &props);

			if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}


}
