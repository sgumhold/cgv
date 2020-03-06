#pragma once

#define _USE_MATH_DEFINES
#include <cgv/defines/deprecated.h>
#include <cgv/data/data_view.h>
#include <cgv/media/font/font.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/media/illum/phong_material.hh>
#include <cgv/media/illum/textured_surface_material.h>
#include <cgv/media/illum/light_source.hh>
#include <cgv/signal/callback_stream.h>
#include <cgv/render/render_types.h>
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

/// different compond types for data elements
enum ElementType {
	ET_VALUE,
	ET_VECTOR,
	ET_MATRIX
};

/// compact type description of data that can be sent to the context; convertible to int
struct type_descriptor
{
	cgv::type::info::TypeId coordinate_type : 8;
	ElementType element_type : 4;
	unsigned nr_rows : 4;
	unsigned nr_columns : 4;
	bool is_row_major : 1;
	bool is_array : 1;
	bool normalize : 1;
	/// construct from int
	type_descriptor(int td = 0) { *reinterpret_cast<int*>(this) = td; }
	/// construct descriptor for values
	type_descriptor(cgv::type::info::TypeId _coordinate_type, bool _normalize = false) : coordinate_type(_coordinate_type), element_type(ET_VALUE), nr_rows(1), nr_columns(1), is_row_major(false), is_array(false), normalize(_normalize) {}
	/// construct descriptor for vectors
	type_descriptor(cgv::type::info::TypeId _coordinate_type, unsigned _nr_entries, bool _normalize = false) : coordinate_type(_coordinate_type), element_type(ET_VECTOR), nr_rows(_nr_entries), nr_columns(1), is_row_major(false), is_array(false), normalize(_normalize) {}
	/// construct descriptor for matrices
	type_descriptor(cgv::type::info::TypeId _coordinate_type, unsigned _nr_rows, unsigned _nr_cols, bool _is_row_major, bool _normalize = false) : coordinate_type(_coordinate_type), element_type(ET_MATRIX), nr_rows(_nr_rows), nr_columns(_nr_cols), is_row_major(_is_row_major), is_array(false), normalize(_normalize) {}
	/// construct descriptor for an array
	type_descriptor(const type_descriptor& td, bool _is_array) : coordinate_type(td.coordinate_type), element_type(td.element_type), nr_rows(td.nr_rows), nr_columns(td.nr_columns), is_row_major(td.is_row_major), normalize(td.normalize), is_array(_is_array) {}
	/// cast to int
	operator int() const { return *reinterpret_cast<const int*>(this); }
};

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

/// convert render pass type into string
extern CGV_API std::string get_render_pass_name(RenderPass rp);

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
	RPF_DEFAULT = RPF_ALL & ~ (RPF_CLEAR_ACCUM|RPF_SET_CLEAR_ACCUM|RPF_CLEAR_STENCIL|RPF_SET_CLEAR_STENCIL)  // all flags set, defines default render pass
};

/// different sides of a material
enum MaterialSide { MS_NONE, MS_FRONT, MS_BACK, MS_FRONT_AND_BACK };

/// different illumination modes
enum IlluminationMode {
	IM_OFF, IM_ONE_SIDED, IM_TWO_SIDED
};

/// different culling modes
enum CullingMode {
	CM_OFF, CM_BACKFACE, CM_FRONTFACE
};


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

/// different sampling strategies for rendering to textures that steer the computation of the \c tex_coord input to the fragment shader
enum CompareFunction
{
	CF_LEQUAL,
	CF_GEQUAL,
	CF_LESS,
	CF_GREATER,
	CF_EQUAL,
	CF_NOTEQUAL,
	CF_ALWAYS,
	CF_NEVER
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
	const context* ctx_ptr;
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
	CompareFunction compare_function;
	bool use_compare_function;
	TextureType  tt;
	bool have_mipmaps;
	/// initialize members
	texture_base(TextureType _tt = TT_UNDEF);
};


/// base interface for shader programs
class CGV_API shader_program_base : public render_component
{
protected:
	bool is_enabled;
	friend class context;

	bool auto_detect_uniforms;
	bool auto_detect_vertex_attributes;

	// uniforms
	bool uses_view;
	bool uses_material;
	bool uses_lights;
	bool uses_gamma;
	
	// vertex attribute names
	int position_index;
	int color_index;
	int normal_index;
	int texcoord_index;

public:
	PrimitiveType geometry_shader_input_type;
	PrimitiveType geometry_shader_output_type;
	int geometry_shader_output_count;
	/// initializes members
	shader_program_base();
	// configure program
	void specify_standard_uniforms(bool view, bool material, bool lights, bool gamma);
	void specify_standard_vertex_attribute_names(context& ctx, bool color = true, bool normal = true, bool texcoord = true);
	void specify_vertex_attribute_names(context& ctx, const std::string& position, const std::string& color = "", const std::string& normal = "", const std::string& texcoord = "");
	// uniforms
	bool does_use_view() const { return uses_view; }
	bool does_use_material() const { return uses_material; }
	bool does_use_lights() const { return uses_lights; }
	bool does_use_gamma() const { return uses_gamma; }

	// vertex attribute names
	int get_position_index() const { return position_index; }
	int get_color_index() const { return color_index; }
	int get_normal_index() const { return normal_index; }
	int get_texcoord_index() const { return texcoord_index; }
};

