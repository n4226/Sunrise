#include "srpch.h"
#include "UIScene.h"


namespace sunrise {

	UIScene::UIScene(Application* app)
		: Scene(app)
	{
		coordinatorCreator = ([this](auto renderer) {
			return new UISceneRenderCoordinator(this,renderer);
		});
	}

}