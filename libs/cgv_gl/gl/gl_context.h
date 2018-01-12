#pragma once

#include <stack>
#include <cgv/render/context.h>
#include <cgv_gl/gl/gl.h>

#include "lib_begin.h"

namespace cgv {
	/// namespace for api independent GPU programming
	namespace render {
		/// namespace for opengl specific GPU programming
		namespace gl {

/// set a very specific texture format from a gl-constant and a component_format description. This should be called after the texture is constructed and before it is created.
extern CGV_API void set_gl_format(texture& tex, GLuint gl_format, const std::string& component_format_description);

/// return the texture format used for a given texture. If called before texture has been created, the function returns, which format would be chosen by the automatic format selection process.
extern CGV_API GLuint get_gl_format(const texture& tex);

extern CGV_API GLuint map_to_gl(PrimitiveType pt);

extern CGV_API GLuint map_to_gl(MaterialSide ms);

/// set material in opengl state to given material
extern CGV_API void set_material(const cgv::media::illum::phong_material& mat, MaterialSide ms, float alpha);


/** implementation of the context API for the OpenGL API excluding methods for font selection, redraw and
    initiation of render passes. */
class CGV_API gl_context : public render::context
{
private:
	std::stack<void*> frame_buffer_stack;
	/// currently enabled shader program
	shader_program_base* enabled_program;
	/// currently enabled attribute array binding
	attribute_array_binding_base* enabled_aab;

	int query_integer_constant(ContextIntegerConstant cic) const;
	GLuint texture_bind(TextureType tt, GLuint tex_id);
	void texture_unbind(TextureType tt, GLuint tmp_id);
	GLuint texture_generate(texture_base& tb);
	void frame_buffer_bind(const frame_buffer_base& fbb, void*& user_data) const;
	void frame_buffer_unbind(const frame_buffer_base& fbb, void*& user_data) const;
	void frame_buffer_bind(frame_buffer_base& fbb) const;
	void frame_buffer_unbind(frame_buffer_base& fbb) const;
protected:
	void put_id(void* handle, void* ptr) const;


	cgv::data::component_format texture_find_best_format(const cgv::data::component_format& cf, render_component& rc, const std::vector<cgv::data::data_view>* palettes = 0) const;
	bool texture_create(texture_base& tb, cgv::data::data_format& df);
	bool texture_create(texture_base& tb, cgv::data::data_format& target_format, const cgv::data::const_data_view& data, int level, int cube_side = -1, const std::vector<cgv::data::data_view>* palettes = 0);
	bool texture_create_from_buffer(texture_base& tb, cgv::data::data_format& df, int x, int y, int level);
	bool texture_replace(texture_base& tb, int x, int y, int z_or_cube_side, const cgv::data::const_data_view& data, int level, const std::vector<cgv::data::data_view>* palettes = 0);
	bool texture_replace_from_buffer(texture_base& tb, int x, int y, int z_or_cube_side, int x_buffer, int y_buffer, unsigned int width, unsigned int height, int level);
	bool texture_generate_mipmaps(texture_base& tb, unsigned int dim);
	bool texture_destruct(texture_base& tb);
	bool texture_set_state(const texture_base& tb);
	bool texture_enable(texture_base& tb, int tex_unit, unsigned int nr_dims);
	bool texture_disable(texture_base& tb, int tex_unit, unsigned int nr_dims);

	bool render_buffer_create(render_component& rc, cgv::data::component_format& cf, int& _width, int& _height);
	bool render_buffer_destruct(render_component& rc);

	bool frame_buffer_create(frame_buffer_base& fbb);
	bool frame_buffer_attach(frame_buffer_base& fbb, const render_component& rb, bool is_depth, int i);
	bool frame_buffer_attach(frame_buffer_base& fbb, const texture_base& t, bool is_depth, int level, int i, int z);
	bool frame_buffer_is_complete(const frame_buffer_base& fbb) const;
	bool frame_buffer_enable(frame_buffer_base& fbb);
	bool frame_buffer_disable(frame_buffer_base& fbb);
	bool frame_buffer_destruct(frame_buffer_base& fbb);
	int frame_buffer_get_max_nr_color_attachments();
	int frame_buffer_get_max_nr_draw_buffers();

	bool shader_code_create(render_component& sc, ShaderType st, const std::string& source);
	bool shader_code_compile(render_component& sc);
	void shader_code_destruct(render_component& sc);

	bool shader_program_create(shader_program_base& spb);
	void shader_program_attach(shader_program_base& spb, const render_component& sc);
	bool shader_program_link(shader_program_base& spb);
	bool shader_program_set_state(shader_program_base& spb);
	bool shader_program_enable(shader_program_base& spb);
	bool shader_program_disable(shader_program_base& spb);
	void shader_program_detach(shader_program_base& spb, const render_component& sc);
	void shader_program_destruct(shader_program_base& spb);

	int  get_uniform_location(const shader_program_base& spb, const std::string& name) const;
	bool set_uniform_void(shader_program_base& spb, int loc, type_descriptor value_type, const void* value_ptr);
	bool set_uniform_array_void(shader_program_base& spb, int loc, type_descriptor value_type, const void* value_ptr, size_t nr_elements);
	int  get_attribute_location(const shader_program_base& spb, const std::string& name) const;
	bool set_attribute_void(shader_program_base& spb, int loc, type_descriptor value_type, const void* value_ptr);