/// base class for attribute_array_bindings
class CGV_API attribute_array_binding_base : public render_component
{
protected:
	bool is_enabled;
	friend class context;
public:
	/// nothing to be done heremembers
	attribute_array_binding_base();
};


/// different vertex buffer types
enum VertexBufferType {
	VBT_UNDEF = -1,
	VBT_VERTICES,
	VBT_INDICES,
	VBT_TEXTURE,
	VBT_UNIFORM,
	VBT_FEEDBACK
};

/// different vertex buffer usages as defined in OpenGL
enum VertexBufferUsage {
	VBU_STREAM_DRAW, VBU_STREAM_READ, VBU_STREAM_COPY, VBU_STATIC_DRAW, VBU_STATIC_READ, VBU_STATIC_COPY, VBU_DYNAMIC_DRAW, VBU_DYNAMIC_READ, VBU_DYNAMIC_COPY
};

/// base interface for a vertex buffer
class CGV_API vertex_buffer_base : public render_component
{
public:
	/// buffer type defaults to VBT_VERTICES
	VertexBufferType type;
	/// usage defaults to VBU_STATIC_DRAW
	VertexBufferUsage usage;
	/// initialize members
	vertex_buffer_base();
};


/// base interface for framebuffer
class CGV_API frame_buffer_base : public render_component
{
protected:
	friend class context;
	bool is_enabled;
	std::vector<int> enabled_color_attachments;
	bool attached[16];
	int width, height;
public:
	/// initialize members
	frame_buffer_base();
};

/// different shader types
enum ShaderType { ST_DETECT, ST_COMPUTE, ST_VERTEX, ST_TESS_CONTROL, ST_TESS_EVALUTION, ST_GEOMETRY, ST_FRAGMENT };

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

/** configuration object used to define context parameters that need to be set already at creation time */
struct CGV_API context_config
{
	/**@name context creation parameters*/
	//@{
	/// default: true
	bool depth_buffer;
	/// default: true
	bool double_buffer;
	/// default: false
	bool alpha_buffer;
	/// default: false
	bool stereo_buffer;
	/// default: false
	bool stencil_buffer;
	/// default: false
	bool accumulation_buffer;
	/// default: false
	bool multi_sample_buffer;
	/// default: -1
	int depth_bits;
	/// default: -1
	int  stencil_bits;
	/// default: -1
	int  accumulation_bits;
	/// default: -1
	int  nr_multi_samples;

	/// default: -1 ... major version of maximum supported OpenGL version
	int  version_major;
	/// default: -1 ... minor version of maximum supported OpenGL version
	int  version_minor;
	/// default: false
	bool forward_compatible;
	/// default: false in release and true in debug version
	bool debug;
	/// default: false
	bool core_profile;
	//@}
	/// construct config with default parameters
	context_config();
	/// reflect the shader_path member
	bool self_reflect(cgv::reflect::reflection_handler& srh);
};

/// type of ref counted pointer to context creation configuration
typedef cgv::data::ref_ptr<context_config> context_config_ptr;

/** configuration object used to define render view creation parameters including error handling configuration */
struct CGV_API render_config : public cgv::base::base, public context_config
{
	/**@name window creation parameters*/
	//@{
	/// default: -1 ... no fullscreen
	int fullscreen_monitor;
	/// default: 640
	int window_width;
	/// default: 480
	int window_height;
	//@}

	/**@name error handling */
	//@{
	/// default: false
	bool abort_on_error;
	/// default: true (only in case a gui_driver, which supports this, is loaded)
	bool dialog_on_error;
	/// default: true
	bool show_error_on_console;
	//@}

	/// construct config with default parameters
	render_config();
	/// return "render_config"
	std::string get_type_name() const;
	/// reflect the shader_path member
	bool self_reflect(cgv::reflect::reflection_handler& srh);
};

/// type of ref counted pointer to render configuration
typedef cgv::data::ref_ptr<render_config> render_config_ptr;

/// return a pointer to the current shader configuration
extern CGV_API render_config_ptr get_render_config();

/// parameters necessary to define window transformation
struct window_transformation
{
	/// viewport parameters [x0,y0,width,height]
	render_types::ivec4 viewport;
	/// range of depth values [min_depth, max_depth]
	render_types::dvec2 depth_range;
};

/** base class for all drawables, which is independent of the used rendering API. */
class CGV_API context : public render_types, public context_config
{
public:
	friend class CGV_API attribute_array_manager;
	friend class CGV_API render_component;
	friend class CGV_API texture;
	friend class CGV_API render_buffer;
	friend class CGV_API frame_buffer;
	friend class CGV_API shader_code;
	friend class CGV_API shader_program;
	friend class CGV_API attribute_array_binding;
	friend class CGV_API vertex_buffer;
	/// dimension independent type of vectors
	typedef cgv::math::vec<float> vec_type;
	/// dimension independent type of matrices
	typedef cgv::math::mat<float> mat_type;
	/// dimension independent type of vectors
	typedef cgv::math::vec<double> dvec_type;
	/// dimension independent type of matrices
	typedef cgv::math::mat<double> dmat_type;
protected:
	friend class shader_program_base;

