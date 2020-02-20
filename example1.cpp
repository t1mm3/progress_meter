#include "progress_meter.hpp"
#include <unistd.h>

void task() {
	usleep(300);
}

int main() {
	size_t num_tasks = 10000;
	ProgressMeter progress_meter(num_tasks);

	for (int i=0; i<num_tasks; i++) {
		task();
		progress_meter();
	}
}