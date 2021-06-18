#pragma once

#include "srpch.h"

#include "core.h"

//#include "Application.h"

namespace sunrise {

	class Application;

	class SUNRISE_API Engine
	{
	public:
		Engine(Application* app);
		~Engine();

		void startup();

		void run();

		void shutdown();

		Application* app;
		

	private:

		friend Application;

		libguarded::plain_guarded<bool,std::mutex> shouldQuit;
	};

	extern SUNRISE_API Engine* engine;

}