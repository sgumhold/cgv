#pragma once

#include <iostream>
#include <chrono>

class Timer
{
public:
	Timer(const char* name)
		: m_name(name)
	{
		m_canceled = false;
		m_startPoint = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		Stop();
	}

	void Cancel()
	{
		m_canceled = true;
	}

	void Stop()
	{
		auto endPoint = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startPoint).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count();

		auto duration = end - start;

		double ms = duration * 0.001;

		if (!m_canceled)
			std::cout << "(TIMER) " << m_name << " : " << ms << "ms\n";
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_startPoint;
	const char* m_name;
	bool m_canceled;
};