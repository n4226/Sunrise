#pragma once

#include "srpch.h"
#include "BinaryFileType.h"

namespace sunrise {

///#pragma padding(1)
//#pragma pack(push, 1)
	// posibly add a majic number
	struct BimageHeader { // offset, size -- offset should be a multiple of size or there will be evil padding
		BinaryFileVersion version; // 0,2
		uint16_t startOfData; // 2,2 -- field includes any padding after header
		uint32_t format; // 4,4
		uint16_t width; // 8,2
		uint16_t height; // 10,2
		uint16_t depth; // 12,2
		uint32_t headerChecksum; // 14,4
		uint64_t fileSize; // 18,8
		uint16_t linearLayout; // 26,2 -- bool


		//static constexpr size_t length = 26;

		// helpers

		vk::Format vkFormat();
		static BinaryFileVersion currentVersion();

	}; // 28
	//static_assert(sizeof(BimageHeader) == 28);
	class BimageEncoder;

	//template<size_t size>
	struct Bimage
	{
		BimageHeader* header; // 0,26
		//std::array<char, size - BimageHeader::length> data; - removed template for simplicity - lack of safetyr will be okay for this type
		std::byte* data; // 26,0..<.infinity

		size_t dataSize();

		Bimage(size_t dataSize);
		Bimage(const Bimage& other);
		~Bimage();

	private:
		friend BimageEncoder;


		/// <summary>
		/// the actual full bimage data in memory
		/// </summary>
		std::byte* allData;
		size_t allDataSize;
	/*
		friend BimageEncoder;

		Bimage(BimageHeader header, std::byte* data)
		: header(header), data(data)
		{};*/

	};

	//template<size_t dataLength>
	class BimageEncoder : public BinaryFileTypeEncoder<Bimage> {
	public:

		virtual Bimage decode(std::fstream& stream) override;

		virtual void encode(const Bimage& decodedItem, std::fstream& stream) override;

	};
}