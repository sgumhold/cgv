#include "render_buffer.h"

namespace cgv {
	namespace render {

render_buffer::render_buffer(const std::string& description)
	: cgv::data::component_format(description)
{
	width = -1;
	height = -1;
	handle = 0;
}

render_buffer::~render_buffer()
{
	if (handle) {
		if (ctx_ptr)
			destruct(*ctx_ptr);
		else
			std::cerr << "could not destruct render buffer in destructor" << std::endl;
	}
}

void render_buffer::destruct(context& ctx)
{
	if (handle) {
		ctx.render_buffer_destruct(*this);
	}
}

bool render_buffer::is_created() const
{
	return handle != 0;
}

void render_buffer::create(context& ctx, int _width, int _height)
{
	if (ctx_ptr)
		destruct(*ctx_ptr);
	else
		destruct(ctx);
	ctx.render_buffer_create(*this, *this, _width, _height);
	ctx_ptr = &ctx;
	width = _width;
	height = _height;
}

bool render_buffer::is_depth_buffer() const
{
	return get_standard_component_format() == cgv::data::CF_D;
}

bool render_buffer::is_color_buffer() const
{
	return get_standard_component_format() == cgv::data::CF_RGB || 
			 get_standard_component_format() == cgv::data::CF_RGBA;
}


	}
}