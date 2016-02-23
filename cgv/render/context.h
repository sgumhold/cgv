#pragma once

#define _USE_MATH_DEFINES
#include <cgv/base/group.h>
#include <cgv/data/data_view.h>
#include <cgv/media/font/font.h>
#include <cgv/media/illum/phong_material.hh>
#include <cgv/media/illum/light_source.hh>
#include <cgv/signal/callback_stream.h>
#include <cgv/math/vec.h>
#include <cgv/math/inv.h>
#include <stack>
#include <vector>
#include <string>

#include "lib_begin.h"

namespace cgv {
	namespace render {

class CGV_API drawable;
class CGV_API textured_material;

/// enumeration of rendering APIs which can be queried from the context
enum RenderAPI {
	RA_OPENGL,
	RA_DIRECTX
};

/// enumeration of different render passes, which can be queried from the context and used to specify a new render pass
enum RenderPass {
	RP_NONE,
	RP_MAIN,                 /// the main rendering pass triggered by the redraw event
	RP_STEREO,               /// rendering of second eye
	RP_SHADOW_MAP,           /// construction of shadow map
	RP_SHADOW_VOLUME,        /// construction of shadow map
	RP_OPAQUE_SURFACES,      /// opaque surface rendering using z-Buffer
	RP_TRANSPARENT_SURFACES, /// transparent surface rendering using depth peeling
	RP_PICK,                 /// in picking pass a small rectangle around the mouse is rendered 
	RP_USER_DEFINED
};

/// available flags that can be queried from the context and set for a new render pass
enum RenderPassFlags {
	RPF_NONE = 0,                      // no frame initialization is performed
	RPF_SET_PROJECTION = 1,            // whether to set default projection matrix
	RPF_SET_MODELVIEW =  1 << 1,             // whether to set default modelview matrix
	RPF_SET_MODELVIEW_PROJECTION = RPF_SET_PROJECTION|RPF_SET_MODELVIEW,  // whether to set default modelview and projection matrix
	RPF_SET_LIGHTS = 1 << 2,                // whether to define default lights
	RPF_SET_MATERIAL = 1 << 3,              // whether to define default material
	RPF_SET_LIGHTS_ON = 1 << 4,            // whether to turn on default lights
	RPF_ENABLE_MATERIAL = 1 << 5,          // whether to enable material
	RPF_SET_LIGHTING = RPF_SET_LIGHTS|RPF_SET_LIGHTS_ON|RPF_ENABLE_MATERIAL,             // whether to define and enable default lighting
	RPF_CLEAR_COLOR = 1 << 6,              // whether to clear the color buffer
	RPF_CLEAR_DEPTH = 1 << 7,             // whether to clear the depth buffer
	RPF_CLEAR_STENCIL = 1 << 8,             // whether to clear the depth buffer
	RPF_CLEAR_ACCUM = 1 << 9,               // whether to clear the accumulation buffer
	RPF_CLEAR_ALL = RPF_CLEAR_COLOR|RPF_CLEAR_DEPTH|RPF_CLEAR_STENCIL|RPF_CLEAR_ACCUM, // whether to clear all buffers
	RPF_DRAWABLES_INIT_FRAME = 1 << 10,    // whether to call the init_frame method of the drawables
	RPF_SET_STATE_FLAGS = 1 << 11,         // whether to set depth buffer and culling flags
	RPF_SET_CLEAR_COLOR = 1 << 12,        // whether to set the clear color
	RPF_SET_CLEAR_DEPTH = 1 << 13,        // whether to set the clear color
	RPF_SET_CLEAR_STENCIL = 1 << 14,        // whether to set the clear color
	RPF_SET_CLEAR_ACCUM = 1 << 15,        // whether to set the accumulation buffer clear color
	RPF_DRAWABLES_DRAW = 1 << 16,         // whether to call draw and finish_draw methods of drawables
	RPF_DRAWABLES_FINISH_FRAME = 1 << 17, // whether to call finish frame method of drawables
	RPF_DRAW_TEXTUAL_INFO = 1 << 18,      // whether to draw textual information
	RPF_DRAWABLES_AFTER_FINISH = 1 << 19, // whether to call after finish method of drawables
	RPF_HANDLE_SCREEN_SHOT = 1 << 20,    // whether to perform a screen shot if this was scheduled
	RPF_ALL = (1 << 21) - 1,            // all flags set, defines default render pass
	RPF_DEFAULT = RPF_ALL & ~ (RPF_SET_LIGHTS|RPF_CLEAR_ACCUM|RPF_SET_CLEAR_ACCUM|RPF_CLEAR_STENCIL|RPF_SET_CLEAR_STENCIL)  // all flags set, defines default render pass
};

/// different sides of a material
enum MaterialSide { MS_NONE, MS_FRONT, MS_BACK, MS_FRONT_AND_BACK };

/// different texture wrap modes
enum TextureWrap { 
	TW_REPEAT = 0, 
	TW_CLAMP = 1, 
	TW_CLAMP_TO_EDGE = 2, 
	TW_CLAMP_TO_BORDER = 3, 
	TW_MIRROR_CLAMP = 4, 
	TW_MIRROR_CLAMP_TO_EDGE = 5, 
	TW_MIRROR_CLAMP_TO_BORDER = 6,
	TW_MIRRORED_REPEAT = 7, 
	TW_LAST
};

/// convert texture wrap to string
extern CGV_API std::string to_string(TextureWrap wrap);

/// different texture filter
enum TextureFilter {
	TF_NEAREST = 0, 
	TF_LINEAR = 1, 
	TF_NEAREST_MIPMAP_NEAREST = 2, 
	TF_LINEAR_MIPMAP_NEAREST = 3, 
	TF_NEAREST_MIPMAP_LINEAR = 4,
	TF_LINEAR_MIPMAP_LINEAR = 5,
	TF_ANISOTROP = 6,
	TF_LAST
};

/// different texture types
enum TextureType {
	TT_UNDEF, 
	TT_1D = 1,
	TT_2D,
	TT_3D,
	TT_CUBEMAP,
	TT_BUFFER
};

/// the six different sides of a cube
enum TextureCubeSides {
	 TCS_PLUS_X,
	TCS_MINUS_X,
	 TCS_PLUS_Y,
	TCS_MINUS_Y,
	 TCS_PLUS_Z,
	TCS_MINUS_Z
};

/// different primitive types
enum PrimitiveType {
	PT_UNDEF,
	PT_POINTS,
	PT_LINES,
	PT_LINES_ADJACENCY,
	PT_LINE_STRIP,
	PT_LINE_STRIP_ADJACENCY,
	PT_LINE_LOOP,
	PT_TRIANGLES,
	PT_TRIANGLES_ADJACENCY,
	PT_TRIANGLE_STRIP,
	PT_TRIANGLE_STRIP_ADJACENCY,
	PT_TRIANGLE_FAN,
	PT_QUADS,
	PT_QUAD_STRIP,
	PT_POLYGON,
	PT_LAST
};

/// different sampling strategies for rendering to textures that steer the computation of the \c tex_coord input to the fragment shader
enum TextureSampling
{
	TS_CELL = 0,   /// for texture resulution N x M x L the \c tex_coord ranges from [1/2N, 1/2M, 1/2L] to [1-1/2N, 1-1/2M, 1-1/2P]
	TS_VERTEX = 1  /// \c tex_coord ranges from [0,0,0] to [1,1,1]
};

/// different text alignments
enum TextAlignment {
	TA_NONE = 0,
	TA_LEFT = 1,    // center of left edge of text bounds
	TA_RIGHT = 2,   // center of right edge of text bounds
	TA_TOP = 4,     // center of top edge of text bounds
	TA_BOTTOM = 8,  // center of bottom edge of text bounds
	TA_TOP_LEFT = TA_LEFT+TA_TOP,    // top left corner of text bounds
	TA_TOP_RIGHT = TA_RIGHT+TA_TOP,  // top right corner of text bounds
	TA_BOTTOM_LEFT = TA_LEFT+TA_BOTTOM,   // bottom left corner of text bounds
	TA_BOTTOM_RIGHT = TA_RIGHT+TA_BOTTOM  // bottom right corner of text bounds
};

/// convert texture filter to string
extern CGV_API std::string to_string(TextureFilter filter_type);

/// convert primitive type to string
extern CGV_API std::string to_string(PrimitiveType pt);

/// convert texture type to string
extern CGV_API std::string to_string(TextureType tt);

/// convert texture cube side to string
extern CGV_API std::string to_string(TextureCubeSides tcs);

class CGV_API context;

/// base interface for all render components
class CGV_API render_component
{
public:
    void* handle;
	void* internal_format;
	void* user_data;
	/// keep pointer to my context
	context* ctx_ptr;
	/// a string that contains the last error
	mutable std::string last_error;
	/// initialize members
	render_component();
	/// return whether component has been created
	bool is_created() const;
	/// copy the rendering api specific id the component to the memory location of the given pointer. 
	/// For opengl this the passed pointer should be of type GLint or GLuint.
	void put_id_void(void* ptr) const;
	/// cast the refence to rendering api specific representation of component id to the specified type
	template <typename T>
	void put_id(T& id) const { put_id_void(&id); }
};

/// base interface for a texture 
class CGV_API texture_base : public render_component
{
public:
	TextureFilter mag_filter;
	TextureFilter min_filter;
	TextureWrap   wrap_s;
	TextureWrap   wrap_t;
	TextureWrap   wrap_r;
	float anisotropy;
	float priority;
	float border_color[4];
	TextureType  tt;
	bool have_mipmaps;
	/// initialize members
	texture_base(TextureType _tt = TT_UNDEF);
};


/// base interface for shader programs
class CGV_API shader_program_base : public render_component
{
public:
	PrimitiveType geometry_shader_input_type;
	PrimitiveType geometry_shader_output_type;
	int geometry_shader_output_count;
	/// initializes members
	shader_program_base();
};

/// base interface for framebuffer
class CGV_API frame_buffer_base : public render_component
{
public:
	int width, height;
	std::vector<int> enabled_color_attachments;
	bool attached[16];
	int old_vp[4];
	/// initialize members
	frame_buffer_base();
};

/// different shader types
enum ShaderType { ST_DETECT, ST_VERTEX, ST_GEOMETRY, ST_FRAGMENT };

/// different frame buffer types which can be combined together with or
enum FrameBufferType {
	FB_0 = 0,
	FB_1 = 1,
	FB_2 = 2,
	FB_3 = 3,
	FB_4 = 4,
	FB_5 = 5,
	FB_6 = 6,
	FB_7 = 7,
	FB_BACK =  128, 
	FB_FRONT = 129, 
	FB_LEFT =  512, 
	FB_RIGHT = 1024, 
	FB_BACK_LEFT  =  FB_BACK+FB_LEFT, 
	FB_BACK_RIGHT =  FB_BACK+FB_RIGHT, 
	FB_FRONT_LEFT  =  FB_FRONT+FB_LEFT, 
	FB_FRONT_RIGHT =  FB_FRONT+FB_RIGHT
};

/// integer constants that can be queried from context
enum ContextIntegerConstant { 
	MAX_NR_GEOMETRY_SHADER_OUTPUT_VERTICES 
};

// forward declaration of all render components
class CGV_API texture;
class CGV_API render_buffer;
class CGV_API frame_buffer;
class CGV_API shader_code;
class CGV_API shader_program;

// declare some colors by name
extern CGV_API float black[4], white[4], gray[4], green[4], brown[4], dark_red[4];
extern CGV_API float cyan[4], yellow[4], red[4], blue[4];

/** base class for all drawables, which is independent of the used rendering API. */
class CGV_API context 
{
protected:
	/// information necessary for a rendering pass
	struct render_info
	{
		RenderPass pass;
		RenderPassFlags flags;
		void* user_data;
	};
	/// store the current render pass
	std::stack<render_info> render_pass_stack;
	/// default render flags with which the main render pass is initialized
	RenderPassFlags default_render_flags;
	/// current background color, depth, stencil and accum color
	float bg_r, bg_g, bg_b, bg_a, bg_d;
	int   bg_s;
	float bg_accum_r, bg_accum_g, bg_accum_b, bg_accum_a;
	/// whether to use phong shading
	bool phong_shading;
	/// current back ground color index
	int current_background;
	/// current cursor location for textual output
	int cursor_x, cursor_y;
	/// use a callback stream to write text to the opengl context
	cgv::signal::callback_stream out_stream;
	/// store current font size
	float current_font_size;
	/// store current font
	cgv::media::font::font_face_ptr current_font_face;
	/// size a tabs
	int tab_size;
	/// offset in x and y direction where text starts
	int x_offset, y_offset;
	/// current number of indentations
	int nr_identations;
	/// store whether we are at the beginning of the line
	bool at_line_begin;
	///
	bool do_screen_shot;
	/// callback method for processing of text from the output stream
	virtual void process_text(const std::string& text);
	/// draw some text at cursor position and update cursor position
	virtual void draw_text(const std::string& text);

