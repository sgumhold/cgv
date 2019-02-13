#include "gl_vr_display.h"

#include <GL/glew.h>

namespace vr {

/// construct
gl_vr_display::gl_vr_display(unsigned _width, unsigned _height, vr_driver* _driver, void* _handle, const std::string& _name, bool _ffb_support, bool _wireless)
	: width(_width), height(_height), vr_kit(_driver, _handle, _name, _ffb_support, _wireless)
{
	nr_multi_samples = 4;
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
	for (unsigned i = 0; i < 2; ++i) {
		if (multi_depth_buffer_id[i] != 0)
			glDeleteRenderbuffers(1, &multi_depth_buffer_id[i]);

		if (multi_tex_id[i] != 0)
			glDeleteTextures(1, &multi_tex_id[i]);
		if (multi_fbo_id[i] != 0)
			glDeleteFramebuffers(1, &multi_fbo_id[i]);
		if (tex_id[i] != 0)
			glDeleteTextures(1, &tex_id[i]);
		if (fbo_id[i] != 0)
			glDeleteFramebuffers(1, &fbo_id[i]);
	}
}

/// initialize render targets and framebuffer objects in current opengl context
bool gl_vr_display::init_fbos()
{
	for (unsigned i = 0; i < 2; ++i) {
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
			last_error = "check status of framebuffer failed";
			return false;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

/// enable the framebuffer object of given eye (0..left, 1..right) 
void gl_vr_display::enable_fbo(int eye)
{
	glEnable(GL_MULTISAMPLE);
	// Left Eye
	glBindFramebuffer(GL_FRAMEBUFFER, multi_fbo_id[eye]);
	glViewport(0, 0, width, height);
}

/// disable the framebuffer object of given eye
void gl_vr_display::disable_fbo(int eye)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_MULTISAMPLE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multi_fbo_id[eye]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id[eye]);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

}