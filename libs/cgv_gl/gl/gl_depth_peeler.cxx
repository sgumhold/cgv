#include <cgv_gl/gl/gl.h>
#include "gl_depth_peeler.h"

namespace cgv {
	namespace render {
		namespace gl {

/// construct uninitialized depth peeler
gl_depth_peeler::gl_depth_peeler(bool f2b, float _depth_bias) 
	: depth_texture("[D]", TF_NEAREST, TF_NEAREST, TW_CLAMP, TW_CLAMP)
{
	query = -1;
	front_to_back = f2b;
	ctx_ptr = 0;
	depth_bias = _depth_bias;
	_invert_t = false;
}

/// set the sign for y
void gl_depth_peeler::invert_t(bool enable)
{
	_invert_t = enable;
}

/// enable back to front mode
void gl_depth_peeler::set_back_to_front()
{
	front_to_back = false;
}
/// enable front to back mode
void gl_depth_peeler::set_front_to_back()
{
	front_to_back = true;
}


/// return whether the mode is front to back
bool gl_depth_peeler::is_front_to_back() const
{
	return front_to_back;
}

/// destruct the depth peeler
gl_depth_peeler::~gl_depth_peeler()
{
	if (is_initialized())
		destruct(*ctx_ptr);
}

/// destruct the depth peeler
void gl_depth_peeler::destruct(context& ctx)
{
	depth_texture.destruct(ctx);
}

/// the depth bias is used as epsilon for the test against the second depth buffer and is initialized to 0.0005f
void gl_depth_peeler::set_depth_bias(float bias)
{
	depth_bias = (GLfloat) bias;
}
/// return the current depth bias 
float gl_depth_peeler::get_depth_bias() const
{
	return depth_bias;
}

/// check whether the depth peeler has been initialized, i.e. the init method has been called successfully before
bool gl_depth_peeler::is_initialized() const
{
	return ctx_ptr != 0;
}

/// checks for extensions and init depth peeler, return success
bool gl_depth_peeler::init(cgv::render::context& ctx) 
{
	if (!ensure_glew_initialized()) {
		last_error = "could not initialize glew";
		return false;
	}
	if (!GLEW_ARB_occlusion_query) {
		last_error = "missing ARB occlusion query extension";
		return false;
	}
	if (!GLEW_ARB_texture_rectangle) {
		last_error = "missing ARB texture rectangle extension";
		return false;
	}
	if (!GLEW_ARB_depth_texture) {
		last_error = "missing ARB depth texture extension";
		return false;
	}
	if (!GLEW_ARB_shadow) {
		last_error = "missing ARB shadow extension";
		return false;
	}
	ctx_ptr = &ctx;
	return true;		
}

void gl_depth_peeler::init_frame(context& ctx)
{
	// allocate memory for depth texture
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	if (!depth_texture.is_created() || vp[2] != depth_texture.get_width() || vp[3] != depth_texture.get_height()) {
		if (depth_texture.is_created())
			depth_texture.destruct(ctx);
		depth_texture.set_width(vp[2]);
		depth_texture.set_height(vp[3]);
		depth_texture.set_compare_mode(true);
		depth_texture.set_compare_function(CF_GREATER);
		depth_texture.create(ctx);
	}
}

void gl_depth_peeler::copy_depth_buffer(context& ctx)
{
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	depth_texture.replace_from_buffer(ctx,0,0,vp[0],vp[1],vp[2],vp[3],0);
}

void gl_depth_peeler::begin_layer(context& ctx, int tex_unit)
{
	glPushAttrib(GL_TEXTURE_BIT);
	glPushAttrib(GL_COLOR_BUFFER_BIT);

	// enable the depth texture (2nd depth buffer)
	depth_texture.enable(ctx, tex_unit);

	if (tex_unit >= 0) {
		if (!ensure_glew_initialized() || !glActiveTextureARB)
			tex_unit = -1;
		else
			glActiveTextureARB(GL_TEXTURE0_ARB+tex_unit);
	}

	// Use OpenGL- Extension GL_ARB_shadow and set the r texture coordinate to be tested
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
	// check whether the computed texture coordinate is greater or equal to the value stored in the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, front_to_back?GL_GEQUAL:GL_LEQUAL);
	// write the result of the test to the alpha channel (1 for r >= tex(s,t) and 0 for r < tex(s,t) )
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_ALPHA);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/*************************************************************
	  
	  the texture coordinates with which the 2nd depth buffer   
	  is accessed must correspond to in s and t to the normalized 
	  device but re-normalized from [-1,1] to [0,1]. The r texture
	  coordinate must correspond to the z-buffer value [0,1]. To
	  compute (s,t,r) 4D texture coordinates (s',t',r',q') are used
	  together with the q'-clip ( s=s'/q', t=t'/q', r=r'/q' ).
	  
	  We use the matrices
	  
	  mm ... modelview matrix
	  mp ... modelview projection
	  transAndScale ... re-normalization matrix

	  these are multiplied together to

	  M = transAndScale*mp*mm

	  to compute s' we compute the scalar product of the first row of M
	  with the current vertex (x,y,z,w). t/r/q are computed from the 
	  second/third/fourth rows of M. This is done by the automatic
	  texture coordinate generation. 
	
	********************************************************************/