	/// whether to automatically set viewing matrixes in current shader program, defaults to true 
	bool auto_set_view_in_current_shader_program;
	/// whether to automatically set lights in current shader program, defaults to true 
	bool auto_set_lights_in_current_shader_program;
	/// whether to automatically set material in current shader program, defaults to true 
	bool auto_set_material_in_current_shader_program;
	/// whether to automatically set gamma in current shader program, defaults to true 
	bool auto_set_gamma_in_current_shader_program;
	/// whether to support view and lighting management of compatibility mode, defaults to true
	bool support_compatibility_mode;
	/// whether to do all drawing in compatibility mode, only possible if support_compatibility_mode is true, , defaults to false
	bool draw_in_compatibility_mode;
	/// whether to debug render passes
	bool debug_render_passes;
	/// whether vsynch should be enabled
	bool enable_vsynch;
	/// whether to use opengl option to support sRGB framebuffer
	bool sRGB_framebuffer;
	/// gamma value passed to shader programs that have gamma uniform
	float gamma;
	/// keep two matrix stacks for model view and projection matrices
	std::stack<dmat4> modelview_matrix_stack, projection_matrix_stack;
	/// keep stack of window transformations
	std::stack<std::vector<window_transformation> > window_transformation_stack;
	/// stack of currently enabled frame buffers
	std::stack<frame_buffer_base*> frame_buffer_stack;
	/// stack of currently enabled shader programs
	std::stack<shader_program_base*> shader_program_stack;
public:
	/// check for current program, prepare it for rendering and return pointer to it
	shader_program_base* get_current_program() const;
protected:
	/// stack of currently enabled attribute array binding
	std::stack<attribute_array_binding_base*> attribute_array_binding_stack;
	/// status information of light sources
	struct light_source_status
	{
		bool enabled;
		vec3 eye_position;
		vec3 eye_spot_direction;
		int light_source_index;
	};
	/// keep track of enabled light source handles
	std::vector<void*> enabled_light_source_handles;
	/// counter to construct light source handles
	size_t light_source_handle;
	/// map handle to light source and light source status information
	std::map<void*, std::pair<cgv::media::illum::light_source, light_source_status> > light_sources;
	/// helper function to send light update events
	virtual void on_lights_changed();
	/// number of default light sources
	static const unsigned nr_default_light_sources = 2;
	/// default light sources
	cgv::media::illum::light_source default_light_source[nr_default_light_sources];
	/// handles of default light sources
	void* default_light_source_handles[nr_default_light_sources];
	/// store a default material
	cgv::media::illum::surface_material default_material;
	/// store pointer to current material
	const cgv::media::illum::surface_material* current_material_ptr;
	/// store flag to tell whether current material is textured
	bool current_material_is_textured;
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
	virtual void destruct_render_objects();
	virtual void put_id(void* handle, void* ptr) const = 0;

	virtual cgv::data::component_format texture_find_best_format(const cgv::data::component_format& cf, render_component& rc, const std::vector<cgv::data::data_view>* palettes = 0) const = 0;
	virtual bool texture_create				(texture_base& tb, cgv::data::data_format& df) const = 0;
	virtual bool texture_create				(texture_base& tb, cgv::data::data_format& target_format, const cgv::data::const_data_view& data, int level, int cube_side = -1, const std::vector<cgv::data::data_view>* palettes = 0) const = 0;
	virtual bool texture_create_from_buffer (texture_base& tb, cgv::data::data_format& df, int x, int y, int level) const = 0;
	virtual bool texture_replace			(texture_base& tb, int x, int y, int z_or_cube_side, const cgv::data::const_data_view& data, int level, const std::vector<cgv::data::data_view>* palettes = 0) const = 0;
	virtual bool texture_replace_from_buffer(texture_base& tb, int x, int y, int z_or_cube_side, int x_buffer, int y_buffer, unsigned int width, unsigned int height, int level) const = 0;
	virtual bool texture_generate_mipmaps	(texture_base& tb, unsigned int dim) const = 0;
	virtual bool texture_destruct           (texture_base& tb) const = 0;
	virtual bool texture_set_state			(const texture_base& tb) const = 0;
	virtual bool texture_enable				(texture_base& tb, int tex_unit, unsigned int nr_dims) const = 0;
	virtual bool texture_disable			(texture_base& tb, int tex_unit, unsigned int nr_dims) const = 0;

	virtual bool render_buffer_create       (render_component& rc, cgv::data::component_format& cf, int& _width, int& _height) const = 0;
	virtual bool render_buffer_destruct     (render_component& rc) const = 0;

	static void get_buffer_list(frame_buffer_base& fbb, std::vector<int>& buffers, int offset = 0);
	virtual bool frame_buffer_create		   (frame_buffer_base& fbb) const;
	virtual bool frame_buffer_attach		   (frame_buffer_base& fbb, const render_component& rb, bool is_depth, int i) const;
	virtual bool frame_buffer_attach		   (frame_buffer_base& fbb, const texture_base& t, bool is_depth, int level, int i, int z) const;
	virtual bool frame_buffer_is_complete(const frame_buffer_base& fbb) const = 0;
	virtual bool frame_buffer_enable		   (frame_buffer_base& fbb);
	virtual bool frame_buffer_disable		   (frame_buffer_base& fbb);
	virtual bool frame_buffer_destruct		   (frame_buffer_base& fbb) const;
	virtual int frame_buffer_get_max_nr_color_attachments() const = 0;
	virtual int frame_buffer_get_max_nr_draw_buffers() const = 0;

	virtual bool shader_code_create  (render_component& sc, ShaderType st, const std::string& source) const = 0;
	virtual bool shader_code_compile (render_component& sc) const = 0;
	virtual void shader_code_destruct(render_component& sc) const = 0;

