#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <cgv/math/ftransform.h>
#include <cgv/media/image/image.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/media/mesh/simple_mesh.h>
#include <cgv/media/illum/textured_surface_material.h>
#include <cgv/render/drawable.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/shader_library.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_gl/sphere_render_data.h>

using namespace cgv::render;

class environment_demo :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler
{
protected:
	typedef cgv::math::fvec<signed char, 4u> cvec4;

	view* view_ptr;

	shader_library shaders;

	unsigned shadow_map_resolution = 4*256u;
	texture depth_map;
	texture color_map;
	frame_buffer depth_map_fb;

	cgv::vec2 sun_position;

	sphere_render_data<> spheres;
	sphere_render_style sphere_style;

	texture environment_map;
	texture irradiance_map;
	texture prefiltered_specular_map;
	texture brdf_lut;

	float lod = 0.0f;

	float roughness = 1.0f;
	cgv::rgb F0 = cgv::rgb(1.0f);

	mesh_render_info box_mesh_info, obj_mesh_info;

	bool show_shadow_map = false;

	float near_plane = 1.0f, far_plane = 10.0f;

	float shadow_blur = 2.0f;

	texture jitter_tex;

	unsigned environment_resolution = 512u;
	unsigned irradiance_resolution = 32u;
	unsigned prefiltered_specular_resolution = 128u;

	struct matrix_stack {
		std::stack<cgv::mat4> s;

		matrix_stack() { init(); }

		void init() {
			cgv::mat4 M;
			M.identity();
			s.push(M);
		}

		void push() { s.push(s.top()); }

		void pop() {
			s.pop();
			if(s.empty())
				init();
		}

		void set(const cgv::mat4& M) { s.top() = M; }

		const cgv::mat4& get() { return s.top(); }

		void mul(const cgv::mat4& M) { set(get() * M); }
	};

	struct render_context {
		context* ctx = nullptr;
		cgv::mat4 view_matrix;
		cgv::mat4 light_matrix;

		void store_view() {
			// if not previously altered the modelview matrix is just the view matrix
			view_matrix = ctx->get_modelview_matrix();
		}

		cgv::mat3 get_normal_matrix(const cgv::mat4& M) {
			cgv::math::fmat<float, 3, 3> NM;
			NM(0, 0) = M(0, 0);
			NM(0, 1) = M(0, 1);
			NM(0, 2) = M(0, 2);
			NM(1, 0) = M(1, 0);
			NM(1, 1) = M(1, 1);
			NM(1, 2) = M(1, 2);
			NM(2, 0) = M(2, 0);
			NM(2, 1) = M(2, 1);
			NM(2, 2) = M(2, 2);
			NM.transpose();
			NM = inv(NM);
			return NM;
		}
	};

	struct scene_object {
		cgv::vec3 position = cgv::vec3(0.0f);
		cgv::vec3 rotation = cgv::vec3(0.0f);
		cgv::vec3 scale = cgv::vec3(1.0f);
		cgv::vec2 uv_scale = cgv::vec2(1.0f);

		cgv::mat4 transformation_matrix;

		mesh_render_info mri;

		texture albedo_tex, metallic_tex, roughness_tex, normal_tex;

		scene_object() {
			transformation_matrix.identity();
		}

		void compute_transformation() {
			transformation_matrix =
				cgv::math::translate4(position) *
				cgv::math::rotate4(rotation) *
				cgv::math::scale4(scale);
		}

		void draw(render_context& rctx, shader_program& shader) {
			context& ctx = *rctx.ctx;
			
			shader.enable(ctx);
			shader.set_uniform(ctx, "model_matrix", transformation_matrix);
			shader.set_uniform(ctx, "model_normal_matrix", rctx.get_normal_matrix(transformation_matrix));
			shader.set_uniform(ctx, "light_space_matrix", rctx.light_matrix * transformation_matrix);
			shader.set_uniform(ctx, "uv_scale", uv_scale);
			shader.disable(ctx);

			ctx.set_modelview_matrix(rctx.view_matrix * transformation_matrix);

			albedo_tex.enable(ctx, 5);
			metallic_tex.enable(ctx, 6);
			roughness_tex.enable(ctx, 7);
			normal_tex.enable(ctx, 8);
			
			mri.bind(ctx, shader, true);
			mri.draw_all(ctx, false, false, false);
			
			albedo_tex.disable(ctx);
			metallic_tex.disable(ctx);
			roughness_tex.disable(ctx);
			normal_tex.disable(ctx);
			
		}

