#pragma once

#include <stack>
#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>

#include "lib_begin.h"

namespace cgv {
	/// namespace for api independent GPU programming
	namespace render {
		/// namespace for opengl specific GPU programming
		namespace gl {

/// construct a 0 terminated list of context creation attribute definitions
extern CGV_API std::vector<int> get_context_creation_attrib_list(cgv::render::context_config& cc);

/// set a very specific texture format from a gl-constant and a component_format description. This should be called after the texture is constructed and before it is created.
extern CGV_API void set_gl_format(texture& tex, GLuint gl_format, const std::string& component_format_description);

/// return the texture format used for a given texture. If called before texture has been created, the function returns, which format would be chosen by the automatic format selection process.
extern CGV_API GLuint get_gl_format(const texture& tex);

extern CGV_API GLuint map_to_gl(PrimitiveType pt);

extern CGV_API GLuint map_to_gl(MaterialSide ms);

extern CGV_API GLuint map_to_gl(BlendFunction blend_function);

/// set material in opengl state to given material
extern CGV_API void set_material(const cgv::media::illum::phong_material& mat, MaterialSide ms, float alpha);


/** implementation of the context API for the OpenGL API excluding methods for font selection, redraw and
    initiation of render passes. */
class CGV_API gl_context : public render::context
{
private:
	int query_integer_constant(ContextIntegerConstant cic) const override;
	GLuint texture_bind(TextureType tt, GLuint tex_id) const;
	void texture_unbind(TextureType tt, GLuint tmp_id) const;
	GLuint texture_generate(texture_base& tb) const;
	/*
	void frame_buffer_bind(const frame_buffer_base& fbb, void*& user_data) const;
	void frame_buffer_unbind(const frame_buffer_base& fbb, void*& user_data) const;
	void frame_buffer_bind(frame_buffer_base& fbb) const;
	void frame_buffer_unbind(frame_buffer_base& fbb) const;
	*/
protected:
	shader_program progs[4];
	mutable cgv::type::uint64_type max_nr_indices, max_nr_vertices;
	void ensure_configured() const;
	void destruct_render_objects() override;
	void put_id(void* handle, void* ptr) const override;
	void draw_elements_void(GLenum mode, size_t count, GLenum type, size_t type_size, const void* indices) const;
	template <typename T>
	void draw_elements(GLenum mode, size_t count, const T* indices) const {
		if (!gl_traits<T>::valid_index)
			error("called draw_elements with invalid index type");
		else
			draw_elements_void(mode, count, gl_traits<T>::type, sizeof(T), indices);
	}

	cgv::data::component_format texture_find_best_format(const cgv::data::component_format& cf, render_component& rc, const std::vector<cgv::data::data_view>* palettes = 0) const override;
	bool texture_create(texture_base& tb, cgv::data::data_format& df) const override;
	bool texture_create(texture_base& tb, cgv::data::data_format& target_format, const cgv::data::const_data_view& data, int level, int cube_side = -1, int num_array_layers = 0, const std::vector<cgv::data::data_view>* palettes = 0) const override;
	bool texture_create_from_buffer(texture_base& tb, cgv::data::data_format& df, int x, int y, int level) const override;
	bool texture_replace(texture_base& tb, int x, int y, int z_or_cube_side, const cgv::data::const_data_view& data, int level, const std::vector<cgv::data::data_view>* palettes = 0) const override;
	bool texture_replace_from_buffer(texture_base& tb, int x, int y, int z_or_cube_side, int x_buffer, int y_buffer, unsigned int width, unsigned int height, int level) const override;
	bool texture_create_mipmaps(texture_base& tb, cgv::data::data_format& df) const override;
	bool texture_generate_mipmaps(texture_base& tb, unsigned int dim) const override;
	bool texture_destruct(texture_base& tb) const override;
	bool texture_set_state(const texture_base& tb) const override;
	bool texture_enable(texture_base& tb, int tex_unit, unsigned int nr_dims) const override;
	bool texture_disable(texture_base& tb, int tex_unit, unsigned int nr_dims) const override;
	bool texture_bind_as_image(texture_base& tb, int tex_unit, int level, bool bind_array, int layer, AccessType access) const override;

	bool render_buffer_create(render_buffer_base& rc, cgv::data::component_format& cf, int& _width, int& _height) const override;
	bool render_buffer_destruct(render_buffer_base& rc) const override;

