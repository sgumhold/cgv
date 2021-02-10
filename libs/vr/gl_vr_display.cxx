#include "gl_vr_display.h"

#include <GL/glew.h>

namespace vr {

/// construct
gl_vr_display::gl_vr_display(unsigned _width, unsigned _height, unsigned _nr_multi_samples)
	: width(_width), height(_height), nr_multi_samples(_nr_multi_samples)
{
	for (unsigned i = 0; i < 2; ++i) {
		multi_depth_buffer_id[i] = 0;
		multi_tex_id[i] = 0;
		multi_fbo_id[i] = 0;
		tex_id[i] = 0;
		fbo_id[i] = 0;
	}
}

/// declare virtual destructor
gl_vr_display::~gl_vr_display() 
{
}

/// return last error of vr_kit
const std::string& gl_vr_display::get_last_error() const 
{
	return last_error; 
}

/// allow to set a different size
void gl_vr_display::set_size(int new_width, int new_height)
{
	width = new_width;
	height = new_height;
}

/// return width in pixel of view
int gl_vr_display::get_width() const
{
	return width;
}
/// return height in pixel of view
int gl_vr_display::get_height() const
{
	return height;
}

/// check whether fbos have been initialized
bool gl_vr_display::fbos_initialized(EyeSelection es) const
{
	switch (es) {
	case ES_BOTH :
		return fbo_id[0] != 0 && fbo_id[1] != 0 && multi_fbo_id[0] != 0 && multi_fbo_id[1] != 0;
	case ES_LEFT:
		return fbo_id[0] != 0 && multi_fbo_id[0];
	case ES_RIGHT:
		return fbo_id[1] != 0 && multi_fbo_id[1] != 0;
	}
	return false;
}

/// destruct render targets and framebuffer objects in current opengl context
void gl_vr_display::destruct_fbos(EyeSelection es)
{
	for (unsigned i = 0; i < 2; ++i) {
		if (es == ES_LEFT && i == 1)
			continue;
		if (es == ES_RIGHT && i == 0)
			continue;
		if (multi_depth_buffer_id[i] != 0) {
			glDeleteRenderbuffers(1, &multi_depth_buffer_id[i]);
			multi_depth_buffer_id[i] = 0;
		}
		if (multi_tex_id[i] != 0) {
			glDeleteTextures(1, &multi_tex_id[i]);
			multi_tex_id[i] = 0;
		}
		if (multi_fbo_id[i] != 0) {
			glDeleteFramebuffers(1, &multi_fbo_id[i]);
			multi_fbo_id[i] = 0;
		}
		if (tex_id[i] != 0) {
			glDeleteTextures(1, &tex_id[i]);
			tex_id[i] = 0;
		}
		if (fbo_id[i] != 0) {
			glDeleteFramebuffers(1, &fbo_id[i]);
			fbo_id[i] = 0;
		}
	}
}

/// initialize render targets and framebuffer objects in current opengl context
bool gl_vr_display::init_fbos(EyeSelection es)
{
	for (unsigned i = 0; i < 2; ++i) {
		if (es == ES_LEFT && i == 1)
			continue;
		if (es == ES_RIGHT && i == 0)
			continue;

		glGenFramebuffers(1, &multi_fbo_id[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, multi_fbo_id[i]);

		glGenRenderbuffers(1, &multi_depth_buffer_id[i]);
		glBindRenderbuffer(GL_RENDERBUFFER, multi_depth_buffer_id[i]);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, nr_multi_samples, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, multi_depth_buffer_id[i]);

		glGenTextures(1, &multi_tex_id[i]);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multi_tex_id[i]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nr_multi_samples, GL_RGBA8, width, height, true);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multi_tex_id[i], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			destruct_fbos();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			last_error = "check status of multi framebuffer failed";
			return false;
		}

		glGenFramebuffers(1, &fbo_id[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_id[i]);

		glGenTextures(1, &tex_id[i]);
		glBindTexture(GL_TEXTURE_2D, tex_id[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id[i], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			destruct_fbos();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			last_error = "check status of framebuffer failed";
			return false;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}
/// bind texture of given eye to current texture unit
void gl_vr_display::bind_texture(int eye)
{
	glBindTexture(GL_TEXTURE_2D, tex_id[eye]);
}

/// enable the framebuffer object of given eye (0..left, 1..right) 
void gl_vr_display::enable_fbo(int eye)
{
	old_msaa = (int)glIsEnabled(GL_MULTISAMPLE);
	if (old_msaa == GL_FALSE)
		glEnable(GL_MULTISAMPLE);
	// Left Eye
	glBindFramebuffer(GL_FRAMEBUFFER, multi_fbo_id[eye]);
	glGetIntegerv(GL_VIEWPORT, vp);
	glViewport(0, 0, width, height);
}

/// initialize render targets and framebuffer objects in current opengl context
bool gl_vr_display::blit_fbo(int eye, int x, int y, int w, int h)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id[eye]);
	glBlitFramebuffer(0, 0, width, height, x, y, x+w, y+h,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	return true;
}

/// disable the framebuffer object of given eye
void gl_vr_display::disable_fbo(int eye)
{
	glViewport(vp[0], vp[1], vp[2], vp[3]);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, multi_fbo_id[eye]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id[eye]);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	if (old_msaa == GL_TRUE)
		glEnable(GL_MULTISAMPLE);
}

}