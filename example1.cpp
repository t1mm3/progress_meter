#include "progress_meter.hpp"
#include <unistd.h>

void active_sleep(double secs) {
	clock_t begin = clock();
	double diff;

	while (1) {
		diff = double(clock()-begin) / CLOCKS_PER_SEC;

		if (diff >= secs) {
			break;
		}
	}
}
void task() {
	active_sleep(0.03);
}

int main() {
	size_t num_tasks = 1000;
	ProgressMeter progress_meter(num_tasks);

	for (int i=0; i<num_tasks; i++) {
		task();
		progress_meter();
	}
}