		void draw_depth(render_context& rctx, shader_program& shader) {
			context& ctx = *rctx.ctx;
			
			shader.enable(ctx);
			shader.set_uniform(ctx, "light_space_matrix", rctx.light_matrix * transformation_matrix);
			shader.disable(ctx);

			mri.bind(ctx, shader, true);
			mri.draw_all(ctx, false, false, false);
		}
	};

	scene_object floor, obj;

	render_context rctx;

	bool animate = false;
	bool show_environment = true;
	float rot_angle = 0.0f;
	float height_offset = 0.0f;

	float normal_map_scale = 1.0f;

public:
	environment_demo() : cgv::base::node("Environment Demo") {
		view_ptr = nullptr;
		shaders.add("cubemap", "cubemap.glpr");
		shaders.add("sky_cubemap_gen", "sky_cubemap_gen.glpr");
		shaders.add("irradiance_map_gen", "irradiance_map_gen.glpr");
		shaders.add("prefiltered_specular_map_gen", "prefiltered_specular_map_gen.glpr");
		shaders.add("brdf_lut_gen", "brdf_lut_gen.glpr");
		shaders.add("screen", "screen.glpr");

		shader_define_map defines;
		shader_code::set_define(defines, "ENABLE_TEXTURES", true, false);

		shaders.add("pbr_surface", "pbr_surface.glpr");
		shaders.add("pbr_surface_textured", "pbr_surface.glpr", defines);
		shaders.add("surface_depth", "surface_depth.glpr");

		sun_position = cgv::vec2(0.0f, 0.6f);

		sphere_style.surface_color = cgv::rgb(0.5f);
		sphere_style.radius = 0.02f;

		cgv::signal::connect(cgv::gui::get_animation_trigger().shoot, this, &environment_demo::timer_event);
	}
	void stream_help(std::ostream& os) {
		return;
	}
	bool handle(cgv::gui::event& e) {
		return false;
	}
	void on_set(void* member_ptr) {
		
		if(member_ptr == &sun_position[0] || member_ptr == &sun_position[1]) {
			context& ctx = *get_context();
			generate_ibl_maps(ctx);
		}

		if(member_ptr == &show_shadow_map) {
			depth_map.set_compare_mode(!show_shadow_map);
		}

		post_redraw();
		update_member(member_ptr);
	}
	std::string get_type_name() const {
		return "environment_demo";
	}
	void clear(cgv::render::context& ctx) {
		ref_sphere_renderer(ctx, -1);
		spheres.destruct(ctx);

		shaders.clear(ctx);
	}
	bool init(cgv::render::context& ctx) {
		ref_sphere_renderer(ctx, 1);

		bool success = true;
		success &= shaders.load_all(ctx);
		
		if(spheres.init(ctx)) {
			spheres.add(cgv::vec3(0.0f, -50.0f, 0.0f), 50.0f);
			spheres.add(cgv::vec3(0.0f, 0.5f, 0.0f), 0.5f);
			spheres.add(cgv::vec3(0.1f, 1.2f, 0.0f), 0.2f);
			spheres.set_out_of_date();
		} else {
			success = false;
		}

		// load mesh files to use as scene objects
		cgv::media::mesh::simple_mesh<> box_mesh, obj_mesh;

		if(getenv("CGV_DIR")) {
			if(box_mesh.read(std::string(getenv("CGV_DIR")) + "/plugins/examples/res/box.obj")) {
				box_mesh.compute_face_tangents();
				floor.mri.construct(ctx, box_mesh);
				floor.position = cgv::vec3(0.0f, -1.0f, 0.0f);
				floor.scale = cgv::vec3(10.0f, 0.2f, 10.0f);
				floor.compute_transformation();
				floor.uv_scale = cgv::vec2(4.0f);
			}

			if(obj_mesh.read(std::string(getenv("CGV_DIR")) + "/plugins/examples/res/blob.obj")) {
				obj_mesh.compute_face_tangents();
				obj.mri.construct(ctx, obj_mesh);
				obj.uv_scale = cgv::vec2(2.0f);
			}
		}

		depth_map.set_data_format("uint16[D]");
		depth_map.create(ctx, TT_2D, shadow_map_resolution, shadow_map_resolution);
		depth_map.set_wrap_s(TW_CLAMP_TO_BORDER);
		depth_map.set_wrap_t(TW_CLAMP_TO_BORDER);
		depth_map.set_min_filter(TF_LINEAR);
		depth_map.set_mag_filter(TF_LINEAR);
		depth_map.set_border_color(1.0f, 1.0f, 1.0f, 1.0f);
		depth_map.set_compare_mode(!show_shadow_map);

		depth_map_fb.create(ctx, shadow_map_resolution, shadow_map_resolution);
		depth_map_fb.attach(ctx, depth_map);
		
		

		// TODO: use framework textured surface materialto handle textures?
		success &= read_texture(ctx, floor.albedo_tex,	 "res://octostone_albedo.png");
		success &= read_texture(ctx, floor.metallic_tex, "res://octostone_metallic.png");
		success &= read_texture(ctx, floor.roughness_tex,"res://octostone_roughness.png");
		success &= read_texture(ctx, floor.normal_tex,	 "res://octostone_normal.png");

		success &= read_texture(ctx, obj.albedo_tex,	"res://ornatebrass_albedo.png");
		success &= read_texture(ctx, obj.metallic_tex,	"res://ornatebrass_metallic.png");
		success &= read_texture(ctx, obj.roughness_tex,	"res://ornatebrass_roughness.png");
		success &= read_texture(ctx, obj.normal_tex,	"res://ornatebrass_normal.png");
		



		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		init_ibl_textures(ctx);
		generate_ibl_maps(ctx);
		compute_brdf_lut(ctx);
		generate_jitter_texture(ctx);


		rctx.ctx = &ctx;

		//TODO: add message if plugin init returns false
		return success;
	}
	void timer_event(double tt, double dt) {
		if(animate) {
			rot_angle += 20.0f * float(dt);
			if(rot_angle > 360.0f) {
				rot_angle = 0.0;
			}
			height_offset = 0.5f * float(sin(tt)) + 0.5f;
			//update_member(&angle);
			post_redraw();
		}
	}
	void init_frame(cgv::render::context& ctx) {
		if(!view_ptr) {
			if((view_ptr = find_view_as_node())) {}
		}
	}
	cgv::mat3 get_normal_matrix(const cgv::mat4& M) {
		cgv::math::fmat<float, 3, 3> NM;
		NM(0, 0) = M(0, 0);
		NM(0, 1) = M(0, 1);
		NM(0, 2) = M(0, 2);
		NM(1, 0) = M(1, 0);
		NM(1, 1) = M(1, 1);
		NM(1, 2) = M(1, 2);
		NM(2, 0) = M(2, 0);
		NM(2, 1) = M(2, 1);
		NM(2, 2) = M(2, 2);
		NM.transpose();
		NM = inv(NM);
		return NM;
	}
	void draw(cgv::render::context& ctx) {
		// lastly render the environment
		if (show_environment) {
			auto& cubemap_prog = shaders.get("cubemap");
			cubemap_prog.enable(ctx);
			cubemap_prog.set_uniform(ctx, "depth_value", 1.0f);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			environment_map.enable(ctx, 0);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			environment_map.disable(ctx);
			glDepthFunc(GL_LESS);
			cubemap_prog.disable(ctx);
		}
		if(!view_ptr)
			return;

		// render debug spheres
		//spheres.render(ctx, ref_sphere_renderer(ctx), sphere_style);
		//return;

		cgv::vec3 eye_pos = cgv::vec3(view_ptr->get_eye());

		cgv::vec3 light_direction = compute_sphere_normal(sun_position, 0.0f, float(2.0 * M_PI), 0.0f, float(M_PI));

		cgv::mat4 light_projection = cgv::math::ortho4(-5.0f, 5.0f, -5.0f, 5.0f, near_plane, far_plane);
		cgv::mat4 light_view = cgv::math::look_at4(5.0f * light_direction, cgv::vec3(0.0f, 0.0f, 0.0f), cgv::vec3(0.0f, 1.0f, 0.0f));

		cgv::mat4 bias_matrix;
		bias_matrix.set_col(0, cgv::vec4(0.5f, 0.0f, 0.0f, 0.0f));
		bias_matrix.set_col(1, cgv::vec4(0.0f, 0.5f, 0.0f, 0.0f));
		bias_matrix.set_col(2, cgv::vec4(0.0f, 0.0f, 0.5f, 0.0f));
		bias_matrix.set_col(3, cgv::vec4(0.5f, 0.5f, 0.5f, 1.0f));
			
		cgv::mat4 light_matrix = light_projection * light_view;

		rctx.store_view();
		rctx.light_matrix = light_matrix;



		auto& depth_prog = shaders.get("surface_depth");
		
		ctx.push_window_transformation_array();
		ctx.set_viewport(cgv::ivec4(0, 0, shadow_map_resolution, shadow_map_resolution));

		depth_map_fb.enable(ctx);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		floor.draw_depth(rctx, depth_prog);
		obj.draw_depth(rctx, depth_prog);

		depth_map_fb.disable(ctx);



		light_matrix = bias_matrix * light_matrix;
		rctx.light_matrix = light_matrix;
		

		// restore previous viewport
		ctx.pop_window_transformation_array();

		if(show_shadow_map) {
			depth_map.enable(ctx, 0);
			//color_map.enable(ctx, 0);

			auto& screen_prog = shaders.get("screen");
			screen_prog.enable(ctx);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			screen_prog.disable(ctx);

			depth_map.disable(ctx);
			//color_map.disable(ctx);
		} else {
			auto& pbr_prog = shaders.get("pbr_surface");
			pbr_prog.enable(ctx);
			pbr_prog.set_uniform(ctx, "eye_pos", eye_pos);
			pbr_prog.set_uniform(ctx, "light_dir", light_direction);

			pbr_prog.set_uniform(ctx, "F0", F0);
			pbr_prog.set_uniform(ctx, "roughness", roughness);

			pbr_prog.set_uniform(ctx, "shadow_blur", shadow_blur);
			pbr_prog.disable(ctx);

			auto& pbr_tex_prog = shaders.get("pbr_surface_textured");
			pbr_tex_prog.enable(ctx);
			pbr_tex_prog.set_uniform(ctx, "eye_pos", eye_pos);
			pbr_tex_prog.set_uniform(ctx, "light_dir", light_direction);

			pbr_tex_prog.set_uniform(ctx, "F0", F0);
			pbr_tex_prog.set_uniform(ctx, "roughness", roughness);
			pbr_tex_prog.set_uniform(ctx, "normal_map_scale", normal_map_scale);

			pbr_tex_prog.set_uniform(ctx, "shadow_blur", shadow_blur);
			pbr_tex_prog.disable(ctx);

			ctx.push_modelview_matrix();

			irradiance_map.enable(ctx, 0);
			prefiltered_specular_map.enable(ctx, 1);
			brdf_lut.enable(ctx, 2);
			depth_map.enable(ctx, 3);
			jitter_tex.enable(ctx, 4);

			floor.draw(rctx, pbr_tex_prog);
			// floor.draw(rctx, ctx.ref_surface_shader_program(false));

			obj.position.y() = height_offset;
			obj.rotation.y() = rot_angle;
			obj.compute_transformation();

			obj.draw(rctx, pbr_tex_prog);
			// obj.draw(rctx, ctx.ref_surface_shader_program(false));

			irradiance_map.disable(ctx);
			prefiltered_specular_map.disable(ctx);
			brdf_lut.disable(ctx);
			depth_map.disable(ctx);
			jitter_tex.disable(ctx);

			ctx.pop_modelview_matrix();
		}
	}
	bool init_ibl_textures(context& ctx) {
		bool success = true;

		if(environment_map.is_created())
			environment_map.destruct(ctx);
		environment_map.set_data_format("flt32[R,G,B]");
		success &= environment_map.create(ctx, TT_CUBEMAP, environment_resolution, environment_resolution);
		environment_map.set_wrap_s(TW_CLAMP_TO_EDGE);
		environment_map.set_wrap_t(TW_CLAMP_TO_EDGE);
		environment_map.set_wrap_r(TW_CLAMP_TO_EDGE);
		environment_map.set_min_filter(TF_LINEAR);
		environment_map.set_mag_filter(TF_LINEAR);

		if(irradiance_map.is_created())
			irradiance_map.destruct(ctx);
		irradiance_map.set_data_format("flt32[R,G,B]");
		success &= irradiance_map.create(ctx, TT_CUBEMAP, irradiance_resolution, irradiance_resolution);
		irradiance_map.set_wrap_s(TW_CLAMP_TO_EDGE);
		irradiance_map.set_wrap_t(TW_CLAMP_TO_EDGE);
		irradiance_map.set_wrap_r(TW_CLAMP_TO_EDGE);
		irradiance_map.set_min_filter(TF_LINEAR);
		irradiance_map.set_mag_filter(TF_LINEAR);

		if(prefiltered_specular_map.is_created())
			prefiltered_specular_map.destruct(ctx);
		prefiltered_specular_map.set_data_format("flt32[R,G,B]");
		success &= prefiltered_specular_map.create(ctx, TT_CUBEMAP, prefiltered_specular_resolution, prefiltered_specular_resolution);
		prefiltered_specular_map.set_wrap_s(TW_CLAMP_TO_EDGE);
		prefiltered_specular_map.set_wrap_t(TW_CLAMP_TO_EDGE);
		prefiltered_specular_map.set_wrap_r(TW_CLAMP_TO_EDGE);
		prefiltered_specular_map.set_min_filter(TF_LINEAR_MIPMAP_LINEAR);
		prefiltered_specular_map.set_mag_filter(TF_LINEAR);
		prefiltered_specular_map.generate_mipmaps(ctx);

		return success;
	}
	void generate_ibl_maps(context& ctx) {
		
		glDisable(GL_DEPTH_TEST);

		// create a frame buffer object to capture the computed values for al IBL maps
		frame_buffer capture_fbo;
		capture_fbo.create(ctx, 0, 0);
		
		// safe default viewport and configure the viewport to the capture dimensions of the environment map
		ctx.push_window_transformation_array();
		ctx.set_viewport(cgv::ivec4(0, 0, environment_resolution, environment_resolution));

		auto& sky_cubemap_gen = shaders.get("sky_cubemap_gen");
		sky_cubemap_gen.set_uniform(ctx, "sun_pos", sun_position);

		unsigned fbo_id = (unsigned)((size_t)capture_fbo.handle) - 1;
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (unsigned)((size_t)environment_map.handle) - 1, 0);

