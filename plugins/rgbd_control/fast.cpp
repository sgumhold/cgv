#include <stdlib.h>
#include "fast.h"
namespace fast {

xy* fast9_detect_nonmax(const fast_byte* im, int xsize, int ysize, int stride, int b, int* ret_num_corners)
{
	xy* corners;
	int num_corners;
	int* scores;
	xy* nonmax;

	corners = fast9_detect(im, xsize, ysize, stride, b, &num_corners);
	scores = fast9_score(im, stride, corners, num_corners, b);
	nonmax = nonmax_suppression(corners, scores, num_corners, ret_num_corners);

	free(corners);
	free(scores);

	return nonmax;
}

xy* fast10_detect_nonmax(const fast_byte* im, int xsize, int ysize, int stride, int b, int* ret_num_corners)
{
	xy* corners;
	int num_corners;
	int* scores;
	xy* nonmax;

	corners = fast10_detect(im, xsize, ysize, stride, b, &num_corners);
	scores = fast10_score(im, stride, corners, num_corners, b);
	nonmax = nonmax_suppression(corners, scores, num_corners, ret_num_corners);

	free(corners);
	free(scores);

	return nonmax;
}

xy* fast11_detect_nonmax(const fast_byte* im, int xsize, int ysize, int stride, int b, int* ret_num_corners)
{
	xy* corners;
	int num_corners;
	int* scores;
	xy* nonmax;

	corners = fast11_detect(im, xsize, ysize, stride, b, &num_corners);
	scores = fast11_score(im, stride, corners, num_corners, b);
	nonmax = nonmax_suppression(corners, scores, num_corners, ret_num_corners);

	free(corners);
	free(scores);

	return nonmax;
}

xy* fast12_detect_nonmax(const fast_byte* im, int xsize, int ysize, int stride, int b, int* ret_num_corners)
{
	xy* corners;
	int num_corners;
	int* scores;
	xy* nonmax;

	corners = fast12_detect(im, xsize, ysize, stride, b, &num_corners);
	scores = fast12_score(im, stride, corners, num_corners, b);
	nonmax = nonmax_suppression(corners, scores, num_corners, ret_num_corners);

	free(corners);
	free(scores);

	return nonmax;
}

void pyramid(const rgbd::frame_type img, int xsize, int ysize, float scalar, int nlevel) {
	std::vector<rgbd::frame_type> imgs;
	imgs.resize(nlevel);

	for (int i = 0; i <nlevel; ++i) {
		// prepare grayscale frame
		imgs.at(i).width = img.width / pow(scalar, i);
		imgs.at(i).height = img.height / pow(scalar, i);
		imgs.at(i).pixel_format = img.pixel_format;
		imgs.at(i).nr_bits_per_pixel = img.nr_bits_per_pixel;
		imgs.at(i).compute_buffer_size();
		imgs.at(i).frame_data.resize(imgs.at(i).buffer_size);

		/*for (int j = 0; j < imgs.at(i).width;++j) {
			for (int k = 0; k < imgs.at(i).height;++k) {
			    
			}
		}*/
	}
}
} // namespace fast