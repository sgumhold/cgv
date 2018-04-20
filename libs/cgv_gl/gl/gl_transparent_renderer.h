#pragma once

#include <cgv/render/texture.h>
#include <cgv/render/frame_buffer.h>

#include "gl_depth_peeler.h"

#include "lib_begin.h"

namespace cgv {
	namespace render {
			namespace gl {


/** OpenGL helper class to simplify transparent rendering. It is derived from a depth peeler whose functionality
    is used for the visibility sorting according to the approach described in the "order independent transparency" 
	paper of Cass Everit. Call the init method after your OpenGL
    context has been created. Transparent rendering is done such that the rendered objects are blended
	over the rendered scene in the current color and depth buffer. Rendering of transparent objects 
	should be done in the finish_frame method of a drawable. A sample implementation looks like this:
\code
	// in init
\endcode
*/
class CGV_API gl_transparent_renderer : public gl_depth_peeler
{
protected:
	/// depth peeler used for visibility sorting
	gl_depth_peeler peeler;
	// depth texture used as depth buffer used in offline rendering
	texture depth_buffer;
	// texture to store the peeled layers, which is used as color buffer in offline rendering and as texture during blending
	texture layer_tex;
	/// texture used as color buffer in front to back mode to blend together all transparent layers before blending the result over the current color buffer
	texture color_tex;
	// frame buffer object used for offline rendering of the layers
	frame_buffer fb;
	/// ensure that texture has the correct dimensions and is created
	void create_and_attach_texture(context& ctx, texture& tex, int w, int h, int i = -1);
	/// renders the given texture over the current viewport
	void blend_texture_over_viewport(context& ctx, texture& tex);
public:
	/// a string that contains the last error, which is only set by the init method
	mutable std::string last_error;
	/// signal called to render the transparent content for first layer without additional depth test
	cgv::signal::signal<context&> render_callback;
	/// signal called to render the transparent content for second and further layers with additional depth test against texture given in second argument
	cgv::signal::signal<context&,texture&> render_callback_2;
	/// construct uninitialized depth peeler
	gl_transparent_renderer(bool front_to_back = true, float _depth_bias = 0.001);
	/// checks for extensions and init depth peeler, return success
	bool init(context& ctx);
	/// configure frame buffer and textures
	void init_frame(context& ctx);
	/// perform transparent rendering by considering a maximum of the specified number of depth layers
	int render_transparent(context& ctx, int max_nr_layers, int tex_unit = -1);
	/// destruct the transparent renderer
	void destruct(context& ctx);
};


		}
	}
}

#include <cgv/config/lib_end.h>