	virtual int query_integer_constant(ContextIntegerConstant cic) const = 0;

	virtual void put_id(void* handle, void* ptr) const = 0;

	virtual cgv::data::component_format texture_find_best_format(
							const cgv::data::component_format& cf, 
							render_component& rc, const std::vector<cgv::data::data_view>* palettes = 0) const = 0;
	
	virtual bool texture_create(
							texture_base& tb, 
							cgv::data::data_format& df) = 0;
	
	virtual bool texture_create(
							texture_base& tb, 
							cgv::data::data_format& target_format, 
							const cgv::data::const_data_view& data, 
							int level, int cube_side = -1, const std::vector<cgv::data::data_view>* palettes = 0) = 0;
	
	virtual bool texture_create_from_buffer(
							texture_base& tb, 
							cgv::data::data_format& df, 
							int x, int y, int level) = 0;
	
	virtual bool texture_replace(
							texture_base& tb, 
							int x, int y, int z_or_cube_side, 
							const cgv::data::const_data_view& data, 
							int level, const std::vector<cgv::data::data_view>* palettes = 0) = 0;

	virtual bool texture_replace_from_buffer(
							texture_base& tb, 
							int x, int y, int z_or_cube_side, 
							int x_buffer, int y_buffer, 
							unsigned int width, unsigned int height, 
							int level) = 0;