	bool frame_buffer_create(frame_buffer_base& fbb) const override;
	bool frame_buffer_attach(frame_buffer_base& fbb, const render_buffer_base& rb, bool is_depth, int i) const override;
	bool frame_buffer_attach(frame_buffer_base& fbb, const texture_base& t, bool is_depth, int level, int i, int z) const override;
	bool frame_buffer_is_complete(const frame_buffer_base& fbb) const override;
	bool frame_buffer_enable(frame_buffer_base& fbb) override;
	bool frame_buffer_disable(frame_buffer_base& fbb) override;
	bool frame_buffer_destruct(frame_buffer_base& fbb) const override;
	void frame_buffer_blit(const frame_buffer_base* src_fbb_ptr, const ivec4& S, frame_buffer_base* dst_fbb_ptr, const ivec4& _D, BufferTypeBits btbs, bool interpolate) const override;
	int frame_buffer_get_max_nr_color_attachments() const override;
	int frame_buffer_get_max_nr_draw_buffers() const override;

	bool shader_code_create(render_component& sc, ShaderType st, const std::string& source) const override;
	bool shader_code_compile(render_component& sc) const override;
	void shader_code_destruct(render_component& sc) const override;

	bool shader_program_create(shader_program_base& spb) const override;
	void shader_program_attach(shader_program_base& spb, const render_component& sc) const override;
	bool shader_program_link(shader_program_base& spb) const override;
	bool shader_program_set_state(shader_program_base& spb) const override;
	bool shader_program_enable(shader_program_base& spb) override;
	bool shader_program_disable(shader_program_base& spb) override;
	void shader_program_detach(shader_program_base& spb, const render_component& sc) const override;
	bool shader_program_destruct(shader_program_base& spb) const override;

	int  get_uniform_location(const shader_program_base& spb, const std::string& name) const override;
	bool set_uniform_void(shader_program_base& spb, int loc, type_descriptor value_type, const void* value_ptr) const override;
	bool set_uniform_array_void(shader_program_base& spb, int loc, type_descriptor value_type, const void* value_ptr, size_t nr_elements) const override;
	int  get_attribute_location(const shader_program_base& spb, const std::string& name) const override;
	bool set_attribute_void(shader_program_base& spb, int loc, type_descriptor value_type, const void* value_ptr) const override;

	bool attribute_array_binding_create(attribute_array_binding_base& aab) const override;
	bool attribute_array_binding_destruct(attribute_array_binding_base& aab);
	bool attribute_array_binding_enable(attribute_array_binding_base& aab) override;
	bool attribute_array_binding_disable(attribute_array_binding_base& aab) override;
	bool set_attribute_array_void(attribute_array_binding_base* aab, int loc, type_descriptor value_type, const vertex_buffer_base* vbb, const void* ptr, size_t nr_elements, unsigned stride_in_bytes) const override;
	bool set_element_array(attribute_array_binding_base* aab, const vertex_buffer_base* vbb) const override;
	bool enable_attribute_array(attribute_array_binding_base* aab, int loc, bool do_enable) const override;
	bool is_attribute_array_enabled(const attribute_array_binding_base* aab, int loc) const override;

	bool vertex_buffer_bind(const vertex_buffer_base& vbb, VertexBufferType _type, unsigned _idx) const override;
	bool vertex_buffer_unbind(const vertex_buffer_base& vbb, VertexBufferType _type, unsigned _idx) const override;
	bool vertex_buffer_create(vertex_buffer_base& vbb, const void* array_ptr, size_t size_in_bytes) const override;
	bool vertex_buffer_resize(vertex_buffer_base& vbb, const void* array_ptr, size_t size_in_bytes) const override;
	bool vertex_buffer_replace(vertex_buffer_base& vbb, size_t offset, size_t size_in_bytes, const void* array_ptr) const override;
	bool vertex_buffer_copy(const vertex_buffer_base& src, size_t src_offset, vertex_buffer_base& target, size_t target_offset, size_t size_in_bytes) const override;
	bool vertex_buffer_copy_back(vertex_buffer_base& vbb, size_t offset, size_t size_in_bytes, void* array_ptr) const override;
	bool vertex_buffer_destruct(vertex_buffer_base& vbb) const override;

