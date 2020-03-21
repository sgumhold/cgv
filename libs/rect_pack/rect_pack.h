#pragma once

#include <vector>
#include <fstream>

#include "lib_begin.h"

namespace rect_pack {

	/// minimal data structure to hold the size of a rectangle
	struct rectangle_size
	{
		int width, height;
	};

	/// minimal data structure to hold the position and size of a rectangle
	struct rectangle
	{
		int x, y, width, height;
	};

	/// randomly construct a vector of rectangle sizes with the given number of rectangles
	extern CGV_API void construct_random_rectangles(unsigned nr_rectangles, std::vector<rectangle_size>& rectangle_sizes_out);

	/// different strategies for comparing rectangles during sorting
	enum CompareStrategy
	{
		CS_ShorterSideFirst, // min(width,height) -> max(width,height)
		CS_LongerSideFirst,  // max(width,heigth) -> min(width,height)
		CS_Area,             // width*height
		CS_Perimeter,        // width+height
		CS_Aspect,           // width/height
		CS_SideDifferent     // abs(width-height)
	};

	/// different packing strategies
	enum PackingStrategy
	{
		PS_Shelf,          // organize in shelves with single height
		PS_ShelfRefill,    // extend shelf packing by keeping track of empty regions and filling them if possible
		PS_Skyline,        // greedy skyline algorithm always placing next rectangle in order
		PS_SkylineRefill,  // extend greedy skyline algorihtm by keeping track of empty regions and filling them if possible
		PS_Guillotine,     // place rectangles in lower left cornder of rectangular subdivision of empty space, after rectangle placement split resulting L-shape into two rectangles
		PS_MaxRectangle,    // same as Guillotine strategy but represent L-shape by two overlapping rectangles of maximum size
		PS_NrStrategies    // enum giving the number of packing strategies
	};

	/** compute a sorted order of the input rectangle sizes in the permutation
		output vector for the given sort strategy in ascending or descending (default) order */
	extern CGV_API void compute_rectangle_permutation(
		const std::vector<rectangle_size>& rectangle_sizes,
		std::vector<unsigned>& permutation_out,
		CompareStrategy compare_strategy = CS_ShorterSideFirst,
		bool sort_ascending = false);

	/** suggest a size width_out X height_out of the output texture by computing the total rectangle area and adding percentual_safety percent
		return the total rectangle area. Restriction to power of 2 dimensions is possible. The percentual_safty allows to iteratively adapt the
		size of the output texture, if the first guess was too small */
	extern CGV_API unsigned suggest_output_size(
		const std::vector<rectangle_size>& rectangle_sizes,
		unsigned& width_out, unsigned& height_out,
		bool restrict_to_power_of_two = true,
		float percentual_safety = 0.02f);

	/** try to pack rectangles of all given sizes in the given order into an output texture of given dimensions with the given strategy.
		The function returns the number of rectangles that did not fit into the output texture. For each rectangle that did not fit in,
		the corresponding entry in rectangles_out contains only zeros and if print_warnings is set to true, also a message is printed to
		std::cout. */
	extern CGV_API unsigned pack_rectangles(
		const std::vector<rectangle_size>& rectangle_sizes,
		const std::vector<unsigned>& permutation,
		std::vector<rectangle>& rectangles_out,
		unsigned width, unsigned height,
		PackingStrategy strategy = PS_MaxRectangle,
		bool print_warnings = false);

	/** use the compute_rectange_permutation function to compute an ordering pack_rectangles method to pack the given rectangles
		into an output texture of dimensions computed with suggest_output_size according to the given strategy. In case the
		pack_rectangles function fails, the percentual safety is increased until all rectangles fit. In case of print_progress being
		true, an asterix is streamed out to std::cout for any failed packing iteration
		*/
	extern CGV_API float pack_rectangles_iteratively(
		const std::vector<rectangle_size>& rectangle_sizes,
		unsigned& width_out, unsigned& height_out,
		std::vector<rectangle>& rectangles_out,
		CompareStrategy compare_strategy = CS_ShorterSideFirst,
		bool sort_ascending = false,
		bool restrict_to_power_of_two = true,
		PackingStrategy strategy = PS_MaxRectangle,
		bool print_warnings = false,
		bool print_progress = false);

	/// save an svg graphics to the given stream that shows the rectangles in a drawing area with the given dimensions
	extern CGV_API bool save_svg(std::ofstream& os, unsigned width, unsigned height, const std::vector<rectangle>& rectangles);

	/// save rectangle packing into a web page with an svg graphics
	extern CGV_API bool save_rectangles_html(const std::string& file_name, unsigned width, unsigned height, const std::vector<rectangle>& rectangles);

	/// generate a packing with the given parameters, measure the time and save result to web page with a file name starting with the given prefix
	extern CGV_API void analyze_pack_rectangles_iteratively(
		const std::string& file_name_prefix,
		std::vector<rectangle_size>& rectangle_sizes,
		CompareStrategy compare_strategy = CS_ShorterSideFirst,
		bool sort_ascending = false,
		bool restrict_to_power_of_two = true,
		PackingStrategy strategy = PS_MaxRectangle,
		bool print_warnings = false,
		bool print_progress = false);

	/// call analyze_pack_rectangles_iteratively for all packing strategies
	extern CGV_API void compare_packing_strategies(
		const std::string& file_name_prefix,
		std::vector<rectangle_size>& rectangle_sizes,
		CompareStrategy compare_strategy = CS_ShorterSideFirst,
		bool sort_ascending = false,
		bool restrict_to_power_of_two = true,
		bool print_warnings = false,
		bool print_progress = false);
}