		glClear(GL_COLOR_BUFFER_BIT);

		sky_cubemap_gen.enable(ctx);
		glDrawArrays(GL_POINTS, 0, 6);
		sky_cubemap_gen.disable(ctx);

		// configure the viewport to the capture dimensions of the irradiance map
		ctx.set_viewport(cgv::ivec4(0, 0, irradiance_resolution, irradiance_resolution));

		environment_map.enable(ctx, 0);

		auto& irradiance_map_gen = shaders.get("irradiance_map_gen");

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (unsigned)((size_t)irradiance_map.handle) - 1, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		irradiance_map_gen.enable(ctx);
		glDrawArrays(GL_POINTS, 0, 6);
		irradiance_map_gen.disable(ctx);

		auto& prefiltered_specular_map_gen = shaders.get("prefiltered_specular_map_gen");

		unsigned int maxMipLevels = 5;
		for(unsigned int mip = 0; mip < maxMipLevels; ++mip) {
			unsigned int mip_resolution = (unsigned)(float(prefiltered_specular_resolution) * std::pow(0.5f, float(mip)));
			// configure the viewport to the capture dimensions of the prefiltered specular map mipmap level
			ctx.set_viewport(cgv::ivec4(0, 0, mip_resolution, mip_resolution));

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			prefiltered_specular_map_gen.set_uniform(ctx, "roughness", roughness);

			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (unsigned)((size_t)prefiltered_specular_map.handle) - 1, mip);
			glClear(GL_COLOR_BUFFER_BIT);