	bool check_gl_error(const std::string& where, const cgv::render::render_component* rc = 0) const;
	bool check_texture_support(TextureType tt, const std::string& where, const cgv::render::render_component* rc = 0) const;
	bool check_shader_support(ShaderType st, const std::string& where, const cgv::render::render_component* rc = 0) const;
	bool check_fbo_support(const std::string& where, const cgv::render::render_component* rc = 0) const;
	/// font used to draw textual info
	mutable cgv::media::font::font_face_ptr info_font_face;
	/// font size to draw textual info
	float info_font_size;
	/// 
	void draw_textual_info() override;
	/// 
	bool show_help;
	bool show_stats;
	/// helper function that multiplies a rotation to modelview matrix such that vector is rotated onto target
	void rotate_vector_to_target(const dvec3& vector, const dvec3& target);
public:
	/// construct context and attach signals
	gl_context();
	/// ensure that glew is initialized, define lighting mode, viewing pyramid and the rendering mode and return whether gl configuration was successful
	bool configure_gl();
	void resize_gl();

	/// set a user defined background color
	void set_bg_color(vec4 rgba) override;
	/// set a user defined background depth value
	void set_bg_depth(float d) override;
	/// set a user defined background stencil value
	void set_bg_stencil(int s) override;
	/// set a user defined background color for the accumulation buffer
	void set_bg_accum_color(vec4 rgba) override;
	/// clear the buffer contents of the flagged buffers to the set background colors
	void clear_background(bool color_flag, bool depth_flag, bool stencil_flag = false, bool accum_flag = false) override;

	/// return info font size
	float get_info_font_size() const;
	/// return info font face and ensure that it is created
	cgv::media::font::font_face_ptr get_info_font_face() const;
	/// overwrite function to return info font size in case no font is currently selected
	float get_current_font_size() const override;
	/// overwrite function to return info font face in case no font is currently selected
	media::font::font_face_ptr get_current_font_face() const override;
	///
	void perform_screen_shot() override;
	/// return the used rendering API
	RenderAPI get_render_api() const override;
	///
	void init_render_pass() override;
	///
	void finish_render_pass() override;
	/**@name light and materials management*/
	//@{
	/// set the current color
	void set_color(const rgba& clr) override;
	/// set the current material 
	void set_material(const cgv::media::illum::surface_material& mat) override;
	/// enable a material with textures
	void enable_material(textured_material& mat) override;
	/// disable a material with textures
	void disable_material(textured_material& mat) override;
	/// return a reference to a shader program used to render without illumination
	shader_program& ref_default_shader_program(bool texture_support = false) override;
	/// return a reference to the default shader program used to render surfaces 
	shader_program& ref_surface_shader_program(bool texture_support = false) override;
	/// get list of program uniforms
	void enumerate_program_uniforms(shader_program& prog, std::vector<std::string>& names, std::vector<int>* locations_ptr = 0, std::vector<int>* sizes_ptr = 0, std::vector<int>* types_ptr = 0, bool show = false) const override;
	/// get list of program attributes
	void enumerate_program_attributes(shader_program& prog, std::vector<std::string>& names, std::vector<int>* locations_ptr = 0, std::vector<int>* sizes_ptr = 0, std::vector<int>* types_ptr = 0, bool show = false) const override;
	/// helper function to send light update events
	void on_lights_changed() override;
	///
	void tesselate_arrow(double length = 1, double aspect = 0.1, double rel_tip_radius = 2.0, double tip_aspect = 0.3, int res = 25, bool edges = false) override;
	/// 
	void tesselate_arrow(const dvec3& start, const dvec3& end, double aspect = 0.1f, double rel_tip_radius = 2.0f, double tip_aspect = 0.3f, int res = 25, bool edges = false) override;
	///
	void draw_light_source(const cgv::media::illum::light_source& l, float intensity_scale, float light_scale) override; 
	//@}
	/// announce an external viewport change performed with rendering API to the cgv framework providing space to temporarily store viewport of cgv framework
	void announce_external_viewport_change(ivec4& cgv_viewport_storage) override;
	/// restore cgv viewport to the state before the external change
	void recover_from_external_viewport_change(const ivec4& cgv_viewport_storage) override;
	/// announce an external frame buffer change performed with rendering API to the cgv framework providing space to temporarily store frame buffer of cgv framework
	void announce_external_frame_buffer_change(void*& cgv_fbo_storage) override;
	/// restore cgv frame buffer to the state before the external change
	void recover_from_external_frame_buffer_change(void* cgv_fbo_storage) override;

