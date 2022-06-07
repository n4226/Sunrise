#pragma once

#include "srpch.h"
#include "../WorldScene.h"


namespace sunrise {
	
	class OnboardWorldBGScene: public WorldScene
	{
	public:

		using WorldScene::WorldScene;

		void load() override;


		void onDrawUI() override;


		void update() override;

	protected:
	private:
	};

}
