#include "progress_meter.hpp"
#include <vector>
#include <string.h>
#include <thread>
#include <atomic>

using namespace std;

size_t fib(size_t i) {
	if (i == 1 || i == 2) {
		return 1;
	}
	return fib(i-1) + fib(i-2);
}

size_t max_tasks;
atomic<size_t> cur_task;

std::unique_ptr<ParallelProgressMeter> progress;


void task(size_t offset) {
	fib(offset+1);
}

void worker(int thread_id) {
	ParallelProgressMeter::Thread thread_progress(*progress);

	while (1) {
		size_t old = cur_task.fetch_add(1);
		if (old >= max_tasks) {
			// done
			break;
		}

		task(old);
		thread_progress();
	}
}


static vector<thread> workers;

int main() {
	max_tasks = 1024;
	cur_task = 0;
	progress = std::make_unique<ParallelProgressMeter>(max_tasks);

	for (int i=0; i<std::thread::hardware_concurrency(); i++) {
		workers.push_back(thread(worker, i));
	}

	for (auto& w : workers) {
		w.join();
	}
}