#include <rect_pack/rect_pack.h>


int main(int argc, char** argv)
{
	std::vector<rect_pack::rectangle_size> rectangle_sizes;
	rect_pack::construct_random_rectangles(1401, rectangle_sizes);
	rect_pack::compare_packing_strategies("rect_pack_", rectangle_sizes, rect_pack::CS_ShorterSideFirst, false, true, false, true);
}
