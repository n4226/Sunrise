#pragma once

#include "srpch.h"

namespace sunrise {


	class SUNRISE_API VaribleIndexAllocator
	{
	public:
		VaribleIndexAllocator(size_t size);

		~VaribleIndexAllocator();

		/// <summary>
		/// returns UINT64_MAX if full
		/// </summary>
		/// <returns></returns>
		size_t alloc(size_t size);
		void free(size_t address, size_t size);


		size_t maxAllocationSizeAvailable();

		const size_t totalSize;
		size_t allocatedSize = 0;

	private:
		
		struct freeSpace {
			size_t start;
			size_t size;

			bool operator<(const freeSpace& other) const {
				return size < other.size;
			}
		};

		struct freeSpace_rank_greater_than {
			bool operator()(freeSpace const* a, freeSpace const* b) const {
				return a->size < b->size;
			}
		};

		void addFreeSpace(VaribleIndexAllocator::freeSpace* newSpace);

		//TODO: this is for testing
		//private:

		std::vector<freeSpace*> freeSpaces = {};
		std::unordered_map<size_t, freeSpace*> freeSpaces_ends = {};
		std::unordered_map<size_t, freeSpace*> freeSpaces_beginnings = {};

		std::unordered_set<freeSpace*> noLongerFreeSpaceHitList = {};

		size_t currentAdress = 0;
	};

}