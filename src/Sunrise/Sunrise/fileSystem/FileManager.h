#pragma once
#include "srpch.h"

//extern FileManager filemanager;


namespace sunrise {

	class FileManager
	{
	public:
		// directories

		static std::string getBaseDir();

		static std::string getTerrainChunkDir();
		static std::string getTerrainChunkAttributesDir();
		static std::string getMaterialDir();
		//TODO: add application spacific config dirs
		static std::string getConfigDir();


		static std::string wokringDir();

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

		static void fetchBaseDir();

		static std::string _baseDir;
	};

}