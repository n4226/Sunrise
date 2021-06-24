#include "srpch.h"
#include "FileManager.h"

//for getting working dir
#ifdef SR_PLATFORM_WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
//

namespace sunrise {

	std::string FileManager::_baseDir = "";


	//FileManager filemanager;

	std::string FileManager::baseEngineResourceDir()
	{
		if (_baseDir == "")
			fetchBaseEngineResourceDir();

		return _baseDir;
	}

	std::string FileManager::engineTerrainChunkDir()
	{
		return baseEngineResourceDir() + "terrain/chunkMeshes/";
	}

	std::string FileManager::engineTerrainChunkAttributesDir()
	{
		return baseEngineResourceDir() + "terrain/chunkAttributes/";
	}

	std::string FileManager::engineMaterialDir()
	{
		return baseEngineResourceDir() + "materials/";
	}

	std::string FileManager::engineConfigDir()
	{
		return baseEngineResourceDir() + "config/";
	}

	std::string FileManager::appWokringDir()
	{
		char buff[FILENAME_MAX]; //create string buffer to hold path
		GetCurrentDir(buff, FILENAME_MAX);
		std::string current_working_dir(buff);
		return current_working_dir;
	}


	std::string FileManager::appConfigDir()
	{
		return "config/";
	}

	bool FileManager::exists(const std::string& path)
	{
		return std::filesystem::exists(path);
	}

	void FileManager::saveStringToFile(std::string contents, std::string& const path)
	{
		std::ofstream out(path);
		out << contents;
		out.close();
	}

	void FileManager::saveBinaryToFile(void* data, size_t length, const std::string& path)
	{
		std::ofstream out(path, std::ios::out | std::ios::binary);
		out.write(static_cast<const char*>(data), length);
		out.close();
	}

	//TODO: make this faster
	std::string FileManager::loadStringfromFile(std::string& const path)
	{
		std::ifstream t(path);
		if (!t.is_open())
			throw FileNotFoundError();
		std::string str((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
		return str;
	}


	void FileManager::fetchBaseEngineResourceDir()
	{
		std::string sunriseDirFolder = appConfigDir() + "directory.sunrise";
		try {
			auto str = loadStringfromFile(sunriseDirFolder);
			_baseDir = std::move(str);
		}
		catch (...) {
			throw SRConfigLocFileNotFoundError();
		}

	}

}