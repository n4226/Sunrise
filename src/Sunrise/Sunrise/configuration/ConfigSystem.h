#pragma once

#include "srpch.h"

namespace sunrise {

	class ConfigSystem
	{
	public:
		struct Config
		{
			nlohmann::json toJson();
			static Config* fromJson(nlohmann::json& j);

			struct Window {

				enum WindowMode : int
				{
					windowed, FullscreenBorderless, Fullscreen
				};

				std::string monitor;
				WindowMode mode;
				size_t group;
				glm::ivec2 size;
				glm::vec2 monitorLocalPostion; // in screen space coords 0,0 top left of screen and 1,1 bottom right
			};

			struct Camera {
				glm::vec3 offset;
				glm::vec3 rotAxis;
				glm::float32 rotAngleDeg;
			};

			std::vector<Window> windows;
			std::vector<Camera> cameras;
		};

		Config& global();

		void readFromDisk();
		void writeToDisk();
		void resetToDefault();

		void writeHelpDoc();

	private:
		Config* config;
	};

	extern ConfigSystem configSystem;

}
