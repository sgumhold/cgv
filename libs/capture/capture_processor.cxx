#include "capture_processor.h"

namespace capture {

const captured_image* frame_provider::get_color_image() const
{
	return get_color_frame();
}

const captured_image* frame_provider::get_infrared_image() const
{
	return get_infrared_frame();
}

const captured_image* frame_provider::get_depth_image() const
{
	return get_depth_frame();
}

const captured_frame* frame_provider::get_color_frame() const
{
	return 0;
}

const captured_frame* frame_provider::get_infrared_frame() const
{
	return 0;
}


const captured_frame* frame_provider::get_depth_frame() const
{
	return 0;
}

}
