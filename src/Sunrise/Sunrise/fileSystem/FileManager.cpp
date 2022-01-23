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

	std::vector<std::string> FileManager::listDirContents(const std::string& path, bool includeSubDirectories,bool filesOnly)
	{
		std::vector<std::string> results;

		if (includeSubDirectories)
			for (auto& path : std::filesystem::recursive_directory_iterator(path)) {
				if (!filesOnly || (filesOnly && path.is_regular_file()))
					results.push_back(path.path().generic_string());
			}
		else
			for (auto path : std::filesystem::directory_iterator(path)) {
				if (!filesOnly || (filesOnly && path.is_regular_file()))
					results.push_back(path.path().generic_string());
			}
		return results;
	}

	bool FileManager::createIntermediateDirs(const std::string& path)
	{
		return std::filesystem::create_directories(std::filesystem::path(path).parent_path());
	}

	void FileManager::saveStringToFile(const std::string& contents, const std::string&  path)
	{
		createIntermediateDirs(path);
		std::ofstream out(path);
		out << contents;
		out.close();
	}

	void FileManager::saveBinaryToFile(const void* data, size_t length, const std::string& path)
	{
		createIntermediateDirs(path);
		std::ofstream out(path, std::ios::out | std::ios::binary);
		out.write(static_cast<const char*>(data), length);
		out.close();
	}

	//TODO: make this faster
	std::string FileManager::loadStringfromFile(const std::string& path)
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
            std::string end = "\n";
//            if (std::find_end(str.begin(), str.end(), end.begin(), end.end()))
            if (str[str.size() - 1] == '\n') {
                str.resize(str.size() - 1);
            }
			_baseDir = std::move(str);
		}
		catch (...) {
			throw SRConfigLocFileNotFoundError();
		}

	}

}
