#pragma once

#include <opencl.h>
#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv/render/texture.h>
#include <cgv/gui/provider.h>


class opengl_opencl_interop : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider
{
protected: 
	cgv::compute::opencl::opencl_base_ptr ocl;
	cgv::render::texture                  tex;
	cl::Image2DGL                         img;
public:
	opengl_opencl_interop(const std::string& _name);

	std::string get_type_name() const;
	
	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
	void clear(cgv::render::context& ctx);

	void create_gui();
};