	//  gather all matrices
	static GLdouble transAndScale[] = {
		0.5,0.0,0.0,0.0,
		0.0,0.5,0.0,0.0,
		0.0,0.0,0.5,0.0,
		0.5,0.5,0.5,1.0};
	GLdouble mp[16];
	glGetDoublev(GL_PROJECTION_MATRIX, &mp[0]);

	GLdouble mm[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, &mm[0]);
	
	// remember matrix mode and set to texture mode
	GLenum current_matrix_mode;
	glGetIntegerv(GL_MATRIX_MODE, (GLint*)&current_matrix_mode);
	glMatrixMode(GL_TEXTURE);

	// push a matrix onto the stack that is only used to compute M = transAndScale*mp*mm
	glPushMatrix();
	glLoadMatrixd(transAndScale);
	glMultMatrixd(mp);
	glMultMatrixd(mm);

	// read M into the variable mm
	glGetDoublev(GL_TEXTURE_MATRIX, mm);
	glPopMatrix();

	// ensure that now texture matrix is used
	glLoadIdentity();

	// restore matrix mode
	glMatrixMode(current_matrix_mode);

	// get the rows of m which are used to compute (s',t',r',q')
	GLdouble rowForS[] = {mm[0], mm[4], mm[8],  mm[12]};
	GLdouble rowForT[] = {
		_invert_t ? (mm[3] -mm[1])  : mm[1],
		_invert_t ? (mm[7] -mm[5])  : mm[5],
		_invert_t ? (mm[11]-mm[9])  : mm[9],
		_invert_t ? (mm[15]-mm[13]) : mm[13] };
	// subtract depth bias in the r-component to implement the depth-epsilon
	GLdouble rowForR[] = {
		front_to_back?(mm[2]-mm[3]*depth_bias):(mm[2]+mm[3]*depth_bias),
		front_to_back?(mm[6]-mm[7]*depth_bias):(mm[6]+mm[7]*depth_bias),
		front_to_back?(mm[10]-mm[11]*depth_bias):(mm[10]+mm[11]*depth_bias),
		front_to_back?(mm[14]-mm[15]*depth_bias):(mm[14]+mm[15]*depth_bias)
	};
	GLdouble rowForQ[] = {mm[3], mm[7], mm[11], mm[15]};

	// use automatic texture generation of OpenGL
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGendv(GL_S, GL_EYE_PLANE, rowForS);
	glEnable(GL_TEXTURE_GEN_S);

	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGendv(GL_T, GL_EYE_PLANE, rowForT);
	glEnable(GL_TEXTURE_GEN_T);

	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGendv(GL_R, GL_EYE_PLANE, rowForR);
	glEnable(GL_TEXTURE_GEN_R);

	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGendv(GL_Q, GL_EYE_PLANE, rowForQ);
	glEnable(GL_TEXTURE_GEN_Q);

	if (tex_unit >= 0)
		glActiveTextureARB(GL_TEXTURE0_ARB);
	
	// create occlusion query, that counts the number of rendered fragments
	glGenQueriesARB(1, &query);
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, query);

	// enable alpha test, where the alpha results from the depth comparison and discards fragments of previous layers
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.01f);
}

/// finish the layer and return the number of drawn fragments in the layer
unsigned int gl_depth_peeler::end_layer(context& ctx)
{
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);
	depth_texture.disable(ctx);
	glPopAttrib();			
	glPopAttrib();			
	// evaluation of occlusion query
	GLuint nr_drawn_fragments;
	glGetQueryObjectuivARB(query,GL_QUERY_RESULT_ARB,&nr_drawn_fragments);
	glDeleteQueriesARB(1,&query);
	query = -1;
	return (unsigned int) nr_drawn_fragments;
}

		}
	}
}