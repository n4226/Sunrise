#pragma once

#include <srpch.h>


namespace sunrise::gfx {


	/// <summary>
	/// holds things like cpu staging buffers - cpu stored vulkan resources - vulkan resources not bound to a single device/renderer
	/// 
	/// nvermind - there has to be a staging buffer per gpu since vulkan ties vpu buffers to a device - so memory has to be coppied to each othe devicce's cpu staging buffer
	/// 
	/// </summary>
	class SharedRendererResources {



	};

}
