#pragma once

#include "srpch.h"

#include "core.h"

//#include "Application.h"

namespace Sunrise {

	class Application;

	class SUNRISE_API Engine
	{
	public:
		Engine(Application* app);
		~Engine();

		void startup();

		Application* app;

	};

	extern SUNRISE_API Engine* engine;

}