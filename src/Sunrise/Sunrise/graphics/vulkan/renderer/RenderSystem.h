#pragma once
#include "srpch.h"

namespace sunrise {

	class Window;
	class Scene;

	
	class System {
	public:
		//System(WorldScene* world);
		virtual ~System() = default;

		//WorldScene* world = nullptr;
		Scene* scene = nullptr;

		virtual void update();
		virtual void lateUpdate() {}
		virtual void setup() {}
		virtual void cleanup() {}

	protected:
		
			template<typename SceneType>
			SceneType* getScene();
	};

	template<typename SceneType>
	SceneType* sunrise::System::getScene()
	{
		SceneType *sc = dynamic_cast<SceneType *>(scene);
		if (sc != nullptr)
		{
			return sc;
		}
		else {
			throw std::logic_error("scene not the correct type");
		}
	}

	/// deprecated
	class RenderSystem : public System
	{
		/// <summary>
		/// called on the update thread
		/// requests the system to render its contents into a given render command buffer using a dispatchQueue and to return when it has finished
		/// system might be able to do this in parallel
		/// </summary>
		/// <param name="buffer"></param>
		virtual vk::CommandBuffer* renderSystem(uint32_t subpass, Window& window) = 0;
	};

}