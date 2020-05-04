#include "rect_pack.h"
#include "RectangleBinPack/MaxRectsBinPack.h"
#include "RectangleBinPack/GuillotineBinPack.h"
#include "RectangleBinPack/SkylineBinPack.h"
#include "RectangleBinPack/ShelfBinPack.h"
#include <cstdio>
#include <random>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime>

namespace rect_pack {

	void construct_random_rectangles(unsigned nr_rectangles, std::vector<rectangle_size>& rectangle_sizes_out)
	{
		std::default_random_engine generator;
		std::uniform_int_distribution<unsigned> distribution(1, 150);
		for (unsigned i = 0; i < nr_rectangles; ++i) {
			rectangle_size r;
			r.width = distribution(generator);
			r.height = distribution(generator);
			rectangle_sizes_out.push_back(r);
		}
	}

	struct CompareShorterSideFirst
	{
		const std::vector<rectangle_size>& R;
		CompareShorterSideFirst(const std::vector<rectangle_size>& _R) : R(_R) {}
		bool operator () (unsigned a, unsigned b) const
		{
			return R[a].width < R[b].width || (R[a].width == R[b].width && R[a].height < R[b].height);
		}
	};

	struct CompareLongerSideFirst
	{
		const std::vector<rectangle_size>& R;
		CompareLongerSideFirst(const std::vector<rectangle_size>& _R) : R(_R) {}
		bool operator () (unsigned a, unsigned b) const
		{
			return R[a].height < R[b].height || (R[a].height == R[b].height && R[a].width < R[b].width);
		}
	};

	struct CompareArea
	{
		const std::vector<rectangle_size>& R;
		CompareArea(const std::vector<rectangle_size>& _R) : R(_R) {}
		bool operator () (unsigned a, unsigned b) const
		{
			return R[a].width*R[a].height < R[b].width*R[b].height;
		}
	};

	struct ComparePerimeter
	{
		const std::vector<rectangle_size>& R;
		ComparePerimeter(const std::vector<rectangle_size>& _R) : R(_R) {}
		bool operator () (unsigned a, unsigned b) const
		{
			return R[a].width + R[a].height < R[b].width + R[b].height;
		}
	};

	struct CompareAspect
	{
		const std::vector<rectangle_size>& R;
		CompareAspect(const std::vector<rectangle_size>& _R) : R(_R) {}
		bool operator () (unsigned a, unsigned b) const
		{
			return R[a].width / R[a].height < R[b].width / R[b].height;
		}
	};

	struct CompareSideDifference
	{
		const std::vector<rectangle_size>& R;
		CompareSideDifference(const std::vector<rectangle_size>& _R) : R(_R) {}
		bool operator () (unsigned a, unsigned b) const
		{
			return abs(R[a].width - R[a].height) < abs(R[b].width - R[b].height);
		}
	};

	void compute_rectangle_permutation(
		const std::vector<rectangle_size>& rectangle_sizes,
		std::vector<unsigned>& permutation_out,
		CompareStrategy compare_strategy,
		bool sort_ascending)
	{
		std::vector<rectangle_size> R(rectangle_sizes);
		for (auto r : R)
			if (r.width > r.height)
				std::swap(r.width, r.height);

		permutation_out.resize(R.size());
		for (unsigned i = 0; i < permutation_out.size(); ++i)
			permutation_out[i] = i;
		switch (compare_strategy) {
		case CS_ShorterSideFirst: std::sort(permutation_out.begin(), permutation_out.end(), CompareShorterSideFirst(R)); break;
		case CS_LongerSideFirst:std::sort(permutation_out.begin(), permutation_out.end(), CompareLongerSideFirst(R)); break;
		case CS_Area:std::sort(permutation_out.begin(), permutation_out.end(), CompareArea(R)); break;
		case CS_Perimeter:std::sort(permutation_out.begin(), permutation_out.end(), ComparePerimeter(R)); break;
		case CS_Aspect:std::sort(permutation_out.begin(), permutation_out.end(), CompareAspect(R)); break;
		case CS_SideDifferent:std::sort(permutation_out.begin(), permutation_out.end(), CompareSideDifference(R)); break;
		}
		if (!sort_ascending)
			std::reverse(permutation_out.begin(), permutation_out.end());
	}

