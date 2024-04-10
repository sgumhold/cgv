#pragma once

#include <cgv/base/node.h>
#include <cgv/math/fvec.h>
#include <cgv/media/color.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/data/data_view.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/texture.h>
#include <cgv_gl/point_renderer.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv_gl/surfel_renderer.h>
#include <cgv_gl/clod_point_renderer.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/arrow_renderer.h>

#include <point_cloud.h>
#include <octree.h>

#include <string>
#include <mutex>
#include <future>
#include <unordered_set>

// these are the vr specific headers
#include <vr/vr_driver.h>
#include <cg_vr/vr_server.h>
#include <vr_view_interactor.h>
#include <vr_render_helpers.h>

#include "lib_begin.h"

enum class LoDMode {
	OCTREE = 1,	//good quality lods
	RANDOM_POISSON = 2, //lower quality but generation is way faster
	INVALID = -1
};

enum class point_label {
	DELETED = 0,
	VISIBLE = 1,
	UNDEFINED = -1
};

struct point_labeling_intend {
	cgv::vec3 position;
	float radius;
	int label;
};

// this is a VR application using the clod_point_renderer for demonstration purposes
class pointcloud_lod_render_test :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
public:
	using vec3 = cgv::vec3;
	using vec4 = cgv::vec4;
	using mat3 = cgv::mat3;
	using mat4 = cgv::mat4;
	using dmat4 = cgv::dmat4;
	using quat = cgv::quat;
	using rgb = cgv::rgb;
	using rgb8 = cgv::rgb8;
	using rgba = cgv::rgba;
	using box3 = cgv::box3;


	//use internal point format
	using LODPoint = cgv::render::clod_point_renderer::Point;

	pointcloud_lod_render_test();

	/// overload to return the type name of this object. By default the type interface is queried over get_type.
	std::string get_type_name() const { return "pointcloud_lod_render_test"; }

	bool self_reflect(cgv::reflect::reflection_handler& rh);

	void on_set(void* member_ptr);
	
	void on_register();
	
	void unregister();

	bool init(cgv::render::context& ctx);

	void init_frame(cgv::render::context& ctx);
	/// overload to draw the content of this drawable
	void draw(cgv::render::context& ctx);
	///
	void find_pointcloud(cgv::render::context& ctx);
	///
	void clear(cgv::render::context& ctx);
	/// 
	bool handle(cgv::gui::event& e);
	/// 
	void stream_help(std::ostream& os);
	///
	void create_gui();

protected:
	void timer_event(double t, double dt);

	void on_load_point_cloud_cb();
	void on_clear_point_cloud_cb();
	void on_rotate_x_cb();
	void rotate_pc_x(const float angle);
	void rotate_pc_y(const float angle);
	void on_rotate_y_cb();
	void on_rotate_z_cb();
	void on_randomize_position_cb();
	void on_reg_find_point_cloud_cb();
	void on_point_cloud_style_cb();
	void on_lod_mode_change();
	void on_random_labels();

	void construct_table(float tw, float td, float th, float tW);
	void construct_room(float w, float d, float h, float W, bool walls, bool ceiling);
	void construct_environment(float s, float ew, float ed, float w, float d, float h);
	void build_scene(float w, float d, float h, float W, float tw, float td, float th, float tW);
	void clear_scene();
	void build_test_object_32();

	/// @param[in]	reduced_points opengl buffer storing points
	/// @param[in]	indices_reduced_points opengl buffer storing position in point_label_buffer associated with the point in reduced_points
	void label_points(cgv::render::context& ctx, GLint label, const vec3 position, const float radius,
		GLuint reduced_points, GLuint reduced_points_indices, const unsigned num_reduced_points);

	point_cloud build_test_point_cloud(int x, int y, int z, int grid_size,float cube_size) {
		double dgrid_size = grid_size;
		point_cloud pc;
		pc.create_colors();
		for (int x = 0; x < grid_size; ++x) {
			for (int y = 0; y < grid_size; ++y) {
				for (int z = 0; z < grid_size; ++z) {
					double dx = x, dy = y, dz = z;
					int i = std::max(std::max(x, y), z)+1;
					vec3 v(dx, dy, dz);
					v *= cube_size / grid_size;
					while (i > 0) {
						pc.add_point(v);
						pc.clr(pc.get_nr_points() - 1) = rgb8(i);
						--i;
					}
				}
			}
		}
		return std::move(pc);
	}

private:
	struct vertex {
		vec3  position;
		float radius;
		rgba  color;
	};
	std::unordered_set<void*> rebuild_ptrs;

	std::string ply_path;
	point_cloud source_pc;
	cgv::render::point_render_style source_prs;
	cgv::render::surfel_render_style source_srs;
	cgv::render::cone_render_style rcrs;
	cgv::render::sphere_render_style srs;
	cgv::render::arrow_render_style ars;

	size_t max_points = -1;

	float rot_intensity;
	float trans_intensity;
	bool view_find_point_cloud;
	bool renderer_out_of_date = true;
	bool recolor_point_cloud = true;
	
	bool pointcloud_fit_table;
	bool put_on_table;
	bool color_based_on_lod;
	bool show_environment;

	bool gui_culling_protection_zone = false;
	float culling_protection_zone_radii[2];
	vec3 culling_protection_zone_positions[2];
	static constexpr int num_culling_protection_zones = 2;

	static constexpr float min_level_hue = 230.0/360.0;
	static constexpr float max_level_hue = 1.0;
	
	int lod_mode = (int)LoDMode::OCTREE;
	
	bool gui_model_positioning = false;
	vec3 model_position= vec3(0);
	vec3 model_rotation = vec3(0);
	float model_scale = 1.f;
	dmat4 model_transform;

	cgv::render::clod_point_render_style cp_style;
	cgv::render::cone_render_style cone_style;

	// environment geometry
	std::vector<box3> boxes;
	std::vector<rgb> box_colors;
	cgv::render::box_render_style style;
	constexpr static float table_height = 0.7f;

	// octree base lod generator
	cgv::pointcloud::octree_lod_generator<LODPoint> lod_generator;
	// storage for generated lod points
	std::vector<LODPoint> points_with_lod;

	vr_view_interactor* vr_view_ptr;

	vec3 pos;
	mat3 ori;
	vec3 c_pos;

	vertex p;

	//coordinate of the controller
	std::vector<vertex> coordinate_c;

	std::vector<vec3> points;
	std::vector<rgb> colors;
	std::vector<vec3> normals;

	std::vector<vec3> forward_points;
	std::vector<rgb> forward_colors;
	std::vector<vec3> forward_normals;

	// alternative draw programm for labeled points
	cgv::render::shader_program custom_draw_prog, labeling_prog;
	std::array<float,2> labeling_radius;
	bool use_label_prog = false;
	std::vector<GLint> point_labels;
	GLuint point_label_buffer = 0;
	bool init_label_buffer = true;


	// queue for actions
	std::vector<point_labeling_intend> queued_actions;
};

#include <cgv/config/lib_end.h>