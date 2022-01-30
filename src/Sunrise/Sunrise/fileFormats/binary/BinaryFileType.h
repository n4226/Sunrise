#pragma once

#include "srpch.h"
#include "../../fileSystem/FileManager.h"

/// <summary>
/// All binary formats assume little endean CPUs for now
/// </summary>

namespace sunrise {
	
	struct BinaryFileVersion {
		uint8_t majorVersion;
		uint8_t minorVersion;
	};

	template<typename DecodedView>
	class BinaryFileTypeEncoder
	{
	public:

		/// <summary>
		/// asumes stream is already open
		/// </summary>
		/// <param name="stream"></param>
		/// <returns></returns>
		virtual DecodedView decode(std::fstream& stream) = 0;
		//virtual DecodedView decode(void* encodedObject, size_t encodedObjectLength) = 0;
		/// <summary>
		/// will encode the item into the given stream at the current position
		/// </summary>
		/// <param name="decodedItem"></param>
		/// <param name="stream"></param>
		virtual void encode(const DecodedView& decodedItem, std::fstream& stream) = 0;
		//virtual void encode(const DecodedView& decodedItem, void* encodedObject, size_t encodedObjectLength) = 0;

		void writeToFile(const DecodedView& decodedItem,const std::string path,bool createIfNeccessary);
		DecodedView readFromFile(const std::string path);

	};

	//template<typename DecodedView>
	//inline void BinaryFileTypeEncoder<DecodedView>::encode(const DecodedView& decodedItem, std::ofstream& stream)
	//{
	//	auto length = (size_t)stream.tellg();

	//	//mesh = malloc(meshLength);

	//	stream.seekg(0);
	//	file.read(reinterpret_cast<char*>(mesh), meshLength);
	//}

	template<typename DecodedView>
	inline void BinaryFileTypeEncoder<DecodedView>::writeToFile(const DecodedView& decodedItem, const std::string path, bool createIfNeccessary)
	{
		if (createIfNeccessary) {
			FileManager::createIntermediateDirs(path);
		}

		std::fstream fs(path, std::fstream::out | std::fstream::binary);

		if (!fs.good()) {
			//abort
			SR_CORE_ERROR("binary write to file failed");
			throw FileManager::FileNotFoundError();
		}

		encode(decodedItem, fs);
		fs.close();
	}

	template<typename DecodedView>
	inline DecodedView BinaryFileTypeEncoder<DecodedView>::readFromFile(const std::string path)
	{	
		// std::ios::ate | 
		std::fstream fs(path,std::fstream::in | std::fstream::binary);


		if (!fs.good()) {
			//abort
			SR_CORE_ERROR("binary read from file failed");
			throw FileManager::FileNotFoundError();
		}
		
		auto data = decode(fs);

		fs.close();

		return data;
	}

}
