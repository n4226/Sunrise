#pragma once

#include "srpch.h"

namespace sunrise {


	class SUNRISE_API Scene
	{
	public:
		Scene();
		virtual ~Scene();

		virtual void load() = 0;

		virtual void update();

		virtual void unload() = 0;


		double time = 0;
		float timef = 0;

		double deltaTime = 0;
		float  deltaTimef = 0;

		size_t frameNum = 0;

	};

}