			prefiltered_specular_map_gen.enable(ctx);
			glDrawArrays(GL_POINTS, 0, 6);
			prefiltered_specular_map_gen.disable(ctx);

		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
		environment_map.disable(ctx);
		
		glEnable(GL_DEPTH_TEST);

		capture_fbo.destruct(ctx);
		
		// restore previous viewport
		ctx.pop_window_transformation_array();
	}
	void compute_brdf_lut(context& ctx) {
		unsigned lut_resolution = 512u;

		if(brdf_lut.is_created()) {
			brdf_lut.destruct(ctx);
		}
		brdf_lut.set_data_format("flt32[R,G]");
		brdf_lut.create(ctx, TT_2D, lut_resolution, lut_resolution);
		brdf_lut.set_wrap_s(TW_CLAMP_TO_EDGE);
		brdf_lut.set_wrap_t(TW_CLAMP_TO_EDGE);
		brdf_lut.set_min_filter(TF_LINEAR);
		brdf_lut.set_mag_filter(TF_LINEAR);

		frame_buffer lut_fb;
		lut_fb.create(ctx, lut_resolution, lut_resolution);
		lut_fb.attach(ctx, brdf_lut, 0);

		// safe main viewport and set viewport to lut resolution
		ctx.push_window_transformation_array();
		ctx.set_viewport(cgv::ivec4(0, 0, lut_resolution, lut_resolution));

		glDisable(GL_DEPTH_TEST);

		lut_fb.enable(ctx, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		auto& brdf_lut_gen = shaders.get("brdf_lut_gen");
		brdf_lut_gen.enable(ctx);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		brdf_lut_gen.disable(ctx);

		lut_fb.disable(ctx);

		glEnable(GL_DEPTH_TEST);

		lut_fb.destruct(ctx);

		// restore previous viewport
		ctx.pop_window_transformation_array();
	}
	cgv::vec3 compute_sphere_normal(cgv::vec2 coord, float phiStart, float phiLength, float thetaStart, float thetaLength) {
		cgv::vec3 normal;
		normal.x() = -sin(thetaStart + coord.y() * thetaLength) * sin(phiStart + coord.x() * phiLength);
		normal.y() = -cos(thetaStart + coord.y() * thetaLength);
		normal.z() = -sin(thetaStart + coord.y() * thetaLength) * cos(phiStart + coord.x() * phiLength);
		return normalize(normal);
	}
	bool generate_jitter_texture(context& ctx) {
		bool success = true;

		std::mt19937 rng(42);
		std::uniform_real_distribution<float> distr(-1.0f, 1.0f);

		// xy resolution of jitter texture
		int size = 16;
		// totl number of samples is samples_u * samples_v
		int samples_u = 8;
		int samples_v = 8;

		std::vector<cvec4> jitter_data(size * size * samples_u * samples_v / 2);

		/*const rgb white = rgb(1.0f);
		const rgb red = rgb(1.0f, 0.0f, 0.0f);
		const rgb green = rgb(0.0f, 1.0f, 0.0f);
		const rgb blue = rgb(0.0f, 0.0f, 1.0f);

		spheres.clear();
		for(unsigned i = 0; i < 3; ++i) {
			spheres.add(cgv::vec3(0.0f, 0.0f, 0.1f*i));
			spheres.add(cgv::vec3(1.0f, 0.0f, 0.1f*i));
			spheres.add(cgv::vec3(0.0f, 1.0f, 0.1f*i));
			spheres.add(cgv::vec3(1.0f, 1.0f, 0.1f*i));
		}
		spheres.fill(white);*/

		for(int i = 0; i < size; i++) {
			for(int j = 0; j < size; j++) {
				for(int k = 0; k < samples_u*samples_v / 2; k++) {

					int x, y;
					cgv::vec4 v;

					x = k % (samples_u / 2);
					y = (samples_v - 1) - k / (samples_u / 2);

					// generate points on a regular rectangular grid with dimensions samples_u x samples_v
					v[0] = (float)(x * 2 + 0.5f) / samples_u;
					v[1] = (float)(y + 0.5f) / samples_v;
					v[2] = (float)(x * 2 + 1 + 0.5f) / samples_u;
					v[3] = v[1];

					/*if(i == 0 && j == 0) {
						spheres.add(cgv::vec3(v[0], v[1], 0.0f), red);
						spheres.add(cgv::vec3(v[2], v[3], 0.0f), red);
					}*/

					// jitter position
					v[0] += distr(rng) * (0.5f / samples_u);
					v[1] += distr(rng) * (0.5f / samples_v);
					v[2] += distr(rng) * (0.5f / samples_u);
					v[3] += distr(rng) * (0.5f / samples_v);


					/*if(i == 0 && j == 0) {
						spheres.add(cgv::vec3(v[0], v[1], 0.1f), green);
						spheres.add(cgv::vec3(v[2], v[3], 0.1f), green);
					}*/

					// warp to disk (does not perform as well as square samples)
					cgv::vec4 d;
					//d[0] = sqrt(v[1]) * cos(2.0f * M_PI * v[0]);
					//d[1] = sqrt(v[1]) * sin(2.0f * M_PI * v[0]);
					//d[2] = sqrt(v[3]) * cos(2.0f * M_PI * v[2]);
					//d[3] = sqrt(v[3]) * sin(2.0f * M_PI * v[2]);
					//d = 0.5f * d + 0.5f;

					// take samples as is
					d = v;

					/*if(i == 1 && j == 0) {
						rgb col = k < 2 ? red : green;
						spheres.add(cgv::vec3(d[0], d[1], 0.2f), col);
						spheres.add(cgv::vec3(d[2], d[3], 0.2f), col);
					}*/

					// save samples as signed bytes to reduce memory requirements
					jitter_data[(k * size * size + j * size + i)] = static_cast<cvec4>(127.0f * d);
				}
			}
		}

		if(jitter_tex.is_created())
			jitter_tex.destruct(ctx);

		cgv::data::data_view jitter_dv = cgv::data::data_view(new cgv::data::data_format(size, size, samples_u * samples_v / 2, TI_INT8, cgv::data::CF_RGBA), jitter_data.data());
		success &= jitter_tex.create(ctx, jitter_dv, 0);
		jitter_tex.set_wrap_s(TW_REPEAT);
		jitter_tex.set_wrap_t(TW_REPEAT);
		jitter_tex.set_wrap_r(TW_REPEAT);
		jitter_tex.set_min_filter(TF_NEAREST);
		jitter_tex.set_mag_filter(TF_NEAREST);

		return success;
	}
	bool read_texture(context& ctx, texture& tex, const std::string& file_name) {
		cgv::data::data_format df;
		cgv::data::data_view dv;
		bool success = tex.create_from_image(df, dv, ctx, file_name);
		tex.set_wrap_s(TW_REPEAT);
		tex.set_wrap_t(TW_REPEAT);
		tex.set_min_filter(TF_ANISOTROP, 16.0f);
		tex.set_mag_filter(TF_LINEAR);
		tex.generate_mipmaps(ctx);
		return success;
	}
	void create_gui() {
		add_decorator("Environment Demo", "heading");

		add_member_control(this, "Show Environment", show_environment, "check");
		add_decorator("Sun Position", "heading", "level=3");
		add_member_control(this, "Horizontal", sun_position[0], "value_slider", "min=0;max=1;step=0.01;ticks=true");
		add_member_control(this, "Vertical", sun_position[1], "value_slider", "min=0.4;max=1;step=0.01;ticks=true");

		add_member_control(this, "Lod", lod, "value_slider", "min=0;max=9;step=0.1;ticks=true");

		add_member_control(this, "Near", near_plane, "value_slider", "min=0.01;max=5.0;step=0.1;ticks=true");
		add_member_control(this, "Far", far_plane, "value_slider", "min=1.0;max=30.0;step=0.1;ticks=true");
		
		add_member_control(this, "Show Shadow Map", show_shadow_map, "check");
		add_member_control(this, "Shadow Blur", shadow_blur, "value_slider", "min=0.0;max=10.0;step=0.1;ticks=true");

		add_member_control(this, "F0", F0);
		add_member_control(this, "Roughness", roughness, "value_slider", "min=0;max=1;step=0.01;ticks=true");
		add_member_control(this, "Normal Map Scale", normal_map_scale, "value_slider", "min=0;max=1;step=0.01;ticks=true");

		add_member_control(this, "Animate", animate, "toggle");

		/*if(begin_tree_node("Sphere Style", sphere_style, true)) {
			align("\a");
			add_gui("sphere_style", sphere_style);
			align("\b");
			end_tree_node(sphere_style);
		}*/
	}
};

#include <cgv/base/register.h>

/// register a factory to create new environment demos
cgv::base::factory_registration<environment_demo> environment_demo_fac("New/Demo/Environment Demo");
//cgv::base::object_registration<environment_demo> environment_demo_fac("New/Demo/Environment Demo");
