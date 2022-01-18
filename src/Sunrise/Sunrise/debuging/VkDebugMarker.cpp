#include "srpch.h"
#include "VkDebugMarker.h"

#include "Sunrise/Sunrise/core/Application.h"
#include "Sunrise/Sunrise/fileSystem/FileManager.h"

#ifdef SR_USE_AFTERMATH
#include "GFSDK_Aftermath.h"
#include "GFSDK_Aftermath_GpuCrashDump.h"
#endif

namespace sunrise::gfx {

	// implementatino heavly inspired by atrticle at: https://www.saschawillems.de/blog/2016/05/28/tutorial-on-using-vulkans-vk_ext_debug_marker-with-renderdoc/

	VkDebug::VkDebug(vk::Device device,Application* app)
		: device(device)
	{
		vk::DynamicLoader dl;
		// This dispatch class will fetch function pointers for the passed device if possible, else for the passed instance
		vk::DispatchLoaderDynamic dldid(app->instance, dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"), device);

		pfnDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)dldid.vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
		pfnCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)dldid.vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
		pfnCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)dldid.vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
		pfnCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)dldid.vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");
	}

	void VkDebug::nameObject(size_t handle, vk::DebugReportObjectTypeEXT objectType,const char* name) const
	{
		// Check for a valid function pointer
		if (pfnDebugMarkerSetObjectName)
		{
			VkDebugMarkerObjectNameInfoEXT nameInfo = {};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
			nameInfo.objectType = static_cast<VkDebugReportObjectTypeEXT>(objectType);
			nameInfo.object = handle;
			nameInfo.pObjectName = name;
			pfnDebugMarkerSetObjectName(device,&nameInfo);
		}
	}

	void VkDebug::insertMarker(vk::CommandBuffer  cmdBuff,const char* name, glm::vec4 color) const
	{
		// Check for a valid function pointer
		if (pfnCmdDebugMarkerInsert) {
			VkDebugMarkerMarkerInfoEXT markerInfo = {};
			markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			memcpy(markerInfo.color, &color[0], sizeof(float) * 4);
			markerInfo.pMarkerName = name;
			pfnCmdDebugMarkerInsert(static_cast<VkCommandBuffer>(cmdBuff), &markerInfo);
		}
	}

	void VkDebug::beginRegion(vk::CommandBuffer cmdBuff,const char* name, glm::vec4 color) const
	{
		// Check for a valid function pointer
		if (pfnCmdDebugMarkerBegin) {
			VkDebugMarkerMarkerInfoEXT markerInfo = {};
			markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			memcpy(markerInfo.color, &color, sizeof(color));
			markerInfo.pMarkerName = name;
			pfnCmdDebugMarkerBegin(static_cast<VkCommandBuffer>(cmdBuff), &markerInfo);
		}
	}

	void VkDebug::endRegion(vk::CommandBuffer cmdBuff) const
	{
		// Check for a valid function pointer
		if (pfnCmdDebugMarkerEnd) {
			pfnCmdDebugMarkerEnd(static_cast<VkCommandBuffer>(cmdBuff));
		}
	}


	// Static callback wrapper for OnCrashDump
	void VkDebug::gpuCrashDumpCallback(
		const void* pGpuCrashDump,
		const uint32_t gpuCrashDumpSize,
		void* pUserData)
	{
		VkDebug* pGpuCrashTracker = reinterpret_cast<VkDebug*>(pUserData);
		pGpuCrashTracker->onCrashDump(pGpuCrashDump, gpuCrashDumpSize);
	}

	// Handler for GPU crash dump callbacks from Nsight Aftermath
	void VkDebug::onCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize)
	{

		SR_CORE_WARN("GPU Crash, writting it to disk");

		FileManager::saveBinaryToFile(pGpuCrashDump, gpuCrashDumpSize, FileManager::appWokringDir() + "/crashDumps/latest.nv-gpudmp");
		
	}


	void VkDebug::initAftermath()
	{
#ifdef SR_USE_AFTERMATH
		// after this call this object can not be copied or moved TODO: fix this by unregistering and re registring
		GFSDK_Aftermath_EnableGpuCrashDumps(
			GFSDK_Aftermath_Version_API,
			GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan, 0, gpuCrashDumpCallback, 0, 0, this);

		aftermathActive = true;
#endif
	}




}
