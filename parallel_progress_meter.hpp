/*
 * Progress Meter
 * Copyright (c) 2020 Tim Gubner
 *
 * Release under MIT license
 */

#ifndef H_PARALLEL_PROGRESS_METER
#define H_PARALLEL_PROGRESS_METER

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <unordered_set>
#include "progress_meter.hpp"

struct ParallelProgressMeter : BaseProgressMeter{
	std::atomic<size_t> num_current;

	struct Thread {
		BookKeeper book;
		ParallelProgressMeter& master;

		double report_cum_time = 0.0;

		Thread(ParallelProgressMeter& master)
			 : master(master) {
			reset();
			master.register_thread(*this);
		}

		~Thread() {
			master.unregister_thread(*this);
		}

		size_t next_refresh;

		void __refresh(size_t curr_ticks) {
			LOG("total %d\n", master.num_total);
			book.keep(next_refresh, curr_ticks, master.num_total,
				[&] (double diff_sec, double todo, double avg) {
					report_cum_time += diff_sec;

					if (report_cum_time >= book.report_sec) {
						master.output(
							(double)curr_ticks / (double)master.num_total,
							(double)todo * avg);

						report_cum_time = 0.0;
					}
				}
			);
		} 

		void operator()(size_t ticks = 1) {
			auto old = master.num_current.fetch_add(ticks) + ticks;

			if (old >= next_refresh) {
				__refresh(old);
			}
		}

		void reset() {
			book.reset(),
			report_cum_time = 0.0;
			next_refresh = 0;
		}
	};

	void register_thread(Thread& t) {
		std::lock_guard<std::mutex> lock(m_mutex);

		m_threads.insert(std::addressof(t));
	}

	void unregister_thread(Thread& t) {
		std::lock_guard<std::mutex> lock(m_mutex);

		m_threads.erase(std::addressof(t));
	}

	ParallelProgressMeter(size_t num) {
		num_total = num;
	}

private:
	std::mutex m_mutex;
	std::unordered_set<Thread*> m_threads;

	size_t num_total;
};

#endif