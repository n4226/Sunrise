#pragma once

#include "core.h"
#include "../graphics/vulkan/RenderContext.h"

namespace sunrise {

	class Scene;

	class SUNRISE_API Application: public gfx::RenderContext
	{
	public:
		Application(Scene* initialScene);
		virtual ~Application();

		virtual void startup();

		/// <summary>
		/// should only be overiden for applicatons without windows
		/// </summary>
		virtual void run();
		virtual void shutdown();

		/// <summary>
		/// Warning: This is called before the engine is completely initilized. Do not reference any engine provided functions or varibles as their behavure is undefined. 
		/// </summary>
		/// <returns></returns>
		virtual const char* getName() = 0;


		// scene api

		void loadScene(Scene* scene, void* animationProperties);



		/// <summary>
		/// normally just one scene
		/// </summary>
		std::vector<Scene*> loadedScenes;

		marl::Scheduler* scheduler;


		void quit();

	protected:
		
		virtual bool wantsWindows();

		void createWindows();

		void createInstance() override;

		bool createRenderer(size_t deviceIndex) override;
		size_t createDevice(size_t window) override;
		void createAllocator(size_t deviceIndex) override;

		void runLoop() override;
		bool shouldLoop() override;
		void runLoopIteration() override;

		static VkBool32 debugCallbackFunc(
			VkDebugReportFlagsEXT                       flags,
			VkDebugReportObjectTypeEXT                  objectType,
			uint64_t                                    object,
			size_t                                      location,
			int32_t                                     messageCode,
			const char* pLayerPrefix,
			const char* pMessage,
			void* pUserData);


		VkDebugReportCallbackEXT debugObject;

	};

	class SUNRISE_API NO_APPLICATION: public Application
	{
	public:
		NO_APPLICATION();
		~NO_APPLICATION();


		 void startup() override;
		 //void run() override;
		 void shutdown() override;

		 const char* getName() override;


	private:

	};


	// Defined by client
	Application* CreateApplication();


}
