#include "srpch.h"
#include "FileManager.h"


namespace sunrise {

	std::string FileManager::_baseDir = "";


	//FileManager filemanager;

	std::string FileManager::getBaseDir()
	{
		if (_baseDir == "")
			fetchBaseDir();

		return _baseDir;
	}

	std::string FileManager::getTerrainChunkDir()
	{
		return getBaseDir() + "terrain/chunkMeshes/";
	}

	std::string FileManager::getTerrainChunkAttributesDir()
	{
		return getBaseDir() + "terrain/chunkAttributes/";
	}

	std::string FileManager::getMaterialDir()
	{
		return getBaseDir() + "materials/";
	}

	std::string FileManager::getConfigDir()
	{
		return getBaseDir() + "config/";
	}

	bool FileManager::exists(std::string& const path)
	{
		return std::filesystem::exists(path);
	}

	void FileManager::saveStringToFile(std::string contents, std::string& const path)
	{
		std::ofstream out(path);
		out << contents;
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


	void FileManager::fetchBaseDir()
	{
		std::string sunriseDirFolder = "config/directory.sunrise";
		try {
			auto str = loadStringfromFile(sunriseDirFolder);
			_baseDir = std::move(str);
		}
		catch (...) {
			throw SRConfigLocFileNotFoundError();
		}

	}

}