	virtual bool texture_generate_mipmaps(
							texture_base& tb, 
							unsigned int dim) = 0;

	virtual bool texture_destruct(render_component& rc) = 0;

	virtual bool texture_set_state(const texture_base& ts) = 0;
	
	virtual bool texture_enable(
							texture_base& tb, 
							int tex_unit, unsigned int nr_dims) = 0;

	virtual bool texture_disable(
							const texture_base& tb, 
							int tex_unit, unsigned int nr_dims) = 0;

	virtual bool render_buffer_create(
							render_component& rc, 
							cgv::data::component_format& cf, 
							int& _width, int& _height) = 0;

	virtual bool render_buffer_destruct(render_component& rc) = 0;

	virtual bool frame_buffer_create(frame_buffer_base& fbb) = 0;

	virtual bool frame_buffer_attach(frame_buffer_base& fbb, 
									 const render_component& rb, bool is_depth, int i) = 0;

	virtual bool frame_buffer_attach(frame_buffer_base& fbb, 
												 const texture_base& t, bool is_depth, 
												 int level, int i, int z) = 0;

	virtual bool frame_buffer_is_complete(const frame_buffer_base& fbb) const = 0;

	virtual bool frame_buffer_enable(frame_buffer_base& fbb) = 0;

	virtual bool frame_buffer_disable(frame_buffer_base& fbb) = 0;

