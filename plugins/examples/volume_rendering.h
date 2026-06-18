#pragma once

#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/media/transfer_function.h>
#include <cgv/render/color_scale_adapter.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/volume_renderer.h>
#include <cgv_gl/box_wire_render_data.h>
#include <cgv_overlay/color_scale_legend.h>
#include <cgv_overlay/color_selector.h>
#include <cgv_overlay/transfer_function_editor.h>

class volume_viewer :
	public cgv::base::group,			// derive from group to support child nodes (needed for overlays)
	public cgv::gui::event_handler,		// derive from event handler to receive input events
	public cgv::gui::provider,			// derive from gui provider to have gui controls
	public cgv::render::drawable		// derive from drawable to allow drawing in the GL context
{
protected:
	/// a transfer funciton editor is used to edit the volume transfer function
	cgv::overlay::transfer_function_editor_ptr transfer_function_editor;
	/// a color scale legend is used to show the volume transfer function
	cgv::overlay::color_scale_legend_ptr legend;
	/// a color selector is used to allow live selection of transfer function colors
	cgv::overlay::color_selector_ptr color_selector;

	/// resolution of the volume
	cgv::uvec3 vres;
	/// spacing of the voxels
	cgv::vec3 vspacing;
	/// whether to show bounding box
	bool show_box;

	// Volume data
	std::vector<float> vol_data;
	cgv::box3 volume_bounding_box;
	cgv::render::texture volume_tex;
	cgv::render::texture depth_tex;

	// Render members
	/// store a pointer to the view
	cgv::render::view* view_ptr;
	/// render style for volume
	cgv::render::volume_render_style vstyle;
	/// index of the transfer function preset
	int transfer_function_preset = 1;
	/// the volume transfer function
	std::shared_ptr<cgv::media::transfer_function> transfer_function = std::make_shared<cgv::media::transfer_function>();
	cgv::render::color_scale_adapter color_scale_adapter;
	/// render data for wireframe box
	cgv::render::box_wire_render_data<> box_rd;
	
	void update_bounding_box();

	void load_transfer_function_preset();

	void create_volume(cgv::render::context& ctx);
	void splat_spheres(std::vector<float>& vol_data, float voxel_size, std::mt19937& rng, size_t n, float radius, float contribution);
	void splat_sphere(std::vector<float>& vol_data, float voxel_size, const cgv::vec3& pos, float radius, float contribution);

	void load_volume_from_file(const std::string& file_name);

	void fit_to_resolution();
	void fit_to_spacing();
	void fit_to_resolution_and_spacing();

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
	bool handle(cgv::gui::event& e);

	// interface of cgv::render::drawable
	void clear(cgv::render::context& ctx);
	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
	void after_finish(cgv::render::context& ctx);

	// interface of provider
	void create_gui();
};
