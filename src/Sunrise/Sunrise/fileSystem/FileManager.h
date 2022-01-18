#pragma once
#include "srpch.h"

//extern FileManager filemanager;


namespace sunrise {

	class SUNRISE_API FileManager
	{
	public:
		// directories

		// app config directories
		static std::string appWokringDir();
		static std::string appConfigDir();

		// engine directories
		static std::string baseEngineResourceDir();

		static std::string engineTerrainChunkDir();
		static std::string engineTerrainChunkAttributesDir();
		static std::string engineMaterialDir();
		static std::string engineConfigDir();



		static bool exists(const std::string& path);
		static std::vector<std::string> listDirContents(const std::string& path, bool includeSubDirectories, bool filesOnl);
		static bool createIntermediateDirs(const std::string& path);

		/// <summary>
		/// DO NOT USE FOR PERFORMANCE SENSATIVE TASKS
		/// </summary>
		/// <returns></returns>
		static void saveStringToFile(const std::string& contents, const std::string&  path);

		static void saveBinaryToFile(const void* data, size_t length, const std::string& path);

		/// <summary>
		/// DO NOT USE FOR PERFORMANCE SENSATIVE TASKS
		/// </summary>
		/// <returns></returns>
		static std::string loadStringfromFile(const std::string& path);

		// errors

		class FileNotFoundError : public std::exception
		{
			virtual const char* what() const throw()
			{
				return "File Not Found";
			}
		};


		class SRConfigLocFileNotFoundError : public std::exception
		{
			virtual const char* what() const throw()
			{
				return "Sunrise directory config file not found";
			}
		};

	protected:

		FileManager();

		static void fetchBaseEngineResourceDir();

		static std::string _baseDir;
	};

}