	virtual bool frame_buffer_destruct(frame_buffer_base& fbb) = 0;

	virtual int frame_buffer_get_max_nr_color_attachments() = 0;

	virtual int frame_buffer_get_max_nr_draw_buffers() = 0;

	virtual void* shader_code_create(const std::string& source, ShaderType st, std::string& last_error) = 0;
	virtual bool shader_code_compile(void* handle, std::string& last_error) = 0;
	virtual void shader_code_destruct(void* handle) = 0;

	virtual bool shader_program_create(void* &handle, std::string& last_error) = 0;
	virtual void shader_program_attach(void* handle, void* code_handle) = 0;
	virtual bool shader_program_link(void* handle, std::string& last_error) = 0;
	virtual bool shader_program_set_state(shader_program_base& spb) = 0;
	virtual bool shader_program_enable(render_component& rc) = 0;
	virtual bool set_uniform_void(void* handle, const std::string& name, int value_type, bool dimension_independent, const void* value_ptr, std::string& last_error) = 0;
	virtual bool shader_program_disable(render_component& rc) = 0;
	virtual void shader_program_detach(void* handle, void* code_handle) = 0;
	virtual void shader_program_destruct(void* handle) = 0;

public:
	friend class CGV_API render_component;
	friend class CGV_API texture;
	friend class CGV_API render_buffer;
	friend class CGV_API frame_buffer;
	friend class CGV_API shader_code;
	friend class CGV_API shader_program;
	/// declare type of matrices
	typedef cgv::math::mat<double> mat_type;
	/// declare type of vectors
	typedef cgv::math::vec<double> vec_type;
protected:
	/// keep two matrix stacks for view and projection matrices
	std::stack<mat_type> V_stack, P_stack;
public:
	/// init the cursor position to (0,0)
	context();
	/// virtual destructor
	virtual ~context();

