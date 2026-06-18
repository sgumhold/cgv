#pragma once

#include <chrono>

namespace cgv {
namespace data {

using time_point = std::chrono::high_resolution_clock::time_point;

/// @brief Keep a time stamp to store modified time of objects.
class time_stamp {
public:
	bool is_valid() const {
		return modified_time_ != time_point::min();
	}

	void reset() {
		modified_time_ = time_point::min();
	}

	void modified() {
		modified_time_ = std::chrono::high_resolution_clock::now();
	}

	time_point get_modified_time() const {
		return modified_time_;
	}

	bool operator>(const time_stamp& other) {
		return this->modified_time_ > other.modified_time_;
	}
	bool operator<(const time_stamp& other) {
		return this->modified_time_ < other.modified_time_;
	}

private:
	time_point modified_time_ = time_point::min();
};

} // namespace data
} // namespace cgv
