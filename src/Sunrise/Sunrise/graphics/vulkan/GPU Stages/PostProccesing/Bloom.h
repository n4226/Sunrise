#pragma once

#include "srpch.h"

namespace sunrise::gfx {

	class Bloom
	{
	public:

		struct Options 
		{
			float threshold = 1;
			float knee = 0.1;

			float upsampleScale = 1;
			float intensity = 1;

			//dirt tex
			float DirtIntensity = 1;
			void* dirtTex = nullptr;
		};

	protected:
	private:
	};

}