	virtual bool shader_program_create   (shader_program_base& spb) const = 0;
	virtual void shader_program_attach(shader_program_base& spb, const render_component& sc) const = 0;
	virtual void shader_program_detach(shader_program_base& spb, const render_component& sc) const = 0;
	virtual bool shader_program_link(shader_program_base& spb) const;
	virtual bool shader_program_set_state(shader_program_base& spb) const = 0;
	virtual bool shader_program_enable   (shader_program_base& spb);
	virtual bool shader_program_disable(shader_program_base& spb);
	virtual bool shader_program_destruct(shader_program_base& spb) const;
	virtual int  get_uniform_location(const shader_program_base& spb, const std::string& name) const = 0;
	virtual bool set_uniform_void(shader_program_base& spb, int loc, type_descriptor value_type, const void* value_ptr) const = 0;
	virtual bool set_uniform_array_void(shader_program_base& spb, int loc, type_descriptor value_type, const void* value_ptr, size_t nr_elements) const = 0;
	virtual int  get_attribute_location(const shader_program_base& spb, const std::string& name) const = 0;
	virtual bool set_attribute_void(shader_program_base& spb, int loc, type_descriptor value_type, const void* value_ptr) const = 0;

	virtual bool attribute_array_binding_create  (attribute_array_binding_base& aab) const = 0;
	virtual bool attribute_array_binding_destruct(attribute_array_binding_base& aab) const;
	virtual bool attribute_array_binding_enable  (attribute_array_binding_base& aab);
	virtual bool attribute_array_binding_disable (attribute_array_binding_base& aab);
	virtual bool set_attribute_array_void(attribute_array_binding_base* aab, int loc, type_descriptor value_type, const vertex_buffer_base* vbb, const void* ptr, size_t nr_elements = 0, unsigned stride_in_bytes = 0) const = 0;
	virtual bool set_element_array(attribute_array_binding_base* aab, const vertex_buffer_base* vbb) const = 0;
	virtual bool enable_attribute_array(attribute_array_binding_base* aab, int loc, bool do_enable) const = 0;
	virtual bool is_attribute_array_enabled(const attribute_array_binding_base* aab, int loc) const = 0;

	virtual bool vertex_buffer_bind(const vertex_buffer_base& vbb, VertexBufferType _type) const = 0;
	virtual bool vertex_buffer_create(vertex_buffer_base& vbb, const void* array_ptr, size_t size_in_bytes) const = 0;
	virtual bool vertex_buffer_replace(vertex_buffer_base& vbb, size_t offset, size_t size_in_bytes, const void* array_ptr) const = 0;
	virtual bool vertex_buffer_copy(const vertex_buffer_base& src, size_t src_offset, vertex_buffer_base& target, size_t target_offset, size_t size_in_bytes) const = 0;
	virtual bool vertex_buffer_copy_back(vertex_buffer_base& vbb, size_t offset, size_t size_in_bytes, void* array_ptr) const = 0;
	virtual bool vertex_buffer_destruct(vertex_buffer_base& vbb) const = 0;
public:
	/// init the cursor position to (0,0)
	context();
	/// virtual destructor
	virtual ~context();
	/// error handling
	virtual void error(const std::string& message, const render_component* rc = 0) const;

	/**@name interface for implementation of specific contexts*/
	//@{
	///
	virtual void init_render_pass();
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
	/// return current render pass recursion depth
	unsigned get_render_pass_recursion_depth() const;
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
	/// set flag whether to debug render passes
	void set_debug_render_passes(bool _debug);
	/// check whether render passes are debugged
	bool get_debug_render_passes() const { return debug_render_passes; }
	/// return whether the context is currently in process of rendering
	virtual bool in_render_process() const = 0;
	/// return whether the context is created
	virtual bool is_created() const = 0;
	/// return whether the context is current
	virtual bool is_current() const = 0;
	/// recreate context based on current context config settings
	virtual bool recreate_context();
	/// make the current context current if possible
	virtual bool make_current() const = 0;
	/// clear the current context, typically used in multi-threaded rendering to allow usage of context in several threads
	virtual void clear_current() const = 0;
	/// attach or detach (\c attach=false) an alpha buffer to the current frame buffer if not present
	virtual void attach_alpha_buffer(bool attach = true) = 0;
	/// attach or detach (\c attach=false) depth buffer to the current frame buffer if not present
	virtual void attach_depth_buffer(bool attach = true) = 0;
	/// attach or detach (\c attach=false) stencil buffer to the current frame buffer if not present
	virtual void attach_stencil_buffer(bool attach = true) = 0;
	/// return whether the graphics card supports stereo buffer mode
	virtual bool is_stereo_buffer_supported() const = 0;
	/// attach or detach (\c attach=false) stereo buffer to the current frame buffer if not present
	virtual void attach_stereo_buffer(bool attach = true) = 0;
	/// attach or detach (\c attach=false) accumulation buffer to the current frame buffer if not present
	virtual void attach_accumulation_buffer(bool attach = true) = 0;
	/// attach or detach (\c attach=false) multi sample buffer to the current frame buffer if not present
	virtual void attach_multi_sample_buffer(bool attach = true) = 0;
	//@}

