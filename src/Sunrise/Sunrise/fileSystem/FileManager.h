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

		/// <summary>
		/// DO NOT USE FOR PERFORMANCE SENSATIVE TASKS
		/// </summary>
		/// <returns></returns>
		static void saveStringToFile(std::string contents,std::string& const path);
		/// <summary>
		/// DO NOT USE FOR PERFORMANCE SENSATIVE TASKS
		/// </summary>
		/// <returns></returns>
		static std::string loadStringfromFile(std::string& const path);

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