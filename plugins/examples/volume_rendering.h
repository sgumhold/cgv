#pragma once

#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/render_types.h>
#include <cgv_gl/volume_renderer.h>
#include <cgv_glutil/application_plugin.h>
#include <cgv_glutil/box_wire_render_data.h>
#include <cgv_glutil/color_map.h>
#include <cgv_glutil/color_map_editor.h>
#include <cgv_glutil/color_map_legend.h>

class volume_viewer :
	public cgv::glutil::application_plugin // inherit from application plugin to enable overlay support
{
private:
	bool do_calculate_gradients;

protected:
	/// store a pointer to the color map editor overlay which is used to edit the volume transfer function
	cgv::glutil::color_map_editor_ptr transfer_function_editor_ptr;
	cgv::glutil::color_map_legend_ptr transfer_function_legend_ptr;

	/// resolution of the volume
	uvec3 vres;
	/// whether to show bounding box
	bool show_box;

	// Volume data
	std::vector<float> vol_data;
	box3 volume_bounding_box;
	cgv::render::texture volume_tex;
	
	// Render members
	/// store a pointer to the view
	cgv::render::view* view_ptr;
	/// render style for volume
	cgv::render::volume_render_style vstyle;
	/// using a color map to define the volume transfer function
	/// gl_color_map supports generation of a texture from its contents
	cgv::glutil::gl_color_map transfer_function;
	/// render data for wireframe box
	cgv::glutil::box_wire_render_data<> box_rd;
	
	void create_volume(cgv::render::context& ctx);
	void splat_spheres(std::vector<float>& vol_data, float voxel_size, std::mt19937& rng, size_t n, float radius, float contribution);
	void splat_sphere(std::vector<float>& vol_data, float voxel_size, const vec3& pos, float radius, float contribution);

	void load_volume_from_file(const std::string& file_name);

	void create_histogram();

public:
	// default constructor
	volume_viewer();
	
	// interface of cgv::base::node
	std::string get_type_name() const { return "volume_viewer"; }
	void stream_stats(std::ostream& os);
	void on_set(void* member_ptr);
	bool self_reflect(cgv::reflect::reflection_handler& _rh);

	// interface of cgv::gui::event_handler
	void stream_help(std::ostream& os);
	bool handle_event(cgv::gui::event& e);

	// interface of cgv::render::drawable
	void clear(cgv::render::context& ctx);
	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);

	// interface of provider
	void create_gui();
};
