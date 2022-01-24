#pragma once

#include "srpch.h"
#include "Sunrise/scene/Scene.h"
#include "UISceneRenderCoordinator.h"

namespace sunrise {


	class UIScene : public Scene
	{
	public:
		UIScene(Application* app);

		virtual void load() override {};

		virtual void onDrawUI()       override {};
		virtual void onDrawMainMenu() override {};

		virtual void unload() override {};

	};

}