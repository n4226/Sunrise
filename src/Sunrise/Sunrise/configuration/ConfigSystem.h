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
				glm::ivec2 size;
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