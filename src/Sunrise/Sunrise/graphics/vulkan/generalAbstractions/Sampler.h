#pragma once

#include "srpch.h"

namespace sunrise::gfx {


	class SUNRISE_API Sampler
	{
	public:

		struct CreateOptions
		{
			vk::Filter magFilter = vk::Filter::eLinear;
			vk::Filter minFilter = vk::Filter::eLinear;

			void setFilterAll(vk::Filter filter);

			vk::SamplerAddressMode addressModeU = vk::SamplerAddressMode::eRepeat;
			vk::SamplerAddressMode addressModeV = vk::SamplerAddressMode::eRepeat;
			vk::SamplerAddressMode addressModeW = vk::SamplerAddressMode::eRepeat;

			void setAdressModeAll(vk::SamplerAddressMode addressMode);

			bool enableAnisotropy = true;
			//TODO: fix defualt value for non suporting gpus
			float maxAnisotropy = 16;

			vk::BorderColor borderColor = vk::BorderColor::eFloatOpaqueBlack;

			bool unnormalizedCoordinates = false;

			vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eLinear;
			float mipLodBias = 0.f;
			float minLod = 0.f;
			float maxLod = 0.f;

		};


		Sampler(vk::Device device, CreateOptions& options);
		~Sampler();

		vk::Sampler vkItem;

	private:

		vk::Device device;

	};

}