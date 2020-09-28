#pragma once

#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv_gl/gl/image_drawable.h>
#include <cgv/media/image/image.h>

#include <libtorch.h>

class libtorch_viewer :
	public cgv::base::node,
	public cgv::gui::provider,
	public cgv::gui::event_handler,
    public cgv::render::gl::image_drawable
{
protected:
	/// Stores the location of the currently selected torch script file
	std::string neural_net_filename;
	/// Stores the neural network
	cgv::nn::tn_ptr neural_net;
	/// Stores the location of the currently selected image file
	std::string input_image_filename;
	/// Stores the input image
	cgv::media::image::image input_image;

	/// Engages the neural net on the input image
	void do_inference();
public:
	libtorch_viewer();

	std::string get_type_name() const { return "libtorch_viewer"; }

	//void clear(cgv::render::context& ctx);

	bool self_reflect(cgv::reflect::reflection_handler& rh);
	void stream_help(std::ostream& os) {}
	void stream_stats(std::ostream& os) {}

	bool handle(cgv::gui::event& e);
	void on_set(void* member_ptr);

	bool init(cgv::render::context& ctx);
	//void draw(cgv::render::context& ctx);

	void create_gui();
};