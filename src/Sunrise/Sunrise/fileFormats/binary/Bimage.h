#pragma once

#include "srpch.h"
#include "BinaryFileType.h"

namespace sunrise {

#pragma padding(1)

	// posibly add a majic number
	struct BimageHeader { // offset, size -- offset should be a multiple of size or there will be evil padding
		BinaryFileVersion version; // 0,2
		uint16_t startOfData; // 2,2 -- field includes any padding after header
		uint16_t format; // 4,2
		uint32_t length; // 6,2
		uint32_t width; // 8,2
		uint32_t height; // 10,2
		uint32_t headerChecksum; // 12,4
		uint64_t fileSize; // 16,8
		uint16_t linearLayout; // 24,2 -- bool


		//static constexpr size_t length = 26;
	}; // 26

	//template<size_t size>
	//struct Bimage
	//{
	//	BimageHeader header; // 0,26
	//	std::array<char, size - BimageHeader::length> data;
	//};

	//template<size_t dataLength>
	//class BimageEncoder : public BinaryFileTypeEncoder<Bimage<dataLength>> {
	//public:

	//	

	//};
}