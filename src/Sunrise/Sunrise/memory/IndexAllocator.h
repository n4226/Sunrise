#pragma once

#include "srpch.h"

namespace sunrise {

	/// <summary>
	/// keeps track of indices of entries with an index and fixed size in collection of memory
	/// //TODO: fix this to actually work and de allocate
	/// </summary>
	class SUNRISE_API IndexAllocator
	{
	public:
		IndexAllocator(size_t size, size_t allocSize);

		~IndexAllocator();

		/// <summary>
		/// returns UINT64_MAX if full
		/// </summary>
		/// <returns></returns>
		size_t alloc();
		void free(size_t index);

		const size_t totalSize;
		const size_t indexCount;
		const size_t allocSize;
		size_t usedSize = 0;


	private:

		/// <summary>
		/// in unscaled space - all indicies a next to eachother = output space / allocSize
		/// </summary>
		//std::set<size_t> usedIndicies;

		size_t currentIndex = 0;

	};

}