	/**@name text output*/
	//@{
	/// use this to push transformation matrices on the stack such that x and y coordinates correspond to window coordinates
	void push_pixel_coords() override;
	/// pop previously changed transformation matrices 
	void pop_pixel_coords() override;
	/// implement according to specification in context class
	virtual bool read_frame_buffer(
		data::data_view& dv, 
		unsigned int x = 0, unsigned int y = 0, 
		FrameBufferType buffer_type = FB_BACK,
		TypeId type = type::info::TI_UINT8,
		data::ComponentFormat cf = data::CF_RGB,
		int w = -1, int h = -1) override;
	//@}

	/**@name tesselation*/
	//@{
	/// helper function to prepare attribute arrays
	bool prepare_attributes(std::vector<vec3>& P, std::vector<vec3>& N, std::vector<vec2>& T, unsigned nr_vertices,
		const float* vertices, const float* normals, const float* tex_coords,
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, bool flip_normals) const;
	/// helper function to disable attribute arrays
	bool release_attributes(const float* normals, const float* tex_coords, const int* normal_indices, const int* tex_coord_indices) const;
	/// pass geometry of given faces to current shader program and generate draw calls to render lines for the edges
	void draw_edges_of_faces(
		const float* vertices, const float* normals, const float* tex_coords,
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices,
		int nr_faces, int face_degree, bool flip_normals = false) const override;
	/// pass geometry of given strip or fan to current shader program and generate draw calls to render lines for the edges
	void draw_edges_of_strip_or_fan(
		const float* vertices, const float* normals, const float* tex_coords,
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices,
		int nr_faces, int face_degree, bool is_fan, bool flip_normals = false) const override;
	/// tesselate with independent faces
	void draw_faces(
		const float* vertices, const float* normals, const float* tex_coords,
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, 
		int nr_faces, int face_degree, bool flip_normals) const override;
	/// tesselate with one face strip
	void draw_strip_or_fan(
		const float* vertices, const float* normals, const float* tex_coords,
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, 
		int nr_faces, int face_degree, bool is_fan, bool flip_normals) const override;
	//@}

	/**@name render state*/
	//@{

	/// set the depth test state
	void set_depth_test_state(DepthTestState state) override;
	/// set the depth test function
	void set_depth_func(CompareFunction func) override;
	/// enable the depth test
	void enable_depth_test() override;
	/// disable the depth test
	void disable_depth_test() override;

	/// set the culling state
	void set_cull_state(CullingMode culling_mode) override;

	/// set the complete blend state
	void set_blend_state(BlendState state) override;
	/// set the blend function
	void set_blend_func(BlendFunction src_factor, BlendFunction dst_factor) override;
	/// set the blend function separately for color and alpha
	void set_blend_func_separate(BlendFunction src_color_factor, BlendFunction dst_color_factor, BlendFunction src_alpha_factor, BlendFunction dst_alpha_factor) override;
	/// enable blending
	void enable_blending() override;
	/// disable blending
	void disable_blending() override;

	/// set the buffer mask for depth and color buffers
	void set_buffer_mask(BufferMask mask) override;
	/// set the depth buffer mask
	void set_depth_mask(bool flag) override;
	/// set the color buffer mask
	void set_color_mask(bvec4 flags) override;

	//@}

	/**@name transformations*/
	//@{
	/// return homogeneous 4x4 viewing matrix, which transforms from world to eye space
	dmat4 get_modelview_matrix() const override;
	/// set the current modelview matrix, which transforms from world to eye space
	void set_modelview_matrix(const dmat4& V) override;
	/// return homogeneous 4x4 projection matrix, which transforms from eye to clip space
	dmat4 get_projection_matrix() const override;
	/// set the current projection matrix, which transforms from eye to clip space
	void set_projection_matrix(const dmat4& P) override;

	/// restore previous viewport and depth range arrays defining the window transformations
	void pop_window_transformation_array() override;
	/// query the maximum number of supported window transformations, which is at least 1 
	unsigned get_max_window_transformation_array_size() const override;
	//@}

protected:
	void update_window_transformation_array();
public:
	/// set the current viewport or one of the viewports in the window transformation array
	void set_viewport(const ivec4& viewport, int array_index = -1) override;
	/// set the current depth range or one of the depth ranges in the window transformation array
	void set_depth_range(const dvec2& depth_range = dvec2(0, 1), int array_index = -1) override;

	// return homogeneous 4x4 projection matrix, which transforms from clip to device space
	// dmat4 get_device_matrix() const;
	/// read the device z-coordinate from the z-buffer for the given device x- and y-coordinates
	double get_window_z(int x_window, int y_window) const override;
	//@}
};

		}
	}
}

#include <cgv/config/lib_end.h>