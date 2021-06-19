#pragma once

#include "srpch.h"

#include "FileManager.h"

namespace sunrise {

	class FileSystem
	{
	public:

		static void initilize() {

			SR_CORE_TRACE("Checking if first launch");

			auto firstLaunchPath = FileManager::appConfigDir() + "first.launch";

			bool firstLaunch = !FileManager::exists(firstLaunchPath);
			
			if (firstLaunch) {
				SR_CORE_INFO("First Launch Detected - Performing First Time Setup");

				FileManager::saveStringToFile(
					"Delete this file to trigger all first time launch tasks on next program start", firstLaunchPath);


				FileManager::saveStringToFile(
					"c:/code/FSTS-World-Data/",
					FileManager::appConfigDir() + "directory.sunrise");


				// other first time setup here
				
				SR_CORE_INFO("First Time Stup Copmleted Successfully");
			}
			else {
				SR_CORE_TRACE("Skipping First Time Setup");


			}

		}

	private:

		FileSystem();

	};

}
