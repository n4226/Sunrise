#pragma once

#include "srpch.h"

namespace sunrise {

	class Application;

	namespace gfx {

		class VkDebug
		{
		public:

			static void init(vk::Device device, Application* app);

			static void nameObject(vk::Device device, size_t handle, vk::DebugReportObjectTypeEXT objectType, const char* name);
			//static void tagObject(size_t handle, char* name);

			static void insertMarker(vk::CommandBuffer cmdBuff, const char* name, glm::vec4 color);
			static void beginRegion(vk::CommandBuffer cmdBuff, const char* name, glm::vec4 color);
			static void endRegion(vk::CommandBuffer cmdBuff);

			static bool active;
		private:

			static PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName;
			static PFN_vkCmdDebugMarkerBeginEXT pfnCmdDebugMarkerBegin;
			static PFN_vkCmdDebugMarkerEndEXT pfnCmdDebugMarkerEnd;
			static PFN_vkCmdDebugMarkerInsertEXT pfnCmdDebugMarkerInsert;
		};

	}
}