	/// return the width of the window
	virtual unsigned int get_width() const = 0;
	/// return the height of the window
	virtual unsigned int get_height() const = 0;
	/// resize the context to the given dimensions
	virtual void resize(unsigned int width, unsigned int height) = 0;


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
	/// announce an external frame buffer change performed with rendering API to the cgv framework providing space to temporarily store frame buffer of cgv framework
	virtual void announce_external_frame_buffer_change(void*& cgv_fbo_storage) = 0;
	/// restore cgv frame buffer to the state before the external change
	virtual void recover_from_external_frame_buffer_change(void* cgv_fbo_storage) = 0;

	/**@name font selection and measure*/
	//@{
	/// enable the given font face with the given size in pixels
	virtual void enable_font_face(media::font::font_face_ptr font_face, float font_size) = 0;
	/// return the size in pixels of the currently enabled font face
	virtual float get_current_font_size() const;
	/// return the currently enabled font face
	virtual media::font::font_face_ptr get_current_font_face() const;
	//@}

	/**@name surface rendering and material management*/
	//@{
	DEPRECATED("deprecated and ignored.") virtual void enable_phong_shading();
	DEPRECATED("deprecated and ignored.") virtual void disable_phong_shading();
	DEPRECATED("deprecated, use set_material instead.") virtual void enable_material(const cgv::media::illum::phong_material& mat = cgv::media::illum::default_material(), MaterialSide ms = MS_FRONT_AND_BACK, float alpha = 1);
	DEPRECATED("deprecated and ignored.") virtual void disable_material(const cgv::media::illum::phong_material& mat = cgv::media::illum::default_material());
	DEPRECATED("deprecated, use enable_material(textured_surface_material) instead.") virtual void enable_material(const textured_material& mat, MaterialSide ms = MS_FRONT_AND_BACK, float alpha = 1);
	//DEPRECATED("deprecated, use disable_material(textured_surface_material) instead.") virtual void disable_material(const textured_material& mat) = 0;
	/// set the current gamma values
	virtual void set_gamma(float _gamma);
	/// query current gamma
	float get_gamma() const { return gamma; }
	/// enable or disable sRGB framebuffer
	virtual void enable_sRGB_framebuffer(bool do_enable = true);
	/// check whether sRGB framebuffer is enabled
	bool sRGB_framebuffer_enabled() { return sRGB_framebuffer; }
	/// set the current color
	virtual void set_color(const rgba& clr) = 0;
	/// set the current color
	virtual void set_color(const rgb& clr, float opacity = 1.0f) { set_color(rgba(clr[0], clr[1], clr[2], opacity)); }
	/// set the current material 
	virtual void set_material(const cgv::media::illum::surface_material& mat);
	/// return pointer to current material or nullptr if no current material is available
	const cgv::media::illum::surface_material* get_current_material() const;
	/// set the current material 
	virtual void set_textured_material(const textured_material& mat);
	/// enable a material with textures
	virtual void enable_material(textured_material& mat) = 0;
	/// disable a material with textures
	virtual void disable_material(textured_material& mat) = 0;
	/// set the shader program view matrices to the currently enabled view matrices
	void set_current_view(shader_program& prog, bool modelview_deps = true, bool projection_deps = true) const;
	/// set the shader program material to the currently enabled material
	void set_current_material(shader_program& prog) const;
	/// set the shader program lights to the currently enabled lights
	void set_current_lights(shader_program& prog) const;
	/// helper function to place lights 
	vec3 get_light_eye_position(const cgv::media::illum::light_source& light, bool place_now) const;
	/// helper function to place spot lights 
	vec3 get_light_eye_spot_direction(const cgv::media::illum::light_source& light, bool place_now) const;
	/// return a reference to a shader program used to render without illumination
	virtual shader_program& ref_default_shader_program(bool texture_support = false) = 0;
	/// return a reference to the default shader program used to render surfaces 
	virtual shader_program& ref_surface_shader_program(bool texture_support = false) = 0;
	/// get list of program uniforms
	virtual	void enumerate_program_uniforms(shader_program& prog, std::vector<std::string>& names, std::vector<int>* locations_ptr = 0, std::vector<int>* sizes_ptr = 0, std::vector<int>* types_ptr = 0, bool show = false) const = 0;
	/// get list of program attributes
	virtual	void enumerate_program_attributes(shader_program& prog, std::vector<std::string>& names, std::vector<int>* locations_ptr = 0, std::vector<int>* sizes_ptr = 0, std::vector<int>* types_ptr = 0, bool show = false) const = 0;
	//@}

	/**@name lights */
	//@{
	DEPRECATED("deprecated, use add_light_source instead.") void* enable_light(const cgv::media::illum::light_source& light) { return add_light_source(light); }
	DEPRECATED("deprecated, use enable_light_source instead.") void disable_light(void* handle) { disable_light_source(handle); }
	DEPRECATED("deprecated, use get_max_nr_enabled_light_sources instead.") unsigned get_max_nr_lights() const { return get_max_nr_enabled_light_sources(); }
	/// return the number of light sources
	size_t get_nr_light_sources() const;
	/// add a new light source, enable it if \c enable is true and place it relative to current model view transformation if \c place_now is true; return handle to light source
	void* add_light_source(const cgv::media::illum::light_source& light, bool enabled = true, bool place_now = false);
	/// remove a light source by handle and whether it existed
	bool remove_light_source(void* handle);
	/// read access to light source
	const cgv::media::illum::light_source& get_light_source(void* handle) const;
	/// read access to light source status
	const light_source_status& get_light_source_status(void* handle) const;
	/// set light source newly
	void set_light_source(void* handle, const cgv::media::illum::light_source& light, bool place_now = true);
	/// place the given light source relative to current model viel transformation
	void place_light_source(void* handle);

