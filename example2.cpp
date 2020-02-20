#include "progress_meter.hpp"
#include <vector>

static std::vector<char*> allocations;

void task(bool alloc) {
	int size = rand() % 1024;
	allocations.push_back(new char[size]);
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