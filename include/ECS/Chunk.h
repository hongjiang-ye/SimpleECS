#pragma once
#include <vector>


namespace ECS
{
	// Just a wrapper of a memory chunk in specified size
	struct Chunk
	{
		Chunk() = delete;
		Chunk(const Chunk&) = delete;
		Chunk operator=(const Chunk&) = delete;

		Chunk(size_t chunk_size) : chunk_size(chunk_size)
		{
			chunk_ptr = new char[chunk_size];
		}

		// Compute the entry address with index and necessary size information, which are stored and passed in 
		// by the caller, instead of saving a copy in each chunk. Write or read of the returned address is 
		// then executed by the caller.
		void* GetAddress(const size_t row_index, const size_t col_index,
			const std::vector<size_t>& row_sizeofs, const size_t col_num)
		{

			char* address = static_cast<char*>(chunk_ptr);

			for (size_t i = 0; i < row_index; i++) {
				address += col_num * row_sizeofs[i];
			}
			address += col_index * row_sizeofs[row_index];

			if (address > static_cast<char*>(chunk_ptr) + chunk_size) return nullptr;
			return static_cast<void*>(address);
		}

		size_t chunk_size;
		void* chunk_ptr;
	};
}
