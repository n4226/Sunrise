#include "srpch.h"
#include "AttributeHelpers.h"

namespace sunrise::gfx {


	/// <summary>
	/// think of binding as buffer
	/// </summary>
	/// <param name="index"></param>
	/// <param name="stride"></param>
	/// <param name="inputRate"></param>
	/// <returns></returns>
	VkVertexInputBindingDescription makeVertBinding(uint32_t index, uint32_t stride, vk::VertexInputRate inputRate)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = index;
		bindingDescription.stride = stride;
		bindingDescription.inputRate = VkVertexInputRate(inputRate);

		return bindingDescription;
	}

	/// <summary>
	/// think of layout as a property
	/// </summary>
	/// <param name="binding"></param>
	/// <param name="location"></param>
	/// <param name="format"></param>
	/// <param name="offset"></param>
	/// <returns></returns>
	VkVertexInputAttributeDescription makeVertAttribute(uint32_t binding, uint32_t location, VertexAttributeFormat format, uint32_t offset)
	{
		return vk::VertexInputAttributeDescription(location, binding, vk::Format(format), offset);
	}

}