	/**@name interface for implementation of specific contexts*/
	//@{
	///
	virtual void init_render_pass();
	///
	virtual cgv::base::group* get_group_interface();
	/// 
	virtual void draw_textual_info();
	///
	virtual void perform_screen_shot();
	///
	virtual void finish_render_pass();
	//@}

	/**@name render process*/
	//@{
	/// helper method to integrate a new child
	virtual void configure_new_child(cgv::base::base_ptr child);
	/// return the used rendering API
	virtual RenderAPI get_render_api() const = 0;
	/// return the current render pass
	virtual RenderPass get_render_pass() const;
	/// return the current render pass flags
	virtual RenderPassFlags get_render_pass_flags() const;
	/// return the current render pass user data
	virtual void* get_render_pass_user_data() const;
	/// return the default render pass flags
	virtual RenderPassFlags get_default_render_pass_flags() const;
	/// return the default render pass flags
	virtual void set_default_render_pass_flags(RenderPassFlags);
	/// perform the given render task
	virtual void render_pass(RenderPass render_pass = RP_MAIN, 
							 RenderPassFlags render_pass_flags = RPF_ALL,
							 void* user_data = 0);
	/// return whether the context is currently in process of rendering
	virtual bool in_render_process() const = 0;
	/// return whether the context is created
	virtual bool is_created() const = 0;
	/// return whether the context is current
	virtual bool is_current() const = 0;
	/// make the current context current if possible
	virtual bool make_current() const = 0;
	/// clear the current context, typically used in multi-threaded rendering to allow usage of context in several threads
	virtual void clear_current() const = 0;
	//@}

	/// return the width of the window
	virtual unsigned int get_width() const = 0;
	/// return the height of the window
	virtual unsigned int get_height() const = 0;
	/// resize the context to the given dimensions
	virtual void resize(unsigned int width, unsigned int height) = 0;
	/// return whether alpha buffer is attached
	virtual bool is_alpha_buffer_attached() const = 0;
	/// attach an alpha buffer to the current frame buffer if not present
	virtual void attach_alpha_buffer() = 0;
	/// detach the alpha buffer if present
	virtual void detach_alpha_buffer() = 0;
	/// return whether stencil buffer is attached
	virtual bool is_stencil_buffer_attached() const = 0;
	/// attach a stencil buffer to the current frame buffer if not present
	virtual void attach_stencil_buffer() = 0;
	/// detach the stencil buffer if present
	virtual void detach_stencil_buffer() = 0;
	/// return whether the graphics card supports quad buffer mode
	virtual bool is_quad_buffer_supported() const = 0;
	/// return whether quad buffer is attached
	virtual bool is_quad_buffer_attached() const = 0;
	/// attach a quad buffer to the current frame buffer if not present
	virtual void attach_quad_buffer() = 0;
	/// detach the quad buffer if present
	virtual void detach_quad_buffer() = 0;
	/// return whether accumulation buffer is attached
	virtual bool is_accum_buffer_attached() const = 0;
	/// attach a accumulation buffer to the current frame buffer if not present
	virtual void attach_accum_buffer() = 0;
	/// detach the accumulation buffer if present
	virtual void detach_accum_buffer() = 0;
	/// return whether multisampling is enabled
	virtual bool is_multisample_enabled() const = 0;
	/// enable multi sampling
	virtual void enable_multisample() = 0;
	/// disable multi sampling
	virtual void disable_multisample() = 0;