	/// return maximum number of light sources, that can be enabled in parallel 
	virtual unsigned get_max_nr_enabled_light_sources() const;
	/// return the number of light sources
	size_t get_nr_enabled_light_sources() const;
	/// access to handle of i-th light source
	void* get_enabled_light_source_handle(size_t i) const;
	/// enable a given light source and return whether there existed a light source with given handle
	bool enable_light_source(void* handle);
	/// disable a given light source and return whether there existed a light source with given handle
	bool disable_light_source(void* handle);
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
	virtual void put_cursor_coords(const vec_type& p, int& x, int& y) const;
	/** flush output_stream and set the current text position from a 3D or 4D
		 location in current world coordinates. These are transformed to mouse
		 coordinates with the put_cursor_coords method. If the optional parameters
		 are given, update the cursor location such that the given text alignment
		 is achieved. */
	virtual void set_cursor(const vec_type& pos, 
		const std::string& text = "", TextAlignment ta = TA_BOTTOM_LEFT,
		int x_offset=0, int y_offset=0);
	//@}

	/**@name drawing*/
	//@{
	/// pass geometry of given faces to current shader program and generate draw calls to render lines for the edges
	virtual void draw_edges_of_faces(
		const float* vertices, const float* normals, const float* tex_coords,
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, 
		int nr_faces, int face_degree, bool flip_normals = false) const = 0;
	/// pass geometry of given strip or fan to current shader program and generate draw calls to render lines for the edges
	virtual void draw_edges_of_strip_or_fan(
		const float* vertices, const float* normals, const float* tex_coords,
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, 
		int nr_faces, int face_degree, bool is_fan, bool flip_normals = false) const = 0;
	/// pass geometry of given faces to current shader program and generate draw calls to render triangles
	virtual void draw_faces(
		const float* vertices, const float* normals, const float* tex_coords,
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices,
		int nr_faces, int face_degree, bool flip_normals = false) const = 0;
	/// pass geometry of given strip or fan to current shader program and generate draw calls to render triangles
	virtual void draw_strip_or_fan(
		const float* vertices, const float* normals, const float* tex_coords,
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices,
		int nr_faces, int face_degree, bool is_fan, bool flip_normals = false) const = 0;

	/// tesselate a unit square in the xy-plane with texture coordinates
	void tesselate_unit_square(bool flip_normals = false, bool edges = false);
	/// tesselate a unit cube with extent from [-1,-1,-1] to [1,1,1] with face normals that can be flipped
	void tesselate_unit_cube(bool flip_normals = false, bool edges = false);
	/// tesselate an axis aligned box
	virtual void tesselate_box(const cgv::media::axis_aligned_box<double, 3>& B, bool flip_normals, bool edges = false) const;
	/// tesselate a prism 
	void tesselate_unit_prism(bool flip_normals = false, bool edges = false);
	/// tesselate a circular disk of radius 1
	void tesselate_unit_disk(int resolution = 25, bool flip_normals = false, bool edges = false);
	/// tesselate a cone of radius 1
	void tesselate_unit_cone(int resolution = 25, bool flip_normals = false, bool edges = false);
	/// tesselate a cylinder of radius 1
	void tesselate_unit_cylinder(int resolution = 25, bool flip_normals = false, bool edges = false);
	/// tesselate a sphere of radius 1
	void tesselate_unit_sphere(int resolution = 25, bool flip_normals = false, bool edges = false);
	/// tesselate a tetrahedron
	void tesselate_unit_tetrahedron(bool flip_normals = false, bool edges = false);
	/// tesselate a octahedron
	void tesselate_unit_octahedron(bool flip_normals = false, bool edges = false);
	/// tesselate a dodecahedron
	void tesselate_unit_dodecahedron(bool flip_normals = false, bool edges = false);
	/// tesselate an icosahedron
	void tesselate_unit_icosahedron(bool flip_normals = false, bool edges = false);
	/// tesselate a torus with major radius of one and given minor radius
	void tesselate_unit_torus(float minor_radius = 0.2f, int resolution = 25, bool flip_normals = false, bool edges = false);
	//! tesselate an arrow from the origin in z-direction
	/*! An arrow of length L is composed of a cylinder of radius R and a cone of radius r.
	    The parameters are
		@param[in] length the total length of the radius
		@param[in] aspect is defined as R/L
		@param[in] rel_tip_radius is defined as r/R
		@param[in] tip_aspect is defined as r/l
	*/
	virtual void tesselate_arrow(double length = 1, double aspect = 0.1, double rel_tip_radius = 2.0, double tip_aspect = 0.3, int res = 25, bool edges = false);
	/// define length and direction from start and end point and draw an arrow
	virtual void tesselate_arrow(const dvec3& start, const dvec3& end, double aspect = 0.1f, double rel_tip_radius = 2.0f, double tip_aspect = 0.3f, int res = 25, bool edges = false);
	//! draw a light source with an emissive material 
	/*! @param[in] l to be rendered light source
	    @param[in] intensity_scale used to multiply with the light source values
	*/
	virtual void draw_light_source(const cgv::media::illum::light_source& l, float intensity_scale, float light_scale); 
	//@}

