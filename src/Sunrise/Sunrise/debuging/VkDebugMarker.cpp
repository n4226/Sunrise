#include "srpch.h"
#include "VkDebugMarker.h"

#include "Sunrise/Sunrise/core/Application.h"

#include "GFSDK_Aftermath.h"
#include "GFSDK_Aftermath_GpuCrashDump.h"

namespace sunrise::gfx {

	// implementatino heavly inspired by atrticle at: https://www.saschawillems.de/blog/2016/05/28/tutorial-on-using-vulkans-vk_ext_debug_marker-with-renderdoc/

	VkDebug::VkDebug(vk::Device device,Application* app)
	{
		vk::DynamicLoader dl;
		// This dispatch class will fetch function pointers for the passed device if possible, else for the passed instance
		vk::DispatchLoaderDynamic dldid(app->instance, dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"), device);

		pfnDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)dldid.vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
		pfnCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)dldid.vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
		pfnCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)dldid.vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
		pfnCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)dldid.vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");
	}

	void VkDebug::nameObject(vk::Device device,size_t handle, vk::DebugReportObjectTypeEXT objectType,const char* name)
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

	void VkDebug::insertMarker(vk::CommandBuffer  cmdBuff,const char* name, glm::vec4 color)
	{
		// Check for a valid function pointer
		if (pfnCmdDebugMarkerInsert) {
			VkDebugMarkerMarkerInfoEXT markerInfo = {};
			markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			memcpy(markerInfo.color, &color[0], sizeof(float) * 4);
			markerInfo.pMarkerName = "Set primary viewport";
			pfnCmdDebugMarkerInsert(static_cast<VkCommandBuffer>(cmdBuff), &markerInfo);
		}
	}

	void VkDebug::beginRegion(vk::CommandBuffer cmdBuff,const char* name, glm::vec4 color)
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

	void VkDebug::endRegion(vk::CommandBuffer cmdBuff)
	{
		// Check for a valid function pointer
		if (pfnCmdDebugMarkerEnd) {
			pfnCmdDebugMarkerEnd(static_cast<VkCommandBuffer>(cmdBuff));
		}
	}


	// Static callback wrapper for OnCrashDump
	void VkDebug::GpuCrashDumpCallback(
		const void* pGpuCrashDump,
		const uint32_t gpuCrashDumpSize,
		void* pUserData)
	{
		VkDebug* pGpuCrashTracker = reinterpret_cast<VkDebug*>(pUserData);
		pGpuCrashTracker->OnCrashDump(pGpuCrashDump, gpuCrashDumpSize);
	}

	// Handler for GPU crash dump callbacks from Nsight Aftermath
	void VkDebug::OnCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize)
	{

		
	}


	void VkDebug::initAftermath()
	{
		GFSDK_Aftermath_EnableGpuCrashDumps(
			GFSDK_Aftermath_Version_API,
			GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan, 0, , 0, 0, this);
	}




}