	/** read the current frame buffer or a rectangular region of it into the given
	    data view.
		 If no format is associated with the data view, a new format is created
		 and assigned to the data view. 
		 
		 If width and height are not specified in the format associated with the
		 data view or in the parameters, they are determined from the current 
		 viewport size.

		 x and y specify the pixel offset of the region measured from the left
		 upper corner.
		 
		 The parameter buffer_type is only used in case of reading color or
		 alpha components and specifies from which color buffer to read the data.
	*/
	virtual bool read_frame_buffer(
		data::data_view& dv, 
		unsigned int x = 0, unsigned int y = 0, 
		FrameBufferType buffer_type = FB_BACK,
		TypeId type = type::info::TI_UINT8,
		data::ComponentFormat cf = data::CF_RGB,
		int w = -1, int h = -1) = 0;
	/** write the content of the frame buffer to an image file. In case of writing a depth buffer
       a the depth offset is subtracted from the value and scaled by the depth scale before conversion
		 to an unsigned int of bit depth 8 is performed. */
	bool write_frame_buffer_to_image(
		const std::string& file_name, 
		data::ComponentFormat cf = data::CF_RGB,
		FrameBufferType buffer_type = FB_BACK,
		unsigned int x = 0, unsigned int y = 0, 
		int w = -1, int h = -1,
		float depth_offset = 0.9f, float depth_scale = 10.0f);
	/// set a user defined background color
	virtual void set_bg_color(float r, float g, float b, float a);
	/// set a user defined background alpha value
	virtual void set_bg_alpha(float a);
	/// set a user defined background depth value
	virtual void set_bg_depth(float d);
	/// set a user defined background stencil value
	virtual void set_bg_stencil(int s);
	/// set a user defined background color for the accumulation buffer
	virtual void set_bg_accum_color(float r, float g, float b, float a);
	/// set a user defined background alpha value for the accumulation buffer
	virtual void set_bg_accum_alpha(float a);
	/// set an indexed background color
	virtual void set_bg_clr_idx(unsigned int idx);
	/// return the current index of the background color
	unsigned int get_bg_clr_idx() const;
	/// copy the current back ground rgba color into the given float array
	void put_bg_color(float* rgba) const;
	/// return the current alpha value for clearing the background
	float get_bg_alpha() const;
	/// copy the current back ground rgba color of the accumulation buffer into the given float array
	void put_bg_accum_color(float* rgba) const;
	/// return the current alpha value for clearing the accumulation buffer
	float get_bg_accum_alpha() const;
	/// return the current depth value for clearing the background
	float get_bg_depth() const;
	/// return the current stencil value for clearing the background
	int get_bg_stencil() const;
	/// the context will be redrawn when the system is idle again
	virtual void post_redraw() = 0;
	/// the context will be redrawn right now. This method cannot be called inside the following methods of a drawable: init, init_frame, draw, finish_draw
	virtual void force_redraw() = 0;

	/**@name font selection and measure*/
	//@{
	/// enable the given font face with the given size in pixels
	virtual void enable_font_face(media::font::font_face_ptr font_face, float font_size) = 0;
	/// return the size in pixels of the currently enabled font face
	virtual float get_current_font_size() const;
	/// return the currently enabled font face
	virtual media::font::font_face_ptr get_current_font_face() const;
	//@}

	/**@name light and materials management*/
	//@{
	/// enable phong shading with the help of a shader (enabled by default)
	virtual void enable_phong_shading();
	/// disable phong shading
	virtual void disable_phong_shading();
	/// enable a material without textures
	virtual void enable_material(const cgv::media::illum::phong_material& mat, MaterialSide ms = MS_FRONT_AND_BACK, float alpha = 1) = 0;
	/// disable phong material
	virtual void disable_material(const cgv::media::illum::phong_material& mat) = 0;
	/// enable a material with textures
	virtual void enable_material(const textured_material& mat, MaterialSide ms = MS_FRONT_AND_BACK, float alpha = 1) = 0;
	/// disable phong material
	virtual void disable_material(const textured_material& mat) = 0;
	/// return maximum number of supported light sources
	virtual unsigned get_max_nr_lights() const = 0;
	/// enable a light source and return a handled to be used for disabling, if no more light source unit available 0 is returned
	virtual void* enable_light(const cgv::media::illum::light_source& light) = 0;
	/// disable a previously enabled light
	virtual void disable_light(void* handle) = 0;
	//@}

