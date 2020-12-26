#include "srpch.h"
#include "Application.h"

namespace sunrise {

	Application::Application()
	{
	}

	Application::~Application()
	{
	}


	//  NO____APPLICTION


	NO_APPLICATION::NO_APPLICATION()
	{

	}

	NO_APPLICATION::~NO_APPLICATION()
	{
	}

	void NO_APPLICATION::startup()
	{
	}

	void NO_APPLICATION::run()
	{
	}

	void NO_APPLICATION::shutdown()
	{
	}

	const char* NO_APPLICATION::getName()
	{
		return nullptr;
	}


}