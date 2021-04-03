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

		virtual DecodedView decode(const std::ofstream& stream) = 0;
		/// <summary>
		/// will encode the item into the given stream at the current position
		/// </summary>
		/// <param name="decodedItem"></param>
		/// <param name="stream"></param>
		virtual void encode(const DecodedView& decodedItem, std::ofstream& stream) = 0;

		void writeToFile(const DecodedView& decodedItem,const std::string path,bool createIfNeccessary);
		DecodedView readFromFile(const std::string path);

	};

	template<typename DecodedView>
	inline void BinaryFileTypeEncoder<DecodedView>::writeToFile(const DecodedView& decodedItem, const std::string path, bool createIfNeccessary)
	{
		std::fstream fs(path, std::fstream::out | std::fstream::binary);

		if (!fs.good()) {
			//abort
			SR_CORE_ERROR("binary write to file failed");
			throw FileManager::FileNotFoundError;
		}

		encode(decodedItem, fs);
		fs.close();
	}

	template<typename DecodedView>
	inline DecodedView BinaryFileTypeEncoder<DecodedView>::readFromFile(const std::string path)
	{
		std::fstream fs(path, std::fstream::in | std::fstream::binary);


		if (!fs.good()) {
			//abort
			SR_CORE_ERROR("binary read from file failed");
			throw FileManager::FileNotFoundError;
		}
		
		auto data = decode(fs);

		fs.close();

		return data;
	}

}
