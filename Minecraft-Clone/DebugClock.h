#pragma once
#include <chrono>
#include <vector>
#include <iostream>

class DebugClock {
private:
#define t_point std::chrono::steady_clock::time_point
#define t_now std::chrono::high_resolution_clock::now()

public:
	static void setEnabled(bool isEnabled) {
		enabled = isEnabled;
	}

	static void recordTime(const char* label) {
		if (enabled) {
			time_points.emplace_back(std::make_pair(label, t_now));
		}
	}

	static void printTimePoints() {
		if (!enabled) {
			return;
		}

		if (time_points.empty()) {
			std::cout << "<=== Debug Clock Empty! ===>" << std::endl;
			return;
		}

		std::cout << "<=== Debug Clock Time Points ===>" << std::endl;

		for (auto& t : time_points) {
			std::cout << "\t" << t.first;

			auto t_diff = std::chrono::duration_cast<std::chrono::milliseconds>(t.second - time_points[0].second);

			std::cout << " : " << t_diff.count() << "ms" << std::endl;
		}

		std::cout << std::endl;
	}

private:
	static std::vector<std::pair<const char*, t_point>> time_points;
	static bool enabled;
};