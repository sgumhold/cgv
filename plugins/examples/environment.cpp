#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/math/ftransform.h>
#include <cgv/media/mesh/simple_mesh.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_glutil/box_render_data.h>
#include <cgv_glutil/sphere_render_data.h>
#include <cgv_glutil/frame_buffer_container.h>
#include <cgv_glutil/shader_library.h>

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

	cgv::glutil::shader_library shaders;

	unsigned shadow_map_resolution = 4*256u;
	texture depth_map;
	texture color_map;
	frame_buffer depth_map_fb;

	vec2 sun_position;

	cgv::glutil::box_render_data<> boxes;
	cgv::glutil::sphere_render_data<> spheres;
	sphere_render_style sphere_style;

	texture environment_map;
	texture irradiance_map;
	texture prefiltered_specular_map;
	texture brdf_lut;

	float lod = 0.0f;

	float roughness = 0.0f;
	rgb F0 = rgb(0.0f);

	mesh_render_info box_mesh_info, obj_mesh_info;

	bool show_shadow_map = false;

	float near_plane = 1.0f, far_plane = 10.0f;

	float shadow_blur = 2.0f;

	texture jitter_tex;

public:
	environment_demo() : cgv::base::node("environment demo") {
		view_ptr = nullptr;
		shaders.add("sky", "sky.glpr");
		shaders.add("sky_cubemap_gen", "sky_cubemap_gen.glpr");
		shaders.add("irradiance_map_gen", "irradiance_map_gen.glpr");
		shaders.add("prefiltered_specular_map_gen", "prefiltered_specular_map_gen.glpr");
		shaders.add("brdf_lut_gen", "brdf_lut_gen.glpr");
		shaders.add("screen", "screen.glpr");

		shaders.add("pbr_surface", "pbr_surface.glpr");
		shaders.add("surface_depth", "surface_depth.glpr");

		sun_position = vec2(0.0f, 0.6f);

		boxes = cgv::glutil::box_render_data<>(true);
		spheres = cgv::glutil::sphere_render_data<>(true);
		
		sphere_style.surface_color = rgb(0.5f);
		sphere_style.radius = 0.02f;
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
			generate_sky_cubemap(ctx);
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
		ref_box_renderer(ctx, -1);
		ref_sphere_renderer(ctx, -1);

		shaders.clear(ctx);
	}
	bool init(cgv::render::context& ctx) {
		ref_box_renderer(ctx, 1);
		ref_sphere_renderer(ctx, 1);

		bool success = true;
		success &= shaders.load_shaders(ctx);
		success &= boxes.init(ctx);

		if(spheres.init(ctx)) {
			spheres.add(vec3(0.0f, -50.0f, 0.0f), 50.0f);
			spheres.add(vec3(0.0f, 0.5f, 0.0f), 0.5f);
			spheres.add(vec3(0.1f, 1.2f, 0.0f), 0.2f);
			spheres.set_out_of_date();
		} else {
			success = false;
		}

		// load a mesh file
		cgv::media::mesh::simple_mesh<> box_mesh, obj_mesh;

		if(getenv("CGV_DIR")) {
			box_mesh.read(std::string(getenv("CGV_DIR")) + "/plugins/examples/res/flat_box.obj");
			obj_mesh.read(std::string(getenv("CGV_DIR")) + "/plugins/examples/res/blob.obj");
		}

		box_mesh_info.construct(ctx, box_mesh);
		obj_mesh_info.construct(ctx, obj_mesh);
		// bind mesh attributes to pbr surface shader program
		//box_mesh_info.bind(ctx, shaders.get("pbr_surface"), true);
		//obj_mesh_info.bind(ctx, shaders.get("pbr_surface"), true);

		/*if(car_mesh.read("res://example.obj")) {
			car_mesh_info.construct(ctx, car_mesh);
			car_mesh_info.bind(ctx, ctx.ref_surface_shader_program(true), true);
			auto& mats = car_mesh_info.get_materials();
			if(mats.size() > 0) {
				mats[0]->set_diffuse_reflectance(rgb(0.7f, 0.6f, 0.1f));
				mats[0]->set_roughness(0.5f);
			}
		}*/

		depth_map.set_data_format("uint16[D]");
		depth_map.create(ctx, TT_2D, shadow_map_resolution, shadow_map_resolution);
		depth_map.set_wrap_s(TW_CLAMP_TO_BORDER);
		depth_map.set_wrap_t(TW_CLAMP_TO_BORDER);
		//depth_map.set_wrap_s(TW_CLAMP_TO_EDGE);
		//depth_map.set_wrap_t(TW_CLAMP_TO_EDGE);
		//depth_map.set_min_filter(TF_NEAREST);
		//depth_map.set_mag_filter(TF_NEAREST);
		depth_map.set_min_filter(TF_LINEAR);
		depth_map.set_mag_filter(TF_LINEAR);
		depth_map.set_border_color(1.0f, 1.0f, 1.0f, 1.0f);
		depth_map.set_compare_mode(!show_shadow_map);

		color_map.set_data_format("flt32[R,G,B]");
		color_map.create(ctx, TT_2D, shadow_map_resolution, shadow_map_resolution);
		color_map.set_wrap_s(TW_REPEAT);
		color_map.set_wrap_t(TW_REPEAT);
		color_map.set_min_filter(TF_LINEAR);
		color_map.set_mag_filter(TF_LINEAR);
		
		depth_map_fb.create(ctx, shadow_map_resolution, shadow_map_resolution);
		depth_map_fb.attach(ctx, depth_map);
		// color buffer is not needed for shadow map but fb with only depth buffer is not valid (solution below still produces framework errors but works fine)
		//depth_map_fb.attach(ctx, color_map, 0);

		// TODO: make framebuffer option to use no draw buffer (only depth) and make this not throw an error
		unsigned dmfbo_handle = (unsigned)depth_map_fb.handle - 1;
		glBindFramebuffer(GL_FRAMEBUFFER, dmfbo_handle);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		generate_sky_cubemap(ctx);





		std::mt19937 rng(42);
		std::uniform_real_distribution<float> distr(-1.0f, 1.0f);

		unsigned jitter_resolution = 16;
		int size = jitter_resolution;
		int samples_u = 8;
		int samples_v = 8;

		

		std::vector<cvec4> jitter_data(size * size * samples_u * samples_v / 2);
		//std::vector<vec4> jitter_data(size * size * samples_u * samples_v / 2);

		int idx = 0;

		float min_u = 1000.0f;
		float max_u = -1000.0f;

		float min_v = 1000.0f;
		float max_v = -1000.0f;

		
		const rgb white = rgb(1.0f);
		const rgb red = rgb(1.0f, 0.0f, 0.0f);
		const rgb green = rgb(0.0f, 1.0f, 0.0f);
		const rgb blue = rgb(0.0f, 0.0f, 1.0f);

		spheres.clear();
		for(unsigned i = 0; i < 3; ++i) {
			spheres.add(vec3(0.0f, 0.0f, 0.1f*i));
			spheres.add(vec3(1.0f, 0.0f, 0.1f*i));
			spheres.add(vec3(0.0f, 1.0f, 0.1f*i));
			spheres.add(vec3(1.0f, 1.0f, 0.1f*i));
		}
		spheres.fill(white);


		for(int i = 0; i < size; i++) {
			for(int j = 0; j < size; j++) {
				float rot_offset = ((float)rand() / RAND_MAX - 1) * 2 * 3.1415926f;
				for(int k = 0; k < samples_u*samples_v / 2; k++) {

					int x, y;
					vec4 v;

					x = k % (samples_u / 2);
					y = (samples_v - 1) - k / (samples_u / 2);

					// generate points on a regular samples_u x samples_v rectangular grid
					v[0] = (float)(x * 2 + 0.5f) / samples_u;
					v[1] = (float)(y + 0.5f) / samples_v;
					v[2] = (float)(x * 2 + 1 + 0.5f) / samples_u;
					v[3] = v[1];



					if(i == 0 && j == 0) {
						//spheres.add(vec3(v[0], v[1], 0.0f), red);
						//spheres.add(vec3(v[2], v[3], 0.0f), red);
					}

					

					// jitter position
					v[0] += distr(rng) * (0.5f / samples_u);
					v[1] += distr(rng) * (0.5f / samples_v);
					v[2] += distr(rng) * (0.5f / samples_u);
					v[3] += distr(rng) * (0.5f / samples_v);


					if(i == 0 && j == 0) {
						//spheres.add(vec3(v[0], v[1], 0.1f), green);
						//spheres.add(vec3(v[2], v[3], 0.1f), green);
					}



					min_u = std::min(min_u, v[0]);
					min_u = std::min(min_u, v[2]);
					max_u = std::max(max_u, v[0]);
					max_u = std::max(max_u, v[2]);

					min_v = std::min(min_v, v[1]);
					min_v = std::min(min_v, v[3]);
					max_v = std::max(max_v, v[1]);
					max_v = std::max(max_v, v[3]);

					// warp to disk
					vec4 d;
					//d[0] = sqrt(v[1]) * cos(2.0f * M_PI * v[0]);
					//d[1] = sqrt(v[1]) * sin(2.0f * M_PI * v[0]);
					//d[2] = sqrt(v[3]) * cos(2.0f * M_PI * v[2]);
					//d[3] = sqrt(v[3]) * sin(2.0f * M_PI * v[2]);
					//d = 0.5f * d + 0.5f;

					d = v;

					if(i == 1 && j == 0) {
						rgb col = k < 2 ? red : green;
						spheres.add(vec3(d[0], d[1], 0.2f), col);
						spheres.add(vec3(d[2], d[3], 0.2f), col);
					}



					//d[0] = v[0];
					//d[1] = v[1];
					//d[2] = v[2];
					//d[3] = v[3];

					/*if(idx == 0) {
						d[0] = -1.0f;
						d[1] = -1.0f;
						d[2] = +1.0f;
						d[3] = -1.0f;
					} else {
						d[0] = -1.0f;
						d[1] = +1.0f;
						d[2] = +1.0f;
						d[3] = +1.0f;
					}*/

					idx += 1;
					idx &= 1;

					//d = 2.0f * d - 1.0f;

					//d[0] = distr(rng);
					//d[1] = distr(rng);
					//d[2] = distr(rng);
					//d[3] = distr(rng);

					jitter_data[(k * size * size + j * size + i)] = static_cast<cvec4>(127.0f * d);
					//jitter_data[(k * size * size + j * size + i)] = d;
				}

				int iii = 0;
			}
		}

		cgv::data::data_view jitter_dv = cgv::data::data_view(new cgv::data::data_format(size, size, samples_u * samples_v / 2, TI_INT8, cgv::data::CF_RGBA), jitter_data.data());
		jitter_tex.create(ctx, jitter_dv, 0);
		jitter_tex.set_wrap_s(TW_REPEAT);
		jitter_tex.set_wrap_t(TW_REPEAT);
		jitter_tex.set_wrap_r(TW_REPEAT);
		jitter_tex.set_min_filter(TF_NEAREST);
		jitter_tex.set_mag_filter(TF_NEAREST);






		return success;
	}
	void init_frame(cgv::render::context& ctx) {
		if(!view_ptr) {
			if(view_ptr = find_view_as_node()) {}
		}
	}
	void draw(cgv::render::context& ctx) {
		if(!view_ptr)
			return;

		vec3 eye_pos = vec3(view_ptr->get_eye());

		vec2 resolution(ctx.get_width(), ctx.get_height());
		auto& sky_prog = shaders.get("sky");
		sky_prog.enable(ctx);
		sky_prog.set_uniform(ctx, "resolution", resolution);
		sky_prog.set_uniform(ctx, "eye_pos", eye_pos);
		sky_prog.set_uniform(ctx, "sun_pos", sun_position);
		sky_prog.set_uniform(ctx, "lod", lod);

		glDisable(GL_DEPTH_TEST);

		environment_map.enable(ctx, 0);
		//irradiance_map.enable(ctx, 0);
		//prefiltered_specular_map.enable(ctx, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		environment_map.disable(ctx);
		//irradiance_map.disable(ctx);
		//prefiltered_specular_map.disable(ctx);

		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);

		sky_prog.disable(ctx);

		

		//spheres.render(ctx, ref_sphere_renderer(ctx), sphere_style);
		//return;






		vec3 light_direction = compute_sphere_normal(sun_position, 0.0f, 2.0 * M_PI, 0.0f, M_PI);

		mat4 light_projection = cgv::math::ortho4(-5.0f, 5.0f, -5.0f, 5.0f, near_plane, far_plane);
		mat4 light_view = cgv::math::look_at4(5.0f * light_direction, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		mat4 bias_matrix;
		bias_matrix.set_col(0, vec4(0.5f, 0.0f, 0.0f, 0.0f));
		bias_matrix.set_col(1, vec4(0.0f, 0.5f, 0.0f, 0.0f));
		bias_matrix.set_col(2, vec4(0.0f, 0.0f, 0.5f, 0.0f));
		bias_matrix.set_col(3, vec4(0.5f, 0.5f, 0.5f, 1.0f));
			
		auto& depth_prog = shaders.get("surface_depth");
		depth_prog.enable(ctx);
		depth_prog.set_uniform(ctx, "light_space_matrix", light_projection * light_view);
		depth_prog.disable(ctx);

		auto& pbr_prog = shaders.get("pbr_surface");
		pbr_prog.enable(ctx);
		pbr_prog.set_uniform(ctx, "eye_pos", eye_pos);
		pbr_prog.set_uniform(ctx, "light_dir", light_direction);
		pbr_prog.set_uniform(ctx, "light_space_matrix", bias_matrix * light_projection * light_view);

		pbr_prog.set_uniform(ctx, "F0", F0);
		pbr_prog.set_uniform(ctx, "roughness", roughness);
		pbr_prog.set_uniform(ctx, "shadow_blur", shadow_blur);
		pbr_prog.disable(ctx);



		


		//glCullFace(GL_FRONT);

		

		ctx.push_window_transformation_array();
		ctx.set_viewport(ivec4(0, 0, shadow_map_resolution, shadow_map_resolution));

		depth_map_fb.enable(ctx);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ctx.push_projection_matrix();
		ctx.push_modelview_matrix();
		ctx.set_projection_matrix(light_projection);
		ctx.set_modelview_matrix(light_view);

		box_mesh_info.bind(ctx, shaders.get("surface_depth"), true);
		obj_mesh_info.bind(ctx, shaders.get("surface_depth"), true);

		//ctx.push_modelview_matrix();
		//ctx.mul_modelview_matrix(cgv::math::scale4(vec3(10.0f, 0.2f, 10.0f)));
		//ctx.mul_modelview_matrix(cgv::math::translate4(vec3(0.0f, -0.1f, 0.0f)));
		box_mesh_info.draw_all(ctx, false, false, false);
		//ctx.pop_modelview_matrix();

		//ctx.push_modelview_matrix();
		//ctx.mul_modelview_matrix(cgv::math::translate4(vec3(0.0f, 1.0f, 0.0f)));
		obj_mesh_info.draw_all(ctx, false, false, false);
		//ctx.pop_modelview_matrix();

		ctx.pop_projection_matrix();
		ctx.pop_modelview_matrix();

		depth_map_fb.disable(ctx);
		
		
		
		//glCullFace(GL_BACK);

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

			//boxes.render(ctx, ref_box_renderer(ctx), box_style);

			//sphere_style.culling_mode = CM_BACKFACE;


			irradiance_map.enable(ctx, 0);
			prefiltered_specular_map.enable(ctx, 1);
			brdf_lut.enable(ctx, 2);
			depth_map.enable(ctx, 3);
			jitter_tex.enable(ctx, 4);

			box_mesh_info.bind(ctx, shaders.get("pbr_surface"), true);
			obj_mesh_info.bind(ctx, shaders.get("pbr_surface"), true);

			//ctx.push_modelview_matrix();
			//ctx.mul_modelview_matrix(cgv::math::scale4(vec3(10.0f, 0.2f, 10.0f)));
			//ctx.mul_modelview_matrix(cgv::math::translate4(vec3(0.0f, -0.1f, 0.0f)));
			box_mesh_info.draw_all(ctx, false, false, false);
			//ctx.pop_modelview_matrix();

			//ctx.push_modelview_matrix();
			//ctx.mul_modelview_matrix(cgv::math::translate4(vec3(0.0f, 1.0f, 0.0f)));
			//ctx.mul_modelview_matrix(cgv::math::rotate4(45.0f, vec3(0.0f, 0.0f, 1.0f)));
			obj_mesh_info.draw_all(ctx, false, false, false);
			//ctx.pop_modelview_matrix();

			//sphere_prog.set_uniform(ctx, "light_space_matrix", light_projection * light_view);
			//sphere_prog.set_uniform(ctx, "light_dir", light_direction);
			//sr.set_prog(sphere_prog);
			//spheres.render(ctx, sr, sphere_style);

			irradiance_map.disable(ctx);
			prefiltered_specular_map.disable(ctx);
			brdf_lut.disable(ctx);
			depth_map.disable(ctx);
			jitter_tex.disable(ctx);
		}


		
		
		







		
		
		
	}
	void generate_sky_cubemap(context& ctx) {
		unsigned environment_resolution = 512u;
		unsigned irradiance_resolution = 32u;
		unsigned prefiltered_specular_resolution = 128u;
		unsigned lut_resolution = 512u;

		if(environment_map.is_created()) {
			environment_map.destruct(ctx);
		}
		environment_map.set_data_format("flt32[R,G,B]");
		environment_map.create(ctx, TT_CUBEMAP, environment_resolution, environment_resolution);
		environment_map.set_wrap_s(TW_CLAMP_TO_EDGE);
		environment_map.set_wrap_t(TW_CLAMP_TO_EDGE);
		environment_map.set_wrap_r(TW_CLAMP_TO_EDGE);
		environment_map.set_min_filter(TF_LINEAR);
		environment_map.set_mag_filter(TF_LINEAR);
		
		if(irradiance_map.is_created()) {
			irradiance_map.destruct(ctx);
		}
		irradiance_map.set_data_format("flt32[R,G,B]");
		irradiance_map.create(ctx, TT_CUBEMAP, irradiance_resolution, irradiance_resolution);
		irradiance_map.set_wrap_s(TW_CLAMP_TO_EDGE);
		irradiance_map.set_wrap_t(TW_CLAMP_TO_EDGE);
		irradiance_map.set_wrap_r(TW_CLAMP_TO_EDGE);
		irradiance_map.set_min_filter(TF_LINEAR);
		irradiance_map.set_mag_filter(TF_LINEAR);

		if(prefiltered_specular_map.is_created()) {
			prefiltered_specular_map.destruct(ctx);
		}
		prefiltered_specular_map.set_data_format("flt32[R,G,B]");
		prefiltered_specular_map.create(ctx, TT_CUBEMAP, prefiltered_specular_resolution, prefiltered_specular_resolution);
		prefiltered_specular_map.set_wrap_s(TW_CLAMP_TO_EDGE);
		prefiltered_specular_map.set_wrap_t(TW_CLAMP_TO_EDGE);
		prefiltered_specular_map.set_wrap_r(TW_CLAMP_TO_EDGE);
		prefiltered_specular_map.set_min_filter(TF_LINEAR_MIPMAP_LINEAR);
		prefiltered_specular_map.set_mag_filter(TF_LINEAR);
		prefiltered_specular_map.generate_mipmaps(ctx);

		if(brdf_lut.is_created()) {
			brdf_lut.destruct(ctx);
		}
		brdf_lut.set_data_format("flt32[R,G]");
		brdf_lut.create(ctx, TT_2D, lut_resolution, lut_resolution);
		brdf_lut.set_wrap_s(TW_CLAMP_TO_EDGE);
		brdf_lut.set_wrap_t(TW_CLAMP_TO_EDGE);
		brdf_lut.set_min_filter(TF_LINEAR);
		brdf_lut.set_mag_filter(TF_LINEAR);

		frame_buffer env_fb, irr_fb, pfs_fb, lut_fb;
		env_fb.create(ctx, environment_resolution, environment_resolution);
		irr_fb.create(ctx, irradiance_resolution, irradiance_resolution);
		pfs_fb.create(ctx, prefiltered_specular_resolution, prefiltered_specular_resolution);
		lut_fb.create(ctx, lut_resolution, lut_resolution);

		lut_fb.attach(ctx, brdf_lut, 0);

		cgv::glutil::box_render_data<> box(true);
		box.init(ctx);
		box.add(vec3(0.0f), vec3(2.0f));
		box.set_out_of_date();

		mat4 capture_projection = cgv::math::perspective4(90.0f, 1.0f, 0.1f, 10.0f);
		mat4 capture_views[] =
		{
		   cgv::math::look_at4(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
		   cgv::math::look_at4(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
		   cgv::math::look_at4(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
		   cgv::math::look_at4(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
		   cgv::math::look_at4(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
		   cgv::math::look_at4(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f))
		};
		
		auto& br = ref_box_renderer(ctx);
	
		glDisable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);

		ctx.push_projection_matrix();
		ctx.push_modelview_matrix();
		ctx.set_projection_matrix(capture_projection);
		
		// safe default viewport and configure the viewport to the capture dimensions
		ctx.push_window_transformation_array();
		ctx.set_viewport(ivec4(0, 0, environment_resolution, environment_resolution));

		auto& sky_cubemap_gen = shaders.get("sky_cubemap_gen");
		sky_cubemap_gen.set_uniform(ctx, "sun_pos", sun_position);

		for(unsigned int i = 0; i < 6; ++i) {
			ctx.set_modelview_matrix(capture_views[i]);
			
			env_fb.attach(ctx, environment_map, i, 0, 0);
			env_fb.enable(ctx, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			br.set_prog(sky_cubemap_gen);
			box.render(ctx, br, box_render_style());

			env_fb.disable(ctx);
		}

		// configure the viewport to the capture dimensions
		ctx.set_viewport(ivec4(0, 0, irradiance_resolution, irradiance_resolution));

		auto& irradiance_map_gen = shaders.get("irradiance_map_gen");
		
		environment_map.enable(ctx, 0);

		for(unsigned int i = 0; i < 6; ++i) {
			ctx.set_modelview_matrix(capture_views[i]);

			irr_fb.attach(ctx, irradiance_map, i, 0, 0);
			irr_fb.enable(ctx, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			br.set_prog(irradiance_map_gen);
			box.render(ctx, br, box_render_style());

			irr_fb.disable(ctx);
		}
		
		auto& prefiltered_specular_map_gen = shaders.get("prefiltered_specular_map_gen");

		unsigned int maxMipLevels = 5;
		for(unsigned int mip = 0; mip < maxMipLevels; ++mip) {
			unsigned int mipWidth = prefiltered_specular_resolution * std::pow(0.5, mip);
			unsigned int mipHeight = prefiltered_specular_resolution * std::pow(0.5, mip);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			prefiltered_specular_map_gen.set_uniform(ctx, "roughness", roughness);
			for(unsigned int i = 0; i < 6; ++i) {
				ctx.set_modelview_matrix(capture_views[i]);

				pfs_fb.attach(ctx, prefiltered_specular_map, i, mip, 0);
				pfs_fb.enable(ctx, 0);
				
				glClear(GL_COLOR_BUFFER_BIT);

				br.set_prog(prefiltered_specular_map_gen);
				box.render(ctx, br, box_render_style());

				pfs_fb.disable(ctx);
			}
		}

		environment_map.disable(ctx);
		
		ctx.pop_projection_matrix();
		ctx.pop_modelview_matrix();

		ctx.set_viewport(ivec4(0, 0, lut_resolution, lut_resolution));

		lut_fb.enable(ctx, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		auto& brdf_lut_gen = shaders.get("brdf_lut_gen");
		brdf_lut_gen.enable(ctx);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		brdf_lut_gen.disable(ctx);

		lut_fb.disable(ctx);

		glEnable(GL_DEPTH_TEST);

		env_fb.destruct(ctx);
		irr_fb.destruct(ctx);
		pfs_fb.destruct(ctx);

		// restore previous viewport
		ctx.pop_window_transformation_array();
	}
	vec3 compute_sphere_normal(vec2 coord, float phiStart, float phiLength, float thetaStart, float thetaLength) {
		vec3 normal;
		normal.x() = -sin(thetaStart + coord.y() * thetaLength) * sin(phiStart + coord.x() * phiLength);
		normal.y() = -cos(thetaStart + coord.y() * thetaLength);
		normal.z() = -sin(thetaStart + coord.y() * thetaLength) * cos(phiStart + coord.x() * phiLength);
		return normalize(normal);
	}
	void create_gui() {
		add_decorator("Environment Demo", "heading");

		add_decorator("Sun Position", "heading", "level=3");
		add_member_control(this, "Horizontal", sun_position[0], "value_slider", "min=0;max=1;step=0.01;ticks=true");
		add_member_control(this, "Vertical", sun_position[1], "value_slider", "min=0.4;max=1;step=0.01;ticks=true");

		add_member_control(this, "Lod", lod, "value_slider", "min=0;max=9;step=0.1;ticks=true");

		add_member_control(this, "Near", near_plane, "value_slider", "min=0.01;max=5.0;step=0.1;ticks=true");
		add_member_control(this, "Far", far_plane, "value_slider", "min=1.0;max=30.0;step=0.1;ticks=true");
		
		add_member_control(this, "Show Shadow Map", show_shadow_map, "check");
		add_member_control(this, "Shadow Blur", shadow_blur, "value_slider", "min=0.0;max=10.0;step=0.1;ticks=true");

		if(begin_tree_node("Sphere Style", sphere_style, true)) {
			align("\a");
			add_gui("sphere_style", sphere_style);
			add_member_control(this, "F0", F0);
			add_member_control(this, "Roughness", roughness, "value_slider", "min=0;max=1;step=0.01;ticks=true");
			align("\b");
			end_tree_node(sphere_style);
		}

		//add_gui("box_style", box_style);
	}
};

#include <cgv/base/register.h>

/// register a factory to create new rounded cone texturing tests
cgv::base::factory_registration<environment_demo> environment_demo_fac("new/demo/environment_demo");
//cgv::base::object_registration<environment_demo> environment_demo_fac("new/demo/environment_demo");
