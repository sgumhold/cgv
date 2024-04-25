#pragma once

#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv_gl/arrow_renderer.h>
#include <cgv_gl/volume_renderer.h>
#include <cgv/render/shader_program.h>

class gradient_viewer :
	public cgv::base::node, 
	public cgv::render::drawable, 
	public cgv::gui::provider, 
	public cgv::gui::event_handler
{
private:
	bool do_calculate_gradients;

protected:
	/// shape of volume content
	enum Shape {
		S_ELLIPSOID,
		S_CUBOID
	} shape;

	/// gradient computation mode
	enum GradientMode {
		GM_CENTRAL,
		GM_SOBEL,
		GM_SOBEL_OPTIMIZED
	} gradient_mode;

	/// resolution of the volume
	cgv::uvec3 vres;
	/// whether to show volume
	bool show_volume;
	/// whether to show gradients
	bool show_gradients;

	// Gradient data
	std::vector<cgv::vec3> positions;
	std::vector<cgv::vec3> directions;
	std::vector<cgv::rgb> colors;

	// Volume rendering
	cgv::box3 volume_bounding_box;
	cgv::render::texture tf_tex;
	cgv::render::texture volume_tex;
	cgv::render::texture gradient_tex;
	cgv::render::texture depth_tex;

	// Render members
	cgv::render::view* view_ptr;
	cgv::render::volume_render_style vstyle;
	cgv::render::arrow_render_style astyle;
	
	// Compute shaders
	cgv::render::shader_program gradient_3d_prog;
	
	void create_test_volume(cgv::render::context& ctx);
	void calculate_gradient_texture(cgv::render::context& ctx);

public:
	// default constructor
	gradient_viewer();
	
	// interface of cgv::base::node
	std::string get_type_name() const { return "gradient_viewer"; }
	void stream_stats(std::ostream& _os);
	void on_set(void* member_ptr);
	bool self_reflect(cgv::reflect::reflection_handler& _rh);

	// interface of cgv::gui::event_handler
	void stream_help(std::ostream& _os);
	bool handle(cgv::gui::event& _e);

	// interface of cgv::render::drawable
	void clear(cgv::render::context& ctx);
	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
	void after_finish(cgv::render::context& ctx);

	// interface of provider
	void create_gui();
};
