#include "pch.h"
#include "FileManager.h"


//FileManager filemanager;

std::string FileManager::getBaseDir()
{
	return "c:/code/FSTS-World-Data/";
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
