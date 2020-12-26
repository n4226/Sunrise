#pragma once

#include "core.h"

namespace sunrise {

	class SUNRISE_API Application
	{
	public:
		Application();
		virtual ~Application();

		virtual void startup() = 0;
		//virtual void run() = 0;
		virtual void shutdown() = 0;

		virtual const char* getName() = 0;

	private:

	};

	class SUNRISE_API NO_APPLICATION: public Application
	{
	public:
		NO_APPLICATION();
		~NO_APPLICATION();


		 void startup() override;
		 void run() override;
		 void shutdown() override;

		 const char* getName() override;

	private:

	};


	// Defined by client
	Application* CreateApplication();


}
