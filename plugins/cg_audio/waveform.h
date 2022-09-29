#include <numeric>
#include <iterator>

namespace wave {

constexpr float PI = 3.1415962;

template <typename OutIt> constexpr void sine(OutIt begin, OutIt end, float amplitude, float frequency, float phase, size_t sampling_rate)
{
	constexpr auto buffer_length = std::distance(begin, end);
	constexpr auto sample_count = buffer_length * sampling_rate / frequency;
	
	for (size_t i = 0; i < buffer_length; {++i; ++begin})
	{
		*begin = amplitude * std::sin(phase + 2 * PI * frequency);
	}
}

} // namespace wave