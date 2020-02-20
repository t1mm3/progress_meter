#include "progress_meter.hpp"
#include <vector>
#include <string.h>

static std::vector<char*> allocations;

void task(bool alloc) {
	int size = rand() % 1024;
	if (size < 0) {
		size = -size;
	}

	if (!size) {
		return;
	}

	char* p = new char[size];
	allocations.push_back(p);
	memset(p, 0, size);
}

int main() {
	size_t num_tasks = 20000;
	ProgressMeter progress_meter(num_tasks);

	for (int i=0; i<num_tasks/2; i++) {
		task(true);
		progress_meter();
	}

	for (auto& p : allocations) {
		delete[] p;
	}
}