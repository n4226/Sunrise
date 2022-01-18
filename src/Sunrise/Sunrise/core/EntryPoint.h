#pragma once

#include "core.h"

#include "Log.h"

extern sunrise::Application* sunrise::CreateApplication();

//#ifdef SR_PLATFORM_WINDOWS

namespace sunrise {

	void runEngine() {
		auto app = sunrise::CreateApplication();

		sunrise::engine = new sunrise::Engine(app);

		sunrise::engine->startup();
	}

	void stopEngine() {

		delete sunrise::engine;
	}

}

// for programs such as dlls which are not in control of their main function allow them to call runEngine on their own
#ifndef SR_NO_AUTO_ENTRY
int main(int arc, char** argv) {

	sunrise::runEngine();
	sunrise::stopEngine();

	return 0;
}
#endif

//#endif
