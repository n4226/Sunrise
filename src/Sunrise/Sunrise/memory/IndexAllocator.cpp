#include "srpch.h"

#include "IndexAllocator.h"

namespace sunrise {


	IndexAllocator::IndexAllocator(size_t indexCount, size_t allocSize)
		: totalSize(indexCount* allocSize), indexCount(indexCount), allocSize(allocSize)
	{

	}

	IndexAllocator::~IndexAllocator()
	{

	}

	size_t IndexAllocator::alloc()
	{
		assert(currentIndex * allocSize < totalSize);
		return (currentIndex++) * allocSize;
	}

	void IndexAllocator::free(size_t index)
	{

	}

}