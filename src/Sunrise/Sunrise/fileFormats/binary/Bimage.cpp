#include "srpch.h"
#include "Bimage.h"

namespace sunrise {





	Bimage BimageEncoder::decode(std::fstream& stream)
	{
		
		stream.seekg(0, stream.end);
		int length = stream.tellg();
		stream.seekg(0, stream.beg);

		SR_CORE_ASSERT(length >= sizeof(BimageHeader));

		auto image = Bimage(length - sizeof(BimageHeader));

		stream.read(reinterpret_cast<char*>(image.allData),image.allDataSize);

//#if SR_ENABLE_PRECONDITION_CHECKS
//		memcpy_s(image.data,image.dataSize(),);
//#else 
//		memcpy();
//#endif
		return image;
	}

	void BimageEncoder::encode(const Bimage& decodedItem, std::fstream& stream)
	{
		//write header required field
		decodedItem.header->startOfData = sizeof(BimageHeader);
		decodedItem.header->fileSize = decodedItem.allDataSize;
		decodedItem.header->version = BimageHeader::currentVersion();

		//easter egg 
		decodedItem.header->headerChecksum = 0x1742;


		stream.write(reinterpret_cast<char*>(decodedItem.allData), decodedItem.allDataSize);
	}

	size_t Bimage::dataSize()
	{
		return allDataSize - sizeof(BimageHeader);
	}

	Bimage::Bimage(size_t dataSize)
		: allDataSize(dataSize + sizeof(BimageHeader)),
		allData((std::byte*)malloc(dataSize + sizeof(BimageHeader)))
	{
		header = reinterpret_cast<BimageHeader*>(allData);
		data = (allData + sizeof(BimageHeader));
	}

	Bimage::Bimage(const Bimage& other)
	{
		allDataSize = other.allDataSize;
		allData = (std::byte*)malloc(allDataSize);

		if (allData) {
			memcpy(allData, other.allData, other.allDataSize);
			header = reinterpret_cast<BimageHeader*>(allData);
			data = (allData + sizeof(BimageHeader));
		}
		else {
			header = nullptr;
			data = nullptr;
		}
	}

	Bimage::~Bimage()
	{
		free(allData);
	}

	vk::Format BimageHeader::vkFormat() {
		return static_cast<vk::Format>(VkFormat(format));
	}

	BinaryFileVersion BimageHeader::currentVersion() {
		return { 0,1 };
	}

}