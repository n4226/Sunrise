#pragma once

#include "core.h"

namespace Sunrise {

	class SUNRISE_API Application
	{
	public:
		Application();
		virtual ~Application();

		virtual const char* getName() = 0;

	private:

	};

	// Defined by client
	Application* CreateApplication();

}
