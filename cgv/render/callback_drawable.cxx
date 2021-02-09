#include "callback_drawable.h"

namespace cgv {
	namespace render {

callback_drawable::callback_drawable(const std::string& name) : node(name)
{
}
void callback_drawable::resize(unsigned w, unsigned h)
{
	resize_callback(w, h);
}
bool callback_drawable::init(cgv::render::context& ctx)
{
	return init_callback(ctx);
}
void callback_drawable::clear(cgv::render::context& ctx)
{
	clear_callback(ctx);
}
void callback_drawable::init_frame(cgv::render::context& ctx)
{
	init_frame_callback(ctx);
}
void callback_drawable::draw(cgv::render::context& ctx)
{
	draw_callback(ctx);
}
void callback_drawable::finish_draw(cgv::render::context& ctx)
{
	finish_draw_callback(ctx);
}
void callback_drawable::finish_frame(cgv::render::context& ctx)
{
	finish_frame_callback(ctx);
}
void callback_drawable::after_finish(cgv::render::context& ctx)
{
	after_finish_callback(ctx);
}
	}
}