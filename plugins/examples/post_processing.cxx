#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/box_render_data.h>
#include <cgv_gl/box_wire_render_data.h>
#include <cgv_gl/cone_render_data.h>
#include <cgv_gl/sphere_render_data.h>
#include <cgv_post/depth_halos.h>
#include <cgv_post/outline.h>
#include <cgv_post/screen_space_ambient_occlusion.h>
#include <cgv_post/temporal_anti_aliasing.h>

using namespace cgv::render;

class post_processing :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler
{
protected:
	view* view_ptr = nullptr;
	
	cgv::post::depth_halos dh;
	cgv::post::outline ol;
	cgv::post::screen_space_ambient_occlusion ssao;
	cgv::post::temporal_anti_aliasing taa;

	box_render_data<> boxes;
	box_wire_render_data<> wire_boxes;
	cone_render_data<> cones;
	sphere_render_data<> spheres;

	bool show_boxes;
	bool show_wire_boxes;
	bool show_cones;
	bool show_spheres;

	bool use_colors;
	bool use_illumination;
	
	
	
	float scale = 1.0f;



public:
	post_processing() : cgv::base::node("Post Processing Demo") {

		show_boxes = false;
		show_wire_boxes = false;
		show_cones = true;
		show_spheres = true;

		use_colors = true;
		use_illumination = true;

		boxes.style.material.set_diffuse_reflectance(cgv::rgb(1.0f));
		wire_boxes.style.default_color = cgv::rgb(1.0f);
		cones.style.material.set_diffuse_reflectance(cgv::rgb(1.0f));
		spheres.style.material.set_diffuse_reflectance(cgv::rgb(1.0f));

		cones.style.radius_scale = 0.05f;
		spheres.style.radius_scale = 0.1f;
	}

	void stream_help(std::ostream& os) {
		return;
	}

	bool handle(cgv::gui::event& e) {
		// return false because we do not handle any events here
		return false;
	}

	void on_set(void* member_ptr) {
		if(member_ptr == &use_colors) {
			cgv::render::ColorMapping cm = use_colors ? cgv::render::CM_COLOR_FRONT : cgv::render::CM_NONE;

			boxes.style.map_color_to_material = cm;
			cones.style.map_color_to_material = cm;
			spheres.style.map_color_to_material = cm;
		}

		if(member_ptr == &use_illumination) {
			cgv::render::IlluminationMode im = use_illumination ? cgv::render::IM_ONE_SIDED : cgv::render::IM_OFF;

			boxes.style.illumination_mode = im;
			cones.style.illumination_mode = im;
			spheres.style.illumination_mode = im;
		}

		// reset temporal anti aliasing when changes occur, to restart the accumulation and prevent ghosting artifacts
		taa.reset();

		post_redraw();
		update_member(member_ptr);
	}

	std::string get_type_name() const {
		return "post_processing_demo";
	}

	void clear(cgv::render::context& ctx) {
		// post processing effects need to be destructed to free created resources
		dh.destruct(ctx);
		ol.destruct(ctx);
		ssao.destruct(ctx);
		taa.destruct(ctx);
	}

	bool init(cgv::render::context& ctx) {
		bool success = true;

		// all post_processing effects need to be initialized to create internally used objects and buffers
		success &= dh.init(ctx);
		success &= ol.init(ctx);
		success &= ssao.init(ctx);
		success &= taa.init(ctx);

		// initialize render data objects
		success &= boxes.init(ctx);
		success &= wire_boxes.init(ctx);
		success &= cones.init(ctx);
		success &= spheres.init(ctx);

		if(success)
			generate_geometry();

		// setup some scene specific parameters
		dh.set_strength(2.0f);
		dh.set_depth_scale(0.3f);

		return success;
	}

	void init_frame(cgv::render::context& ctx) {
		if(!view_ptr && (view_ptr = find_view_as_node())) {
			// temporal anti aliasing needs access to the current view
			taa.set_view(view_ptr);
		}

		// call ensure in init_frame on post processing effects to make sure the internal buffers are created and sized accordingly
		dh.ensure(ctx);
		ol.ensure(ctx);
		ssao.ensure(ctx);
		taa.ensure(ctx);
	}

