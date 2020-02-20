#include "progress_meter.hpp"
#include <vector>
#include <string.h>

static std::vector<char*> allocations;

const size_t max_size = 1024*1024*128;
const size_t max_alloc = 4;

size_t get_alloc_size() {
	int size = 0;

	while (!size) {
		size = rand() % max_alloc;
		if (size < 0) {
			size = -size;
		}
	}

	return size;
}

size_t task() {
	int num_alloc = rand() % 8;
	if (num_alloc < 0) {
		num_alloc = -num_alloc;
	}

	const size_t size = get_alloc_size();

	for (int i=0; i<num_alloc; i++) {
		char* p = new char[size];
		memset(p, 0, size);
		delete[] p;
	}

	if (size) {
		char* p = new char[size];
		allocations.push_back(p);
		memset(p, 0, size);
	}
	return size;
}

int main() {
	ProgressMeter progress_meter(max_size);

	size_t allocated = 0;
	while (allocated < max_size) {
		size_t sz = task();
		progress_meter(sz);

		allocated += sz;
	}

	for (auto& p : allocations) {
		delete[] p;
	}
}