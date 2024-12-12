#include "gl_transparent_renderer.h"
#include "gl_tools.h"
#include <cgv_gl/gl/gl.h>


namespace cgv {
	namespace render {
			namespace gl {


/// construct uninitialized depth peeler
gl_transparent_renderer::gl_transparent_renderer(bool front_to_back, float _depth_bias)
	: gl_depth_peeler(front_to_back, _depth_bias), 
	  layer_tex("uint8[R,G,B,A]",TF_NEAREST,TF_NEAREST), 
	  color_tex("uint8[R,G,B,A]",TF_NEAREST,TF_NEAREST), 
	  depth_buffer("[D]", TF_NEAREST, TF_NEAREST)
{
}

/// checks for extensions and init depth peeler, return success
bool gl_transparent_renderer::init(context& ctx)
{
	if (!gl_depth_peeler::init(ctx))
		return false;
	if (!GLEW_EXT_framebuffer_object) {
		last_error = "gl_transparent_renderer::init -> framebuffer objects not supported";
		return false;
	}
	return true;
}

/// ensure that texture has the correct dimensions and is created
void gl_transparent_renderer::create_and_attach_texture(context& ctx, texture& tex, int w, int h, int i)
{
	if (tex.is_created() && tex.get_width() == w && tex.get_height() == h)
		return;

	// depth buffer
	if (tex.is_created())
		tex.destruct(ctx);
	tex.set_width(w);
	tex.set_height(h);
	tex.create(ctx);

	fb.attach(ctx,tex,0,i);
}



/// configure frame buffer and textures
void gl_transparent_renderer::init_frame(context& ctx)
{
	// init the peeler for the frame
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	// query size of view
	int w = vp[2];
	int h = vp[3];

	// ensure that frame buffer and depth texture are created and have correct dimensions
	if (!fb.is_created() || fb.get_width() != w || fb.get_height() != h) {
		// frame buffer
		if (fb.is_created())
			fb.destruct(ctx);
		fb.create(ctx,w,h);
	}
	create_and_attach_texture(ctx, depth_buffer, w, h);
	create_and_attach_texture(ctx, layer_tex, w, h, 0);
	if (is_front_to_back())
		create_and_attach_texture(ctx, color_tex, w, h, 1);
	else
		color_tex.destruct(ctx);
	if (!fb.is_complete(ctx)) {
		std::cerr << "gl_transparent_renderer::init_frame -> framebuffer not complete" << std::endl;
		abort();
	}
	gl_depth_peeler::init_frame(ctx);
}

/// renders the given texture over the current viewport
void gl_transparent_renderer::blend_texture_over_viewport(context& ctx, texture& tex)
{
	// blend layer over global frame buffer
	glPushAttrib(GL_TEXTURE_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	tex.enable(ctx);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glEnable(GL_BLEND);
		if (front_to_back)
			glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE, GL_DST_ALPHA, GL_ZERO);
		else
			glBlendFuncSeparate(GL_ONE, GL_SRC_ALPHA, GL_DST_ALPHA, GL_ZERO);

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		mat4 inv_PV = inv(ctx.get_projection_matrix()*ctx.get_modelview_matrix());
		vec4 p_lb = inv_PV * vec4(-1,-1, 0, 1);
		vec4 p_rb = inv_PV * vec4( 1,-1, 0, 1);
		vec4 p_rt = inv_PV * vec4( 1, 1, 0, 1);
		vec4 p_lt = inv_PV * vec4(-1, 1, 0, 1);

		glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glVertex4fv(p_lb.data());
			glTexCoord2f(1,0);
			glVertex4fv(p_rb.data());
			glTexCoord2f(1,1);
			glVertex4fv(p_rt.data());
			glTexCoord2f(0,1);
			glVertex4fv(p_lt.data());
		glEnd();

	tex.disable(ctx);
	glPopAttrib();
}

/// perform transparent rendering by considering a maximum of the specified number of depth layers
int gl_transparent_renderer::render_transparent(context& ctx, int max_nr_layers, int tex_unit)
{
	glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_SCISSOR_BIT);
	glDisable(GL_SCISSOR_TEST);

