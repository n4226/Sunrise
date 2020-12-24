#pragma once

#include "core.h"

#include "Log.h"

#ifdef SR_PLATFORM_WINDOWS

extern Sunrise::Application* Sunrise::CreateApplication();

int main(int arc, char** argv) {

	auto app = Sunrise::CreateApplication();

	Sunrise::engine = new Sunrise::Engine(app);

	Sunrise::engine->startup();

	delete Sunrise::engine;
}

#endif 