	void draw(cgv::render::context& ctx) {
		// TODO: fxaa produces black backgorund

		// Post processing effects need to encapsulate the draw calls of the geometry to which they shall be applied.
		// Call begin() before drawing the main geometry to enable the post process framebuffers.
		taa.begin(ctx);
		dh.begin(ctx);
		ol.begin(ctx);
		ssao.begin(ctx);

		ctx.push_modelview_matrix();

		cgv::mat4 M(0.0f);
		M(0, 0) = scale;
		M(1, 1) = scale;
		M(2, 2) = scale;
		M(3, 3) = 1.0f;

		ctx.mul_modelview_matrix(M);

		// Now draw geometry
		if(show_boxes)
			boxes.render(ctx);
		if(show_wire_boxes)
			wire_boxes.render(ctx);
		if(show_cones)
			cones.render(ctx);
		if(show_spheres)
			spheres.render(ctx);

		ctx.pop_modelview_matrix();

		// Call end() when rendering is finished to apply the effect and blit the result to the main framebuffer
		// Attention! When using multiple effects they must be ended in reverse order.
		ssao.end(ctx);
		ol.end(ctx);
		dh.end(ctx);
		taa.end(ctx);
	}

	void generate_geometry() {
		// clear the previous geometry, which will also mark it as out of date and induce a transfer to the GPU buffers
		boxes.clear();
		wire_boxes.clear();
		cones.clear();
		spheres.clear();

		int n = 1000;
		std::mt19937 rng(2845762);
		std::uniform_real_distribution<float> snorm_distr(-1.0f, 1.0f);
		std::uniform_real_distribution<float> unorm_distr(0.0f, 1.0f);

		for(int i = 0; i < n; ++i) {
			cgv::vec3 pos(
				snorm_distr(rng),
				snorm_distr(rng),
				snorm_distr(rng)
			);

			cgv::vec3 dir(
				snorm_distr(rng),
				snorm_distr(rng),
				snorm_distr(rng)
			);
			dir.normalize();

			cgv::vec3 ext(
				unorm_distr(rng),
				unorm_distr(rng),
				unorm_distr(rng)
			);
			ext = 0.2f * ext + 0.1f;

			cgv::rgb col(
				unorm_distr(rng),
				unorm_distr(rng),
				unorm_distr(rng)
			);

			boxes.add(pos, ext, col);

			cgv::rgb inv_col(
				1.0f - col.R(),
				1.0f - col.G(),
				1.0f - col.B()
			);
			wire_boxes.add(pos, ext, inv_col);
			
			spheres.add(pos, col, cgv::math::lerp(0.5f, 1.0f, unorm_distr(rng)));

			cones.add(pos, pos + 0.25f*(0.95f*unorm_distr(rng) + 0.05f)*dir, inv_col, cgv::math::lerp(0.5f, 1.0f, unorm_distr(rng)));
		}
	}

	void create_gui() {
		add_decorator("Post Processing Demo", "heading");

		add_decorator("Objects", "heading", "level=3");
		add_member_control(this, "Boxes", show_boxes, "check");
		add_member_control(this, "Wire Boxes", show_wire_boxes, "check");
		add_member_control(this, "Cones", show_cones, "check");
		add_member_control(this, "Spheres", show_spheres, "check");

		add_member_control(this, "Color", use_colors, "toggle");
		add_member_control(this, "Illumination", use_illumination, "toggle");

		add_member_control(this, "Scene Scale", scale, "value_slider", "min=0;max=10;step=0.01");

		// TODO: add gui creators
		if(begin_tree_node("TAA", taa, false)) {
			align("\a");
			//add_gui("", taa);
			taa.create_gui(this);
			align("\b");
			end_tree_node(taa);
		}

		if(begin_tree_node("SSAO", ssao, false)) {
			align("\a");
			//add_gui("", ssao);
			ssao.create_gui(this);
			align("\b");
			end_tree_node(ssao);
		}

		if(begin_tree_node("Depth Halos", dh, false)) {
			align("\a");
			//add_gui("", dh);
			dh.create_gui(this);
			align("\b");
			end_tree_node(dh);
		}

		if(begin_tree_node("Outline", ol, false)) {
			align("\a");
			//add_gui("", dh);
			ol.create_gui(this);
			align("\b");
			end_tree_node(ol);
		}
	}
};

#include <cgv/base/register.h>

/// register a factory to create new post processing demos
cgv::base::factory_registration<post_processing> post_processing_fac("New/Demo/Post Processing");
