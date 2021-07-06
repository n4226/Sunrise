#include "srpch.h"
#include "VaribleIndexAllocator.h"

namespace sunrise {

	VaribleIndexAllocator::VaribleIndexAllocator(size_t size)
		: totalSize(size)
	{
		auto fullSpace = new freeSpace;

		fullSpace->start = 0;
		fullSpace->size = size;

		freeSpaces.push_back(fullSpace);
		freeSpaces_beginnings.insert(std::pair<size_t, freeSpace*>(0, fullSpace));
		freeSpaces_ends.insert(std::pair<size_t, freeSpace*>(size - 1, fullSpace));
		std::make_heap(freeSpaces.begin(), freeSpaces.end(), freeSpace_rank_greater_than());
	}

	VaribleIndexAllocator::~VaribleIndexAllocator()
	{

	}


	size_t VaribleIndexAllocator::maxAllocationSizeAvailable()
	{
		freeSpace* space = nullptr;

		bool loop = true;

		if (freeSpaces.empty())
		{
			throw std::runtime_error("allocation empty");
			return 0;
		}

		std::pop_heap(freeSpaces.begin(), freeSpaces.end(), freeSpace_rank_greater_than());
		space = freeSpaces[freeSpaces.size() - 1];
		std::push_heap(freeSpaces.begin(), freeSpaces.end(), freeSpace_rank_greater_than());
		
		return space->size;
	}


	size_t VaribleIndexAllocator::alloc(size_t size)
	{

		freeSpace* space = nullptr;

		bool loop = true;

		if (freeSpaces.empty())
		{
			throw std::runtime_error("allocation empty");
			return 0;
		}

		while (loop)
		{
			std::pop_heap(freeSpaces.begin(), freeSpaces.end(), freeSpace_rank_greater_than());
			space = freeSpaces[freeSpaces.size() - 1];
			freeSpaces.pop_back();

			if (noLongerFreeSpaceHitList.count(space) == 0) {
				freeSpaces_ends.erase(space->start + space->size - 1);
				freeSpaces_beginnings.erase(space->start);
				loop = false;
			}
			else {
				noLongerFreeSpaceHitList.erase(space);
				delete space;
			}
		}

		SR_CORE_ASSERT(space->size >= size);

		allocatedSize += size;

		auto newSpace = new freeSpace;

		newSpace->start = space->start + size;
		newSpace->size = space->size - size;

		if (newSpace->size > 0)
			addFreeSpace(newSpace);

#if SR_LOGGING
		auto allocPercent = static_cast<float>(allocatedSize) / (totalSize / 100);
		if (allocPercent > 10)
			SR_CORE_WARN("varible allocator usage is {}%",allocPercent);
#endif

		auto start = space->start;

		delete space;

		return start;


		/* pre free list heap system

		allocatedSize += size;
		//std::cout << "allocated: " << allocatedSize << "/" << totalSize << " = " << static_cast<float>(allocatedSize) / (totalSize / 100) << "%" << std::endl;
		assert(allocatedSize <= totalSize);

		auto out = currentAdress;
		currentAdress += size;

		return out;
		*/
	}

	void VaribleIndexAllocator::free(size_t address, size_t size)
	{



		auto newSpace = new freeSpace;

		newSpace->start = address;
		newSpace->size = size;


		if (freeSpaces_ends.count(address - 1) > 0) {
			auto previous = freeSpaces_ends.at(address - 1);

			freeSpaces_ends.erase(address - 1);
			noLongerFreeSpaceHitList.insert(previous);

			newSpace->start = previous->start;
			newSpace->size += previous->size;
		}

		if (freeSpaces_beginnings.count(address + size) > 0) {
			auto next = freeSpaces_beginnings.at(address + size);

			freeSpaces_beginnings.erase(address + size);
			noLongerFreeSpaceHitList.insert(next);

			newSpace->size += next->size;
		}


		addFreeSpace(newSpace);


		allocatedSize -= size;

	}

	void VaribleIndexAllocator::addFreeSpace(VaribleIndexAllocator::freeSpace* newSpace)
	{
		freeSpaces.push_back(newSpace);
		std::push_heap(freeSpaces.begin(), freeSpaces.end(), freeSpace_rank_greater_than());
		freeSpaces_beginnings.emplace(std::pair<size_t, freeSpace*>(newSpace->start, newSpace));
		freeSpaces_ends.emplace(std::pair<size_t, freeSpace*>(newSpace->start + newSpace->size - 1, newSpace));
	}

}