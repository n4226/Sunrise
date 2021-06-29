#pragma once

#include "srpch.h"

namespace sunrise {

	class Application;

	namespace gfx {

		//TODO: make this work with multi gpu
		class VkDebug
		{
		public:
			VkDebug(vk::Device device, Application* app);

			// debug report api
			void nameObject(size_t handle, vk::DebugReportObjectTypeEXT objectType, const char* name) const;
			//static void tagObject(size_t handle, char* name);

			void insertMarker(vk::CommandBuffer cmdBuff, const char* name, glm::vec4 color) const;
			void beginRegion(vk::CommandBuffer cmdBuff, const char* name, glm::vec4 color) const;
			void endRegion(vk::CommandBuffer cmdBuff) const;
			bool debugActive;
			// end debug report api

			//aftermath api
			void initAftermath();
			bool aftermathActive = false;
		private:


			PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName;
			PFN_vkCmdDebugMarkerBeginEXT pfnCmdDebugMarkerBegin;
			PFN_vkCmdDebugMarkerEndEXT pfnCmdDebugMarkerEnd;
			PFN_vkCmdDebugMarkerInsertEXT pfnCmdDebugMarkerInsert;

			// aftermath

			static void VkDebug::gpuCrashDumpCallback(
				const void* pGpuCrashDump,
				const uint32_t gpuCrashDumpSize,
				void* pUserData);

			void VkDebug::onCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize);

			vk::Device device;
		};

	}
}