	unsigned suggest_output_size(
		const std::vector<rectangle_size>& rectangle_sizes,
		unsigned& width_out, unsigned& height_out,
		bool restrict_to_power_of_two,
		float percentual_safety)
	{
		// compute total area
		unsigned summed_rectangle_area = 0;
		for (auto r : rectangle_sizes)
			summed_rectangle_area += r.width*r.height;

		// increase area
		double output_area = (double)summed_rectangle_area * (1.0 + percentual_safety);

		double side_length = sqrt(double(output_area));

		if (restrict_to_power_of_two) {
			width_out = unsigned(pow(2.0, ceil(log(side_length) / log(2.0))));
			height_out = unsigned(pow(2.0, ceil(log(output_area / width_out) / log(2.0))));
		}
		else {
			width_out = unsigned(ceil(side_length));
			height_out = unsigned(ceil(output_area / width_out));
		}
		return summed_rectangle_area;
	}

	unsigned pack_rectangles(
		const std::vector<rectangle_size>& rectangle_sizes,
		const std::vector<unsigned>& permutation,
		std::vector<rectangle>& rectangles_out,
		unsigned width, unsigned height,
		PackingStrategy strategy,
		bool print_warnings)
	{
		rectangles_out.resize(rectangle_sizes.size());
		unsigned nr_failed = 0;

		switch (strategy) {
		case PS_Shelf:
		case PS_ShelfRefill:
		{
			rbp::ShelfBinPack bin(width, height, strategy == PS_ShelfRefill);
			for (auto i : permutation) {
				unsigned w = rectangle_sizes[i].width, h = rectangle_sizes[i].height;
				if (w > h)
					std::swap(w, h);
				rbp::Rect r = bin.Insert(w, h, rbp::ShelfBinPack::ShelfBestHeightFit);
				if (r.width*r.height == 0) {
					if (print_warnings)
						std::cout << "could not place " << rectangle_sizes[i].width << "x" << rectangle_sizes[i].height << std::endl;
					++nr_failed;
				}
				rectangle rect;
				rect.x = r.x;
				rect.y = r.y;
				rect.width = r.width;
				rect.height = r.height;
				rectangles_out[i] = rect;
			}
			break;
		}
		case PS_Skyline:
		case PS_SkylineRefill:
		{
			rbp::SkylineBinPack bin(width, height, strategy == PS_SkylineRefill);
			for (auto i : permutation) {
				unsigned w = rectangle_sizes[i].width, h = rectangle_sizes[i].height;
				if (w > h)
					std::swap(w, h);
				rbp::Rect r = bin.Insert(w, h, rbp::SkylineBinPack::LevelBottomLeft);
				if (r.width*r.height == 0) {
					if (print_warnings)
						std::cout << "could not place " << rectangle_sizes[i].width << "x" << rectangle_sizes[i].height << std::endl;
					++nr_failed;
				}
				rectangle rect;
				rect.x = r.x;
				rect.y = r.y;
				rect.width = r.width;
				rect.height = r.height;
				rectangles_out[i] = rect;
			}
			break;
		}
		case PS_Guillotine:
		{
			rbp::GuillotineBinPack bin(width, height);
			for (auto i : permutation) {
				unsigned w = rectangle_sizes[i].width, h = rectangle_sizes[i].height;
				if (w > h)
					std::swap(w, h);
				rbp::Rect r = bin.Insert(w, h, true, rbp::GuillotineBinPack::RectBestShortSideFit, rbp::GuillotineBinPack::SplitShorterAxis);
				if (r.width*r.height == 0) {
					if (print_warnings)
						std::cout << "could not place " << rectangle_sizes[i].width << "x" << rectangle_sizes[i].height << std::endl;
					++nr_failed;
				}
				rectangle rect;
				rect.x = r.x;
				rect.y = r.y;
				rect.width = r.width;
				rect.height = r.height;
				rectangles_out[i] = rect;
			}
			break;
		}
		case PS_MaxRectangle:
		{
			rbp::MaxRectsBinPack bin(width, height);
			for (auto i : permutation) {
				unsigned w = rectangle_sizes[i].width, h = rectangle_sizes[i].height;
				if (w > h)
					std::swap(w, h);
				rbp::Rect r = bin.Insert(w, h, rbp::MaxRectsBinPack::RectBestShortSideFit);
				if (r.width*r.height == 0) {
					if (print_warnings)
						std::cout << "could not place " << rectangle_sizes[i].width << "x" << rectangle_sizes[i].height << std::endl;
					++nr_failed;
				}
				rectangle rect;
				rect.x = r.x;
				rect.y = r.y;
				rect.width = r.width;
				rect.height = r.height;
				rectangles_out[i] = rect;
			}
			break;
		}
		}
		return nr_failed;
	}

