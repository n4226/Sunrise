#pragma once

#include "srpch.h"
#include "Sunrise/scene/Scene.h"

namespace sunrise {

	/// <summary>
	/// NOT WORKING YET
	/// </summary>
	class UIScene : public Scene
	{
	public:
		using Scene::Scene;

		virtual void load() override {};

		virtual void onDrawUI()       override {  };
		virtual void onDrawMainMenu() override {};

		virtual void unload() override {};

	};

}