	/**@name transformations*/
	//@{
	DEPRECATED("deprecated: use get_modelview_matrix() instead.") dmat_type get_V() const { return dmat_type(4,4,&get_modelview_matrix()(0,0)); }
	DEPRECATED("deprecated: use set_modelview_matrix() instead.") void set_V(const dmat_type& V) const { const_cast<context*>(this)->set_modelview_matrix(dmat4(4,4,&V(0,0))); }
	DEPRECATED("deprecated: use push_modelview_matrix() instead.") void push_V() { push_modelview_matrix(); }
	DEPRECATED("deprecated: use pop_modelview_matrix() instead.") void pop_V() { pop_modelview_matrix(); }
	DEPRECATED("deprecated: use get_projection_matrix() instead.") dmat_type get_P() const { return dmat_type(4, 4, &get_projection_matrix()(0, 0)); }
	DEPRECATED("deprecated: use set_projection_matrix() instead.") void set_P(const dmat_type& P) const { const_cast<context*>(this)->set_projection_matrix(dmat4(4,4,&P(0, 0))); }
	DEPRECATED("deprecated: use push_projection_matrix() instead.") void push_P() { push_projection_matrix(); }
	DEPRECATED("deprecated: use pop_projection_matrix() instead.") void pop_P() { pop_projection_matrix(); }
	DEPRECATED("deprecated: use get_device_matrix() instead.") 	dmat_type get_D() const { return dmat_type(4, 4, &get_window_matrix()(0, 0)); }
	DEPRECATED("deprecated: use get_modelview_projection_device_matrix() instead.")	mat_type get_DPV() const { return dmat_type(4, 4, &get_modelview_projection_window_matrix()(0, 0)); }
	/** use this to push transformation matrices on the stack such that
	    x and y coordinates correspond to window coordinates, i.e. the 
		 coordinates of the mouse pointer and the cursor for text output. */
	virtual void push_pixel_coords() = 0;
	/// pop previously changed transformation matrices 
	virtual void pop_pixel_coords() = 0;
	/// return homogeneous 4x4 viewing matrix, which transforms from world to eye space
	virtual dmat4 get_modelview_matrix() const = 0;
	/// set the current modelview matrix, which transforms from world to eye space
	virtual void set_modelview_matrix(const dmat4& MV);
	/// multiply given matrix from right to current modelview matrix
	virtual void mul_modelview_matrix(const dmat4& MV);
	//! push the current viewing matrix onto a matrix stack for viewing matrices.
	/*! A software implementation is used for the matrix stack as some hardware 
	    stacks - i.e. in opengl - have strong limitations on their maximum size. 
		The push_V method does not change the current viewing matrix similarly to
		the glPushMatrix function. Use pop_V() to restore the pushed viewing matrix
		into the current viewing matrix. Don't intermix these methods with the 
		correspondong opengl or directx functions.*/
	void push_modelview_matrix();
	/// see push_V for an explanation
	void pop_modelview_matrix();
	/// return homogeneous 4x4 projection matrix, which transforms from eye to clip space
	virtual dmat4 get_projection_matrix() const = 0;
	/// set the current projection matrix, which transforms from eye to clip space
	virtual void set_projection_matrix(const dmat4& P);
	/// multiply given matrix from right to current projection matrix
	virtual void mul_projection_matrix(const dmat4& P);
	/// same as push_V but for the projection matrix - a different matrix stack is used.
	void push_projection_matrix();
	/// see push_P for an explanation
	void pop_projection_matrix();
	/// push a copy of the current viewport and depth range arrays defining the window transformations
	void push_window_transformation_array();
	//! restore previous viewport and depth range arrays defining the window transformations
	/*! An error is emitted when the method fails because the stack of window 
	    transformations would become empty, which is not allowed. */
	virtual void pop_window_transformation_array();
	/// announce an external viewport change performed with rendering API to the cgv framework providing space to temporarily store viewport of cgv framework
	virtual void announce_external_viewport_change(ivec4& cgv_viewport_storage) = 0;
	/// restore cgv viewport to the state before the external change
	virtual void recover_from_external_viewport_change(const ivec4& cgv_viewport_storage) = 0;
	/// query the maximum number of supported window transformations, which is at least 1 
	virtual unsigned get_max_window_transformation_array_size() const = 0;
protected:
	bool ensure_window_transformation_index(int& array_index);
public:
	//! set the current viewport or one of the viewports in the window transformation array
	/*! If the parameter \c array_index is -1 (for example by not specifying it),
	    the current window transformation array is resized to a single viewport and
		depth range and the viewport is set to the integer vector of pixel values 
		in the \c viewport parameter: [x0,y0,width,height]. If an \c array_index >= 0
		is specified, the window transformation array is resized such that the 
		specified \c array_index is valid and the corresponding viewport is set.
		If resizing generates new viewports or depth ranges, the default values
		are set, which are [0,0,widthOfContext, heightOfContext] for viewports
		and [0.0,1.0] for depth ranges. If resizing increases the number of viewport
		transformations over the allowed number, an error is issued. */
	virtual void set_viewport(const ivec4& viewport, int array_index = -1);
	//! set the current depth range or one of the depth ranges in the window transformation array
	/*! The behaviour with respect to parameters \c array_index is the same as
	    in the set_viewport() method.*/
	virtual void set_depth_range(const dvec2& depth_range = dvec2(0, 1), int array_index = -1);
	/// return the current window transformation array
	const std::vector<window_transformation>& get_window_transformation_array() const;
	//! return a homogeneous 4x4 matrix to transform clip to window coordinates
	/*! In window coordinates x- and y-coordinates correspond to mouse pixel coordinates
	    and z to depth value stored in the depth buffer. This is a different convention as
		in OpenGL where the y-coordinates point from bottom to top instead from top to bottom. 
		Optionally one can specify a window transformation index with the parameter \c array_index
		for the case when an array of window transformations is used.*/
	dmat4 get_window_matrix(unsigned array_index = 0) const;
	/// return a homogeneous 4x4 matrix to transfrom from model to window coordinates, i.e. the product of modelview, projection and device matrix in reversed order (window_matrix*projection_matrix*modelview_matrix)
	dmat4 get_modelview_projection_window_matrix(unsigned array_index = 0) const;
	/// read the window z-coordinate from the depth buffer for the given window x- and y-coordinates
	virtual double get_window_z(int x_window, int y_window) const = 0;
	//! compute model space 3D point from the given window location
	/*! the function queries the window z coordinate from the depth buffer and inversely transforms the
		window space 3D point with the current modelview_projection_window matrix */
	inline vec3 get_model_point(int x_window, int y_window) const { 
		return get_model_point(x_window, y_window, get_window_z(x_window, y_window)); 
	}
	//! compute model space 3D point from the given window coordinates
	/*! the function inversely transforms the window space 3D point with the current
		modelview_projection_window matrix */
	inline vec3 get_model_point(int x_window, int y_window, double z_window) const {
		return get_model_point(x_window, y_window, z_window, get_modelview_projection_window_matrix());
	}
	//! compute model space 3D point from the given window location and modelview_projection_window matrix
	/*! the function queries the window z coordinate from the depth buffer and inversely transforms the
		window space 3D point with the given modelview_projection_window matrix */
	inline vec3 get_model_point(int x_window, int y_window, const dmat4& modelview_projection_window_matrix) const {
		return get_model_point(x_window, y_window, get_window_z(x_window, y_window), modelview_projection_window_matrix);
	}
	//! compute model space 3D point from the given window coordinates with the given modelview_projection_window matrix
	/*! the function inversely transforms the window space 3D point with the given
		modelview_projection_window matrix */
	inline vec3 get_model_point(int x_window, int y_window, double z_window, const dmat4& modelview_projection_window_matrix) const {
		return get_model_point(dvec3(x_window+0.5, y_window+0.5, z_window), modelview_projection_window_matrix);
	}
	//! compute model space 3D point from the given window space point
	/*! the function inversely transforms the window space 3D point with the current
		modelview_projection_window matrix */
	inline vec3 get_model_point(const vec3& p_window) const {
		return get_model_point(p_window, get_modelview_projection_window_matrix());
	}
	//! compute model space 3D point from the given window space point and the given modelview_projection_window matrix
	/*! the function inversely transforms the window space point with the given
		modelview_projection_window matrix */
	vec3 get_model_point(const dvec3& p_window, const dmat4& modelview_projection_window_matrix) const;
	/// return homogeneous 4x4 projection matrix, which transforms from clip to device space
	DEPRECATED("use get_window_matrix() instead.") dmat4 get_device_matrix() const { return get_window_matrix(); }
	/// return matrix to transfrom from model to device coordinates, i.e. the product of modelview, projection and device matrix in reversed order (device_matrix*projection_matrix*modelview_matrix)
	DEPRECATED("use get_modelview_projection_window_matrix() instead.") dmat4 get_modelview_projection_device_matrix() const;
	/// read the window z-coordinate from the z-buffer for the given device x- and y-coordinates
	DEPRECATED("use get_window_z()") double get_z_D(int x_D, int y_D) const { return get_window_z(x_D, y_D); }
	/// compute the location in world space of a device x/y-location. For this the device point is extended with the device z-coordinate currently stored in the displayed depth buffer.
	DEPRECATED("use get_model_point()") vec3 get_point_W(int x_D, int y_D) const { return get_model_point(x_D, y_D); }
	/// compute the location in world space of a device x/y-location by inversion of the given transformation from world to device space. For this the device point is extended with the device z-coordinate currently stored in the displayed depth buffer.
	DEPRECATED("use get_model_point()") vec3 get_point_W(int x_D, int y_D, const dmat4& MPD) const { return get_model_point(x_D, y_D, MPD); }
	/// compute the location in world space of a device point. For this the current world to device transformation is inverted.
	DEPRECATED("use get_model_point()") vec3 get_point_W(int x_D, int y_D, double z_D) const { return get_model_point(x_D, y_D, z_D); }
	/// compute the location in world space of a device point by inversion of the given world to device transformation.
	DEPRECATED("use get_model_point()") vec3 get_point_W(int x_D, int y_D, double z_D, const dmat4& MPD) const { return get_model_point(x_D, y_D, z_D, MPD); }
	/// compute a the location in world space of a device point.
	DEPRECATED("use get_model_point()") vec3 get_point_W(const vec3& p_D) const { return get_model_point(p_D); }
	/// compute a the location in world space of a device point.
	DEPRECATED("use get_model_point()") vec3 get_point_W(const vec3& p_D, const dmat4& MPD) const { return get_model_point(p_D, MPD); }
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