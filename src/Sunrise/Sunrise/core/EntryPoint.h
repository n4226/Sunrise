#pragma once

#include "core.h"

#include "Log.h"

#ifdef SR_PLATFORM_WINDOWS

extern sunrise::Application* sunrise::CreateApplication();

int main(int arc, char** argv) {

	auto app = sunrise::CreateApplication();

	sunrise::engine = new sunrise::Engine(app);

	sunrise::engine->startup();

	delete sunrise::engine;
}

#endif 