	/**@name text output*/
	//@{
	/** returns an output stream whose output is printed at the current cursor 
	    location, which is managed by the context. The coordinate system of the
		 cursor location corresponds to window / mouse coordinates. The cursor position is
		 updated during text drawing also by special characters like tab or new line and
		 can be read back with the get_cursor method. Use the flush method of the 
		 output_stream to ensure that text has been drawn to the context. */
	virtual std::ostream& output_stream();
	/** flush the output_stream and set a new cursor position given in 
		 window/mouse coordinates */
	virtual void set_cursor(int x, int y);
	/// return current cursor location in mouse coordinates
	virtual void get_cursor(int& x, int& y) const;
	/** transform point p in current world coordinates into cursor coordinates 
		 and put x and y coordinates into the passed variables */
	virtual void put_cursor_coords(const vec_type& p, int& x, int& y) const = 0;
	/** flush output_stream and set the current text position from a 3D or 4D
		 location in current world coordinates. These are transformed to mouse
		 coordinates with the put_cursor_coords method. If the optional parameters
		 are given, update the cursor location such that the given text alignment
		 is achieved. */
	virtual void set_cursor(const cgv::math::vec<float>& pos, 
		const std::string& text = "", TextAlignment ta = TA_BOTTOM_LEFT,
		int x_offset=0, int y_offset=0);
	/// same as with float version of vec
	virtual void set_cursor(const cgv::math::vec<double>& pos, 
		const std::string& text = "", TextAlignment ta = TA_BOTTOM_LEFT,
		int x_offset=0, int y_offset=0);
	//@}

	/**@name tesselation*/
	//@{
	/// tesselate with independent faces
	virtual void draw_faces(
		const double* vertices, const double* normals, const double* tex_coords, 
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, 
		int nr_faces, int face_degree, bool flip_normals = false) const = 0;
	/// tesselate with one face strip
	virtual void draw_strip_or_fan(
		const double* vertices, const double* normals, const double* tex_coords, 
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, 
		int nr_faces, int face_degree, bool is_fan, bool flip_normals = false) const = 0;

	/// tesselate a unit square in the xy-plane with texture coordinates
	void tesselate_unit_square(bool flip_normals = false);
	/// tesselate a unit cube with extent from [-1,-1,-1] to [1,1,1] with face normals that can be flipped
	void tesselate_unit_cube(bool flip_normals = false);
	/// tesselate a prism 
	void tesselate_unit_prism(bool flip_normals = false);
	/// tesselate a circular disk of radius 1
	void tesselate_unit_disk(int resolution = 25, bool flip_normals = false);
	/// tesselate a cone of radius 1
	void tesselate_unit_cone(int resolution = 25, bool flip_normals = false);
	/// tesselate a cylinder of radius 1
	void tesselate_unit_cylinder(int resolution = 25, bool flip_normals = false);
	/// tesselate a sphere of radius 1
	void tesselate_unit_sphere(int resolution = 25, bool flip_normals = false);
	/// tesselate a tetrahedron
	void tesselate_unit_tetrahedron(bool flip_normals = false);
	/// tesselate a octahedron
	void tesselate_unit_octahedron(bool flip_normals = false);
	/// tesselate a dodecahedron
	void tesselate_unit_dodecahedron(bool flip_normals = false);
	/// tesselate an icosahedron
	void tesselate_unit_icosahedron(bool flip_normals = false);
	/// tesselate a torus with major radius of one and given minor radius
	void tesselate_unit_torus(float minor_radius = 0.2f, int resolution = 25, bool flip_normals = false);
	//! tesselate an arrow from the origin in z-direction
	/*! An arrow of length L is composed of a cylinder of radius R and a cone of radius r.
	    The parameters are
		@param[in] length the total length of the radius
		@param[in] aspect is defined as R/L
		@param[in] rel_tip_radius is defined as r/R
		@param[in] tip_aspect is defined as r/l
	*/
	virtual void tesselate_arrow(double length = 1, double aspect = 0.1, double rel_tip_radius = 2.0, double tip_aspect = 0.3);
	/// define length and direction from start and end point and draw an arrow
	virtual void tesselate_arrow(const cgv::math::fvec<double,3>& start, const cgv::math::fvec<double,3>& end, double aspect = 0.1f, double rel_tip_radius = 2.0f, double tip_aspect = 0.3f);
	//! draw a light source with an emissive material 
	/*! @param[in] l to be rendered light source
	    @param[in] intensity_scale used to multiply with the light source values
	*/
	virtual void draw_light_source(const cgv::media::illum::light_source& l, float intensity_scale, float light_scale); 
	//@}