	int nr_fragments, nr_layers = 0;
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	GLint vp_off[4] = { 0,0, vp[2], vp[3] };
	if (front_to_back) {
		glDepthFunc(GL_LESS);
		glClearDepth(1);
		// use current depth buffer to initialize depth buffer of fbo
		depth_buffer.replace_from_buffer(ctx,0,0,vp[0],vp[1],vp[2],vp[3],0);
		// render first layer to layer texture
		glClearColor(0, 0, 0, 1);
		fb.enable(ctx, 0);
			// render without a shadow test
			glClear(GL_COLOR_BUFFER_BIT);
			render_callback(ctx);
		fb.disable(ctx);

//		layer_tex.write_to_file(ctx, "S:/temp/debug/layer_ 0.bmp");
//		depth_buffer.write_to_file(ctx, "S:/temp/debug/depth_buffer_ 0.bmp", -1, 2.0f);

		// iterate layers
		do {
			fb.enable(ctx,1);
				if (nr_layers == 0)
					glClear(GL_COLOR_BUFFER_BIT);
				blend_texture_over_viewport(ctx, layer_tex);
			fb.disable(ctx);
//			color_tex.write_to_file(ctx, "S:/temp/debug/color_" + cgv::utils::to_string(nr_layers, 2) + ".bmp");

			// check for termination
			if (++nr_layers > max_nr_layers)
				break;

			// enable frame buffer object
			fb.enable(ctx, 0);
				copy_depth_buffer(ctx);
			fb.disable(ctx);
			depth_buffer.replace_from_buffer(ctx,0,0,vp[0],vp[1],vp[2],vp[3],0);

			fb.enable(ctx, 0);				
				// draw and use first layer (no depth peeling necessary)
				glClear(GL_COLOR_BUFFER_BIT);
				// create occlusion query, that counts the number of rendered fragments
				glGenQueriesARB(1, &query);
				glBeginQueryARB(GL_SAMPLES_PASSED_ARB, query);

				render_callback_2(ctx, depth_texture);
				glEndQueryARB(GL_SAMPLES_PASSED_ARB);
				// evaluation of occlusion query
				GLuint nr_drawn_fragments;
				glGetQueryObjectuivARB(query, GL_QUERY_RESULT_ARB, &nr_drawn_fragments);
				glDeleteQueriesARB(1, &query);
				query = -1;
				nr_fragments = nr_drawn_fragments;

			fb.disable(ctx);
//			layer_tex.write_to_file(ctx, "S:/temp/debug/layer_"+cgv::utils::to_string(nr_layers,2)+".bmp");
//			depth_buffer.write_to_file(ctx, "S:/temp/debug/depth_buffer_" + cgv::utils::to_string(nr_layers,2) + ".bmp", -1, 2.0f);
//			std::cout << "layer " << nr_layers << " : " << nr_fragments << std::endl;
		} while (nr_fragments > 0);
		
		front_to_back = false;
		blend_texture_over_viewport(ctx, color_tex);
		front_to_back = true;
		
	}
	else {
		glDepthFunc(GL_GREATER);
		glClearDepth(0);
		// init shadow depth map to current depth buffer
		copy_depth_buffer(ctx);
		fb.enable(ctx, 0);
			// render with shadow test
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			begin_layer(ctx);
				render_callback(ctx);
			end_layer(ctx);
		fb.disable(ctx);

		// iterate layers
		do {
			blend_texture_over_viewport(ctx, layer_tex);

			// check for termination
			if (++nr_layers > max_nr_layers)
				break;

			// enable frame buffer object
			fb.enable(ctx, 0);
				copy_depth_buffer(ctx);
				// draw and use first layer (no depth peeling necessary)
				glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
				begin_layer(ctx);
					render_callback(ctx);
				nr_fragments = end_layer(ctx);
			fb.disable(ctx);
		} while (nr_fragments > 0);
	}

	glPopAttrib();
	return nr_layers;
}

/// destruct the transparent renderer
void gl_transparent_renderer::destruct(context& ctx)
{
	gl_depth_peeler::destruct(ctx);
	fb.destruct(ctx);
	layer_tex.destruct(ctx);
	color_tex.destruct(ctx);
	depth_buffer.destruct(ctx);
}

		}
	}
}