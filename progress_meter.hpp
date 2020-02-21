/*
 * Progress Meter
 * Copyright (c) 2020 Tim Gubner
 *
 * Release under MIT license
 */

#ifndef H_PROGRESS_METER
#define H_PROGRESS_METER

#include <ctime>
#include <string.h>
#include <iostream>

#if 0
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

struct BaseProgressMeter {
	template<size_t N, typename T>
	struct Window {
		static constexpr size_t WINDOW_SIZE = N;
		T m_data[WINDOW_SIZE];
		size_t m_write_pos;

		T m_sum;
		
		T sum() const {
			return m_sum;
		}


		size_t num_collected() const {
			return m_write_pos;
		}

		bool empty() const {
			return !m_write_pos;
		}

		bool full() const {
			return m_write_pos >= WINDOW_SIZE;
		}

		size_t size() const {
			return full() ? WINDOW_SIZE : m_write_pos;
		}

		double avg() const {
			if (empty()) {
				return 0.0;
			}
			return (double)sum() / (double)size();
		}


		void add(const T& v) {
			size_t index = m_write_pos % WINDOW_SIZE;
			m_sum -= m_data[index];
			m_data[index] = v;
			m_sum += m_data[index];

			m_write_pos++;
		}

		void clear() {
			for (size_t i=0; i<WINDOW_SIZE; i++) {
				m_data[i] = 0;
			}
			m_write_pos = 0;
			m_sum = 0;
		}

		Window() {
			clear();
		}

	};

	struct BookKeeper {
		clock_t last_clock;
		bool has_last = false;
		size_t last_tick = 0;
		size_t warmup_counter = 0;

		Window<4, double> speed_window;


		//! Report every x secs
		static constexpr double report_sec = 2.0;

		//! Refresh every x secs
		static constexpr double refresh_sec = std::min(report_sec / 100.0, 0.0050);

		static constexpr size_t warmup_iterations = 10;
		static constexpr size_t min_granularity = 1;
		static constexpr size_t max_granularity = 10000;

		static constexpr double max_divergence = 2.0;

		void reset() {
			has_last = false;
			last_tick = 0;
			warmup_counter = 0;
			speed_window.clear();
		}

		template<typename T>
		void keep(size_t& next_refresh, size_t num_current, size_t num_total,
				const T& update_report) {
			clock_t curr_clock = clock();

			if (has_last) {
				if (curr_clock < last_clock) {
					return;
				}
				const bool warmup_mode = 
					warmup_counter < warmup_iterations;
				const double diff_sec = double(curr_clock - last_clock)
					/ CLOCKS_PER_SEC;
				const double speed = diff_sec / (double)(num_current - last_tick);

				speed_window.add(speed);
				warmup_counter++;

				const size_t todo = num_total - num_current;
				const double avg = speed_window.avg();

				if (!warmup_mode && speed_window.full() &&
						(speed/max_divergence > avg || avg > max_divergence*speed)) {
					speed_window.clear();

#if 0
					bool lo = speed/max_divergence < avg;
					bool hi = avg < max_divergence*speed;

					LOG("Too far off, clear window.\n"
						"Avg %f, Speed %f, between %f, %f, lo %d, hi %d\n",
						avg, speed, speed/max_divergence,
						max_divergence*speed, lo, hi);
#endif
					warmup_counter = 0;
				} else {
					LOG("Diff %f Avg %f, Speed %f, Dist %f, Todo %f\n",
						diff_sec, avg, speed,
						(double)(num_current - last_tick),
						(double)(num_total - num_current));
				}

				update_report(diff_sec, todo, avg);
#if 0
				report_cum_time += diff_sec;

				if (report_cum_time >= report_sec) {
					output((double)num_current / (double)num_total,
						(double)todo * avg);

					report_cum_time = 0.0;
				}
#endif
				int64_t delay = refresh_sec / avg;
				if (warmup_mode) {
					delay = min_granularity;
				} else {
					delay = std::min(delay, (int64_t)min_granularity);
					delay = std::max(delay, (int64_t)max_granularity);
				}

				next_refresh = num_current + delay;
				LOG("next @ %ld\n", next_refresh);
			} else {
				has_last = true;
				next_refresh = num_current + min_granularity;
			}


			last_tick = num_current;
			last_clock = clock();
		}
	};

	virtual ~BaseProgressMeter() {

	}

	virtual void output(double progress, double sec_to_finish) {
		std::cerr << (progress*100.0) << "% done ... "
			<< sec_to_finish << " secs to go\n" << std::endl;
	}
};

struct ProgressMeter : BaseProgressMeter {
	
private:
	size_t next_refresh = 0;
	size_t num_current;
	size_t num_total;

	BookKeeper book;
	double report_cum_time = 0.0;

	// internal refresh ... can count and print. However doesn't influence progress
	void __refresh() {
		book.keep(next_refresh, num_current, num_total,
			[&] (double diff_sec, double todo, double avg) {
				report_cum_time += diff_sec;

				if (report_cum_time >= book.report_sec) {
					output((double)num_current / (double)num_total,
						(double)todo * avg);

					report_cum_time = 0.0;
				}
			}
		);
	}


public:

	// Update progress to the next tick
	void operator()(size_t ticks = 1) {
		num_current += ticks;

		if (num_current >= next_refresh) {
			__refresh();
		}
	}

	void extend_by(size_t ticks) {
		num_total += ticks;
	}

	void extend_to(size_t ticks) {
		num_total = ticks;
	}

	void reset() {
		next_refresh = 0;
		num_current = 0;
		report_cum_time = 0;
		book.reset();
	}

	void clear(size_t ticks) {
		reset();
		num_total = ticks;
	}

	ProgressMeter(size_t total) {
		clear(total);
	}
};

#endif