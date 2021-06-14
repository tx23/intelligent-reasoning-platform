#ifndef TIMER_RECORD
#define TIMER_RECORD

#include <sys/time.h>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <chrono>  // NOLINT
#include <iostream>
#include <glog/logging.h>

using std::vector;
using std::string;
using std::queue;
using std::chrono::time_point;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::high_resolution_clock;


class Timer {
	public:
		Timer() : time_interval_(0.), min_(FLT_MAX), max_(0.), init_(false), count_(0) {
			start_ = high_resolution_clock::now();
		}

		void init() {
			start_ = high_resolution_clock::now();
		    init_ = true;
		}

		void update_boundary(float interval) {
			if (interval < min_) min_ = interval;
		    if (interval > max_) max_ = interval;
		}

		void record_time() {
			if (!init_) return;
		    auto end = high_resolution_clock::now();
		    std::chrono::duration<float> diff = end - start_;
		    float time = duration_cast<microseconds>(diff).count();
		    time_interval_ += time;
		    update_boundary(time);
		    start_ = high_resolution_clock::now();
		    count_++;
//		    if (getenv("INTERVAL_TIME") != NULL) {
//			    LOG(INFO) << "Interval time: " << time;
//		    }
		}

		void print_statistics() {
			LOG(INFO) << "Interval time:  ave: " << time_interval_ / count_
			          << "  min: " << min_ << "  max: " << max_;
		}

		void log(const char* msg) {
			if (time_interval_ == 0.) {
				auto end = high_resolution_clock::now();
				std::chrono::duration<float> diff = end - start_;
				time_interval_ = duration_cast<microseconds>(diff).count();
		    }
			std::cout << msg << ": " << time_interval_ << " us" << std::endl;
		}

		float getDuration() {
			if (time_interval_ == 0.) {
			    auto end = high_resolution_clock::now();
			    duration<float> diff = end - start_;
			    time_interval_ = duration_cast<microseconds>(diff).count();
		    }
			return time_interval_;
		}

		public:
			float time_interval_;

		protected:
		    time_point<high_resolution_clock> start_;
			float min_;
			float max_;
			bool init_;
			int count_;
};

#endif
