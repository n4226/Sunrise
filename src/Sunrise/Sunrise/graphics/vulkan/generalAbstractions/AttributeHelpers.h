#pragma once

#include "srpch.h"

namespace sunrise::gfx {

	// Vertex description and attribute helpers

	enum class VertexAttributeFormat
	{
		vec1 = VK_FORMAT_R32_SFLOAT,
		vec2 = VK_FORMAT_R32G32_SFLOAT,
		vec3 = VK_FORMAT_R32G32B32_SFLOAT,
		vec4 = VK_FORMAT_R32G32B32A32_SFLOAT,
	};


	VkVertexInputBindingDescription makeVertBinding(uint32_t index, uint32_t stride, vk::VertexInputRate inputRate = vk::VertexInputRate::eVertex);

	/// <summary>
	///
	/// The format parameter describes the type of data for the attribute.A bit confusingly, the formats are specified using the same enumeration as color formats.The following shader typesand formats are commonly used together :
	///
	/// float : VK_FORMAT_R32_SFLOAT
	/// vec2 : VK_FORMAT_R32G32_SFLOAT
	/// vec3 : VK_FORMAT_R32G32B32_SFLOAT
	/// vec4 : VK_FORMAT_R32G32B32A32_SFLOAT
	/// </summary>
	/// <returns></returns>
	VkVertexInputAttributeDescription makeVertAttribute(uint32_t binding, uint32_t location, VertexAttributeFormat format, uint32_t offset);


}