	/**@name transformations*/
	//@{
	/** use this to push transformation matrices on the stack such that 
	    x and y coordinates correspond to window coordinates, i.e. the 
		 coordinates of the mouse pointer and the cursor for text output. */
	virtual void push_pixel_coords() = 0;
	/// pop previously changed transformation matrices 
	virtual void pop_pixel_coords() = 0;
	/// return homogeneous 4x4 viewing matrix, which transforms from world to eye space
	virtual mat_type get_V() const = 0;
	/// set the current viewing matrix, which transforms from world to eye space
	virtual void set_V(const mat_type& V) const = 0;
	//! push the current viewing matrix onto a matrix stack for viewing matrices.
	/*! A software implementation is used for the matrix stack as some hardware 
	    stacks - i.e. in opengl - have strong limitations on their maximum size. 
		The push_V method does not change the current viewing matrix similarly to
		the glPushMatrix function. Use pop_V() to restore the pushed viewing matrix
		into the current viewing matrix. Don't intermix these methods with the 
		correspondong opengl or directx functions.*/
	void push_V();
	/// see push_V for an explanation
	void pop_V();
	/// return homogeneous 4x4 projection matrix, which transforms from eye to clip space
	virtual mat_type get_P() const = 0;
	/// set the current projection matrix, which transforms from eye to clip space
	virtual void set_P(const mat_type& P) const = 0;
	/// same as push_V but for the projection matrix - a different matrix stack is used.
	void push_P();
	/// see push_P for an explanation
	void pop_P();
	/// return homogeneous 4x4 projection matrix, which transforms from clip to device space
	virtual mat_type get_D() const = 0;
	/// return homogeneous 4x4 matrix, which transforms from world to device space
	virtual mat_type get_DPV() const;
	/// read the device z-coordinate from the z-buffer for the given device x- and y-coordinates
	virtual double get_z_D(int x_D, int y_D) const = 0;
	/// compute the location in world space of a device x/y-location. For this the device point is extended with the device z-coordinate currently stored in the displayed depth buffer.
	vec_type get_point_W(int x_D, int y_D) const;
	/// compute the location in world space of a device x/y-location by inversion of the given transformation from world to device space. For this the device point is extended with the device z-coordinate currently stored in the displayed depth buffer.
	vec_type get_point_W(int x_D, int y_D, const mat_type& DPV) const;
	/// compute the location in world space of a device point. For this the current world to device transformation get_DPV is inverted.
	vec_type get_point_W(int x_D, int y_D, double z_D) const;
	/// compute the location in world space of a device point by inversion of the given world to device transformation.
	vec_type get_point_W(int x_D, int y_D, double z_D, const mat_type& DPV) const;
	/// compute a the location in world space of a device point.
	vec_type get_point_W(const vec_type& p_D) const;
	/// compute a the location in world space of a device point.
	virtual vec_type get_point_W(const vec_type& p_D, const mat_type& MPD) const;
	//@}
};

/** construct a context of the given size. This is primarily used to create
    a context without a window for console applications that render into a frame
    buffer object only. The newly created context will be current right after
	 creation. After usage you need to delete the context by hand. */
extern CGV_API context* create_context(RenderAPI api = RA_OPENGL, 
		unsigned int w = 800, unsigned int h = 600, 
		const std::string& title = "", bool show = false);

typedef context* (*context_creation_function_type)(RenderAPI api, unsigned int w, unsigned int h, const std::string& title, bool show);

/// registration context creation functions
extern CGV_API void register_context_factory(context_creation_function_type);

struct CGV_API context_factory_registration
{
	context_factory_registration(context_creation_function_type fp);
};

	}
}

#include <cgv/config/lib_end.h>