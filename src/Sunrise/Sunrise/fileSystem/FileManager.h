#pragma once


//extern FileManager filemanager;

class FileManager
{
public:
	// directories

	static std::string getBaseDir();

	static std::string getTerrainChunkDir();
	static std::string getTerrainChunkAttributesDir();
	static std::string getMaterialDir();
	static std::string getConfigDir();


	static void saveStringToFile();
	static void loadStringfromFile();

protected:

	FileManager();

};

