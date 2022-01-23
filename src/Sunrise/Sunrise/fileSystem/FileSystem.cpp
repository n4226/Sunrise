#include "srpch.h"
#include "FileSystem.h"

void sunrise::FileSystem::initilize() {

    SR_CORE_TRACE("About to launch file system with working directory of {}", FileManager::appWokringDir());
    
	SR_CORE_TRACE("Checking if first launch");

	auto firstLaunchPath = FileManager::appConfigDir() + "first.launch";

	bool firstLaunch = !FileManager::exists(firstLaunchPath);

	if (firstLaunch) {
		SR_CORE_INFO("First Launch Detected - Performing First Time Setup");

		FileManager::saveStringToFile(
			"Delete this file to trigger all first time launch tasks on next program start", firstLaunchPath);

        
#ifdef SR_PLATFORM_WINDOWS
        auto defaultSRPath = "c:/Sunrise-World-Data/";
#elif defined(SR_PLATFORM_MACOS)
        auto defaultSRPath = "~/Sunrise-World-Data/";
#else
#error filesystem for this platform not setup
#endif

		FileManager::saveStringToFile(
			defaultSRPath,
			FileManager::appConfigDir() + "directory.sunrise");


		// other first time setup here

        //SR_CORE_ASSERT(FileManager::exists(FileManager::engineConfigDir() + "/global.cfg")); // engine not installed corretly
        
		SR_CORE_INFO("First Time Stup Copmleted Successfully");
	}
	else {
		SR_CORE_TRACE("Skipping First Time Setup");


	}

}
