#include "srpch.h"
#include "GPUSelector.h"

#include "Sunrise/core/Window.h"

#include "dxgi.h"
#include <GLFW/glfw3.h>

#include <locale>
#include <codecvt>

#pragma comment(lib, "d3d11.lib")
//#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "dxgi.lib")
//#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace sunrise::gfx {


	bool QueueFamilyIndices::isComplete() {
		// resource transfer faqmily is not currently optional
		return graphicsFamily.has_value() && presentFamily.has_value() && resourceTransferFamily.has_value();
	}

	vk::PhysicalDevice GPUSelector::primaryGPU(vk::Instance instance, const Window* window)
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

			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(devices.at(i), &props);

			SR_CORE_TRACE("gpu {} is a {}, id: ({})", i, props.deviceName, props.deviceID);
		}

		for (uint32_t i = 0; i < deviceCount; i++)
		{
			auto device = vk::PhysicalDevice(devices.at(i));
			
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(devices.at(i),&props);

			SR_CORE_TRACE("checking if {} is sutable, location: ({})", props.deviceName, device);

			if (gpuSutable(device, window)) {
				SR_CORE_TRACE("this gpu, {} ({}) is sutable", props.deviceName, device);
				return device;
			}
		}
		return vk::PhysicalDevice(devices[0]);
	}

	bool GPUSelector::gpuSutable(const vk::PhysicalDevice device, const Window* window)
	{
		PROFILE_FUNCTION;

		SR_CORE_TRACE("determining if this gpu ({}) is sutable", device);
		VkPhysicalDeviceProperties2 deviceProperties = vk::PhysicalDeviceProperties2();
		VkPhysicalDeviceIDProperties deviceIds = vk::PhysicalDeviceIDProperties();
		deviceProperties.pNext = &deviceIds;
		vkGetPhysicalDeviceProperties2(device, &deviceProperties);
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		// add check for required extensions here later

		auto families = gpuQueueFamilies(device, window->surface);

		bool swapChainAdequate = false;
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, window->surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

		bool sutable = deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader && families.isComplete() && swapChainAdequate;


		//TODO: support failback for other platforms
		//have to determine if this gpu is directly connected to the monitor displaying the surface - need to discurage user from dragging windows accross windows whuich are on different gpus

		/*steps - 
		*/
	

		bool directConnection = WindowsVarifiedCorrectGPUForMonitor(device, window);



		//TODO: impliment rank system for gpu to make sure if no descrete gpu is connected to monitor direclty it still works and other situations
		return sutable && directConnection;
	}


	bool GPUSelector::WindowsVarifiedCorrectGPUForMonitor(vk::PhysicalDevice device, const Window* window)
	{
		VkPhysicalDeviceProperties2 deviceProperties = vk::PhysicalDeviceProperties2();
		VkPhysicalDeviceIDProperties deviceIds = vk::PhysicalDeviceIDProperties();
		deviceProperties.pNext = &deviceIds;
		vkGetPhysicalDeviceProperties2(device, &deviceProperties);

		auto vkWindowHWMD = glfwGetWin32Window(window->window);
		auto vKMintor = MonitorFromWindow(vkWindowHWMD, MONITOR_DEFAULTTONULL);

		LUID vkDeviceLUID = *(PLUID)deviceIds.deviceLUID;


		IDXGIFactory* pFactory = NULL;

		CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&pFactory);

		UINT i = 0;
		IDXGIAdapter* pAdapter;
		std::vector <IDXGIAdapter*> vAdapters;
		while (pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			vAdapters.push_back(pAdapter);
			++i;
		}

		//TODO: stop memory leak of dx objects - use defer
		//defer(xxxxx)

		for (UINT i = 0; i < vAdapters.size(); i++) {


			UINT j = 0;
			IDXGIOutput* pOutput;
			while (vAdapters[i]->EnumOutputs(j, &pOutput) != DXGI_ERROR_NOT_FOUND)
			{
				DXGI_OUTPUT_DESC des;
				pOutput->GetDesc(&des);

				//need to check if this monitor is the one for the requested window 
				//- than need to check if this device is the vk one to varify the gpu is correct f or window
				auto correct = des.Monitor == vKMintor;
				if (correct) {
#if SR_LOGGING
					std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
					std::string narrow = converter.to_bytes(des.DeviceName);
					SR_CORE_TRACE("{}, is correct mointor: {}", narrow, correct);
#endif

					DXGI_ADAPTER_DESC gpuDes;
					vAdapters[i]->GetDesc(&gpuDes);

					auto deviceLUID = gpuDes.AdapterLuid;

					if (vkDeviceLUID.LowPart == deviceLUID.LowPart && vkDeviceLUID.HighPart == deviceLUID.HighPart) {
						SR_CORE_TRACE("found matching gpu, {}", deviceProperties.properties.deviceName);
						return true;
					}



				}

				++j;
			}
		}
		return false;
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
