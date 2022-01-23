#pragma once

#include "core.h"
#include "../graphics/vulkan/RenderContext.h"
#include "input/InputHolder.h"

#define USE_ASIO
// see: https://stackoverflow.com/questions/9750344/boostasio-winsock-and-winsock-2-compatibility-issue
#ifdef _WIN32
#  ifdef USE_ASIO
//     Set the proper SDK version before including boost/Asio
#      include <SDKDDKVer.h>
//     Note boost/ASIO includes Windows.h. 
#      include <asio.hpp>
#   else //  USE_ASIO
#      include <Windows.h>
#   endif //  USE_ASIO
#else // _WIN32
#  ifdef USE_ASIO
#     include <asio.hpp>
#  endif // USE_ASIO
#endif //_WIN32

//namespace asio {
//	class io_context;
//	class any_io_executor;
//}

namespace sunrise {

	class Scene;

	class SUNRISE_API Application: public gfx::RenderContext, public InputHolder
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
		void unloadScene(Scene* scene);

		void hotReloadScene();

		/// <summary>
		/// normally just one scene
		/// </summary>
		std::vector<Scene*> loadedScenes;

		marl::Scheduler* scheduler{};
		/// <summary>
		/// will remaine nullptr if config contrext thread is disabled
		/// </summary>
		asio::io_context* context = nullptr;
		std::thread* contextThread;

		/// <summary>
		/// triggers context to finish when all registered tasks are done
		/// does not force stop or wait for it to stop
		/// </summary>
		void stopASIOContext();
		void forceStopASIOContext();

		bool getKey(int key) override;

		void quit();

		static void setDefaultTheme();

		bool imguiValid();

        void startProfileSession();
        bool isProfiling();
        void endProfileSession();
        
    protected:
        bool profiling = false;
		
		void configureGLFWEvents();

		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		
		static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
		static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

		/// <summary>
		/// used to prevent context from exiting run until disered
		/// </summary>
		asio::any_io_executor* contextWork = 0;

		struct ApplicationConfig {
			/// <summary>
			/// the number to subtract from the hardware cores e.g if cpu has 4 cores and this is 1 marl will have acces to 3 treads
			/// default is 2, 1 for main thread and 1 for asio context
			/// </summary>
			size_t marlThreadCountOffset = 2;
			bool enableMarl = true;

			bool enableAsioContext = true;
			bool enableAsioContextThread = true;

			bool vulkan = true;
			bool wantsWindows = true;
			bool useFileSys = true;

			//MARK: UI
			bool drawAppDomainUI = true;
			std::function<void()> addToMainMenu = []() {};
			std::function<void()> addToFileMainMenu = []() {};
			std::function<void()> setAppTheme = Application::setDefaultTheme;
		};
		ApplicationConfig config;

		//depricated use configure() instead
		//virtual bool wantsWindows();

		// overidden by subclass to define important 
		virtual ApplicationConfig configure() {return {};}

		void createWindows();

		void createInstance() override;

		bool createRenderer(size_t deviceIndex) override;
		size_t createDevice(size_t window) override;
		void createAllocator(size_t deviceIndex) override;

		void runLoop() override;
		bool shouldLoop() override;
		void runLoopIteration() override;
			
		void drawMainUI();

		bool _imguiValid = false;

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
