#pragma once

#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {
			namespace gl {


/** OpenGL helper class to simplify depth peeling. Call the init method after your OpenGL
    context has been created. A sample usage during a drawing method would look like:

\code
	 // draw and use first layer (no depth peeling necessary)
	 glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	 render_scene();

	 use_current_layer();

	 // init peeler
	 peeler.init_peeling();
	 int nr_fragments, nr_layers = 0;

	 // repeat until no more fragments have been drawn
	 do {
		 ++nr_layers;

	    // copy previous depth buffer to secondary depth buffer
	    peeler.copy_depth_buffer();

		 // then peel the next layer
		 glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		 peeler.begin_layer();
		 render_scene();
		 nr_fragments = peeler.end_layer();
		 use_current_layer();

	 }	while(nr_fragments > 0);
	 return nr_layers;
\endcode
	 */
class CGV_API gl_depth_peeler
{
protected:
	unsigned int query;
	texture depth_texture;
	float depth_bias;
	bool front_to_back;
	context* ctx_ptr;
	bool _invert_t;
public:
	/// a string that contains the last error, which is only set by the init method
	mutable std::string last_error;
	/// construct uninitialized depth peeler
	gl_depth_peeler(bool front_to_back = true, float _depth_bias = 0.001);
	/// destruct the depth peeler
	virtual ~gl_depth_peeler();
	/// set the sign for the vertical texture coordinate. In case of some ATI-cards this needs to be enabled to avoid flipping intermediate textures.
	void invert_t(bool enable = true);
	/// enable back to front mode
	void set_back_to_front();
	/// enable front to back mode
	void set_front_to_back();
	/// return whether the mode is front to back
	bool is_front_to_back() const;
	/// destruct the depth peeler
	virtual void destruct(context& ctx);
	/// the depth bias is used as epsilon for the test against the second depth buffer and is initialized to 0.0005f
	void set_depth_bias(float bias);
	/// return the current depth bias 
	float get_depth_bias() const;
	/// check whether the depth peeler has been initialized, i.e. the init method has been called successfully before
	bool is_initialized() const;
	/// checks for extensions and init depth peeler, return success
	virtual bool init(context& ctx);
	/// call this after the viewport has been set and before the first call to copy_depth_buffer or begin_layer
	virtual void init_frame(context& ctx);
	/// copies the current depth buffer to the second depth buffer, what is typically done before peeling a layer
	void copy_depth_buffer(context& ctx);
	/// start to extract the next layer. Within begin_layer and end_layer the following OpenGL features are used: texture generation functions, 2D texturing and the alpha Test
	void begin_layer(context& ctx, int tex_unit = -1);
	/// finish the layer and return the number of drawn fragments in the layer. All OpenGL settings are restored.
	unsigned int end_layer(context& ctx);
};


		}
	}
}

#include <cgv/config/lib_end.h>
