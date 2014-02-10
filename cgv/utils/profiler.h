#ifndef PROFILER_H
#define PROFILER_H

#include <map>
#include <vector>
#include <ctime>
#include <cgv/utils/stopwatch.h>
#include <iostream>
#include <string>


namespace cgv {
	namespace utils{

	/**
	 * A profiler class for managing different stopwatches.
	 *
	 * Example:
	 *
	 * Profiler<> p;
	 * 
	 * p.prepare_counter("all");
	 * p.prepare_counter("graphics");
	 * p.prepare_counter("simulation");
	 *
	 * ...
	 * while(not_stopped)
	 * {
	 *  stopwatch sa(p.get_time_accu("all"));
	 *	{
	 *		stopwatch ss(p.get_time_accu("simulation"));
	 *		simulate();
	 *	}
	 *	{
	 *		stopwatch sr(p.get_time_accu("graphics"));
	 *		render();
	 *	}
	 * }
	 * ...
	 * std::cout << p;
	 */

	template< typename T = std::string>
	class Profiler
	{
		
		std::map<T,double> counter_times;
		std::map<T,int> counter_calls;

	public:
		void prepare_counter(const T &handle)
		{
			counter_times[handle]=0.0;
			counter_calls[handle]=0;
		
		}

		inline double *get_time_accu(const T handle)
		{
			counter_calls[handle]++;
			return &(counter_times[handle]);
		}

		void zero_counters()
		{
			for(std::map<T,double>::iterator it = counter_times.begin();
				it != counter_times.end();it++)
			{
				it->second=0;
				counter_calls[it->first]=0;
			}
		}

		
		friend std::ostream& operator<< (std::ostream& out, Profiler p)
		{
			for(std::map<T,double>::iterator it = p.counter_times.begin();
				it != p.counter_times.end();it++)
			{
				out << it->first << ": "<< p.counter_calls[it->first] << " calls, "<<it->second << " sec, " 
					<< it->second/p.counter_calls[it->first] << " sec/call (mean)"<< std::endl;
			}
		
			return out;
		}

	};
	
	}

}


#endif// PROFILER_H