	bool attribute_array_binding_create(attribute_array_binding_base& aab);
	bool attribute_array_binding_destruct(attribute_array_binding_base& aab);
	bool attribute_array_binding_enable(attribute_array_binding_base& aab);
	bool attribute_array_binding_disable(attribute_array_binding_base& aab);
	bool set_attribute_array_void(attribute_array_binding_base* aab, int loc, type_descriptor value_type, const vertex_buffer_base* vbb, const void* ptr, size_t nr_elements, unsigned stride_in_bytes);
	bool enable_attribute_array(attribute_array_binding_base* aab, int loc, bool do_enable);
	bool is_attribute_array_enabled(const attribute_array_binding_base* aab, int loc) const;

	bool vertex_buffer_create(vertex_buffer_base& vbb, const void* array_ptr, size_t size_in_bytes);
	bool vertex_buffer_replace(vertex_buffer_base& vbb, size_t offset, size_t size_in_bytes, const void* array_ptr);
	bool vertex_buffer_copy(const vertex_buffer_base& src, size_t src_offset, vertex_buffer_base& target, size_t target_offset, size_t size_in_bytes);
	bool vertex_buffer_copy_back(vertex_buffer_base& vbb, size_t offset, size_t size_in_bytes, void* array_ptr);
	bool vertex_buffer_destruct(vertex_buffer_base& vbb);

	bool check_gl_error(const std::string& where, const cgv::render::render_component* rc = 0);
	bool check_texture_support(TextureType tt, const std::string& where, const cgv::render::render_component* rc = 0);
	bool check_shader_support(ShaderType st, const std::string& where, const cgv::render::render_component* rc = 0);
	bool check_fbo_support(const std::string& where, const cgv::render::render_component* rc = 0);
	/// font used to draw textual info
	cgv::media::font::font_face_ptr info_font_face;
	/// font size to draw textual info
	float info_font_size;
	/// 
	void draw_textual_info();
	/// 
	bool show_help;
	bool show_stats;
public:
	/// construct context and attach signals
	gl_context();
	/// ensure that glew is initialized, define lighting mode, viewing pyramid and the rendering mode and return whether gl configuration was successful
	bool configure_gl();
	void resize_gl();
	///
	void perform_screen_shot();
	/// return the used rendering API
	RenderAPI get_render_api() const;
	///
	void init_render_pass();
	///
	void finish_render_pass();

	/**@name light and materials management*/
	//@{
	/// enable a material without textures
	void enable_material(const cgv::media::illum::phong_material& mat, MaterialSide ms = MS_FRONT_AND_BACK, float alpha = 1);
	/// disable phong material
	void disable_material(const cgv::media::illum::phong_material& mat);
	/// enable a material with textures
	void enable_material(const textured_material& mat, MaterialSide ms = MS_FRONT_AND_BACK, float alpha = 1);
	/// disable phong material
	void disable_material(const textured_material& mat);
	/// return maximum number of supported light sources
	unsigned get_max_nr_lights() const;
	/// enable a light source and return a handled to be used for disabling, if no more light source unit available 0 is returned
	void* enable_light(const cgv::media::illum::light_source& light);
	/// disable a previously enabled light
	void disable_light(void* handle);
	///
	void tesselate_arrow(double length = 1, double aspect = 0.1, double rel_tip_radius = 2.0, double tip_aspect = 0.3, int res = 25);
	/// 
	void tesselate_arrow(const cgv::math::fvec<double, 3>& start, const cgv::math::fvec<double, 3>& end, double aspect = 0.1f, double rel_tip_radius = 2.0f, double tip_aspect = 0.3f, int res = 25);
	///
	void draw_light_source(const cgv::media::illum::light_source& l, float intensity_scale, float light_scale); 
	//@}

	/**@name text output*/
	//@{
	/// use this to push transformation matrices on the stack such that x and y coordinates correspond to window coordinates
	void push_pixel_coords();
	/// transform point p into cursor coordinates and put x and y coordinates into the passed variables
	void put_cursor_coords(const vec_type& p, int& x, int& y) const;
	/// pop previously changed transformation matrices 
	void pop_pixel_coords();
	/// implement according to specification in context class
	virtual bool read_frame_buffer(
		data::data_view& dv, 
		unsigned int x = 0, unsigned int y = 0, 
		FrameBufferType buffer_type = FB_BACK,
		TypeId type = type::info::TI_UINT8,
		data::ComponentFormat cf = data::CF_RGB,
		int w = -1, int h = -1);
	//@}

	/**@name tesselation*/
	//@{
	/// tesselate with independent faces
	virtual void draw_faces(
		const double* vertices, const double* normals, const double* tex_coords, 
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, 
		int nr_faces, int face_degree, bool flip_normals) const;
	/// tesselate with one face strip
	virtual void draw_strip_or_fan(
		const double* vertices, const double* normals, const double* tex_coords, 
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, 
		int nr_faces, int face_degree, bool is_fan, bool flip_normals) const;
	//@}

	/**@name transformations*/
	//@{
	/// return homogeneous 4x4 viewing matrix, which transforms from world to eye space
	mat_type get_V() const;
	/// set the current viewing matrix, which transforms from world to eye space
	void set_V(const mat_type& V) const;
	/// return homogeneous 4x4 projection matrix, which transforms from eye to clip space
	mat_type get_P() const;
	/// set the current projection matrix, which transforms from eye to clip space
	void set_P(const mat_type& P) const;
	/// return homogeneous 4x4 projection matrix, which transforms from clip to device space
	mat_type get_D() const;
	/// read the device z-coordinate from the z-buffer for the given device x- and y-coordinates
	double get_z_D(int x_D, int y_D) const;
	//@}
};

		}
	}
}

#include <cgv/config/lib_end.h>