#include "srpch.h"
#include "UIScene.h"


namespace sunrise {

	UIScene::UIScene(Application* app)
		: Scene(app)
	{
		coordinator = new UISceneRenderCoordinator(this);
	}

}