	float pack_rectangles_iteratively(
		const std::vector<rectangle_size>& rectangle_sizes,
		unsigned& width_out, unsigned& height_out,
		std::vector<rectangle>& rectangles_out,
		CompareStrategy compare_strategy,
		bool sort_ascending,
		bool restrict_to_power_of_two,
		PackingStrategy strategy,
		bool print_warnings, bool print_progress)
	{
		std::vector<unsigned> rectangle_permutation;
		rectangles_out.clear();
		compute_rectangle_permutation(rectangle_sizes, rectangle_permutation, compare_strategy, sort_ascending);
		unsigned summed_rectangle_area;
		unsigned nr_failed;
		float percentual_safety = 2.5f / rectangle_sizes.size();
		do {
			percentual_safety *= 2;
			summed_rectangle_area = suggest_output_size(rectangle_sizes, width_out, height_out, restrict_to_power_of_two, percentual_safety);
			nr_failed = pack_rectangles(rectangle_sizes, rectangle_permutation, rectangles_out, width_out, height_out, strategy, print_warnings);
			if (nr_failed > 0 && print_progress) {
				std::cout << "*"; std::cout.flush();
			}
		} while (nr_failed > 0);
		return float(summed_rectangle_area) / (width_out * height_out);
	}

	bool save_svg(std::ofstream& os, unsigned width, unsigned height, const std::vector<rectangle>& rectangles)
	{
		os << "<svg width = \"" << width << "\" height = \"" << height << "\">" << std::endl;
		for (auto r : rectangles)
			os << "<rect x = \"" << r.x << "\" y = \"" << r.y << "\" width = \"" << r.width << "\" height = \"" << r.height << "\" style = \"fill:grey;stroke-width:1;stroke:black\" />" << std::endl;
		os << "</svg>" << std::endl;
		return !os.fail();
	}

	bool save_rectangles_html(const std::string& file_name, unsigned width, unsigned height, const std::vector<rectangle>& rectangles)
	{
		std::ofstream os(file_name);
		os << "<!DOCTYPE html>\n<html>\n<body>\n<div>" << std::endl;
		if (!save_svg(os, width, height, rectangles))
			return false;
		os << "\n</div>\n</body>\n</html>" << std::endl;
		return !os.fail();
	}

	void analyze_pack_rectangles_iteratively(
		const std::string& file_name_prefix,
		std::vector<rectangle_size>& rectangle_sizes,
		CompareStrategy compare_strategy,
		bool sort_ascending,
		bool restrict_to_power_of_two,
		PackingStrategy strategy,
		bool print_warnings,
		bool print_progress)
	{
		static const char* strategy_names[] = {
			"Shelf",
			"ShelfRefill",
			"Skyline",
			"SkylineRefill",
			"Guillotine",
			"MaxRectangle"
		};
		std::vector<rectangle> rectangles;
		unsigned width, height;
		std::cout << std::setw(13) << strategy_names[unsigned(strategy)] << ": "; std::cout.flush();
		std::chrono::time_point<std::chrono::system_clock> start, end;
		start = std::chrono::system_clock::now();
		float occupancy = pack_rectangles_iteratively(rectangle_sizes, width, height, rectangles, compare_strategy, sort_ascending, restrict_to_power_of_two, strategy, print_warnings, print_progress);
		end = std::chrono::system_clock::now();
		auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		std::cout << "\n" << width << "x" << height << " -> " << occupancy << " in " << elapsed_milliseconds << "ms\n" << std::endl;
		save_rectangles_html(file_name_prefix + strategy_names[unsigned(strategy)]+ ".html", width, height, rectangles);
	}

	void compare_packing_strategies(
		const std::string& file_name_prefix,
		std::vector<rectangle_size>& rectangle_sizes,
		CompareStrategy compare_strategy,
		bool sort_ascending,
		bool restrict_to_power_of_two,
		bool print_warnings,
		bool print_progress)
	{
		for (unsigned i=0; i<unsigned(PS_NrStrategies); ++i)
			analyze_pack_rectangles_iteratively(file_name_prefix, rectangle_sizes, compare_strategy, sort_ascending, restrict_to_power_of_two, PackingStrategy(i), print_warnings, print_progress);
	}
}