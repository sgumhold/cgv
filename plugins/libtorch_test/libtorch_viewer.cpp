#include "libtorch_viewer.h"

#include <cgv/media/image/image.h>
#include <cgv/render/texture.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

#include <libtorch.h>

libtorch_viewer::libtorch_viewer() {

	set_name("Libtorch MNIST");
}

bool libtorch_viewer::self_reflect(cgv::reflect::reflection_handler& rh) {

	return false;
}

bool libtorch_viewer::handle(cgv::gui::event& e) {

	return false;
}

void libtorch_viewer::on_set(void* member_ptr) {

	if(member_ptr == &neural_net_filename) {
		bool success{ false };
		neural_net = cgv::nn::load_net_from_file(neural_net_filename, &success);
		assert(success);
		return;
	}

	if(member_ptr == &input_image_filename) {
		cgv::media::image::image img;
		img.read(input_image_filename);
		input_image = img;
		read_image(input_image_filename);

		post_redraw();
		return;
	}

	update_member(member_ptr);
	post_redraw();
}

bool libtorch_viewer::init(cgv::render::context& ctx) {

	return image_drawable::init(ctx);
}

void libtorch_viewer::create_gui() {
	add_decorator("TorchScript NN", "heading", "level=2");

	add_gui("TorchScript", neural_net_filename, "file_name", "title='Open TorchScript NN';filter='TorchScript Export (zip):*.zip|All Files:*.*'");
	add_gui("Input Image", input_image_filename, "file_name", "title='Open Input Image';filter='Image (png):*.png|All Files:*.*'");
	add_decorator("", "separator", "");
	connect_copy(add_button("Inference")->click, rebind(this, &libtorch_viewer::do_inference));
}

void libtorch_viewer::do_inference() {

	if(input_image.empty() || !neural_net) {
		std::cerr << "Input image or neural network is not specified!";
		return;
	}

	// The dimensions of the input image should be the exact dimensions required for the neural net (28x28).
	// Check image size and resize if necessary.
	cgv::media::image::image resized_image;

	unsigned w = input_image.get_width();
	unsigned h = input_image.get_height();

	if(w != 28 && h != 28) {
		resized_image.resize(28, 28, input_image);
		
		/*bool success = resized_image.write("out.png");
		if(!success)
			std::cout << "error writing image" << std::endl;*/

		input_image = resized_image;
	}

	unsigned num_components = input_image.get_nr_components();

	std::vector<float> pixels;
	unsigned char* pixels_ptr = input_image.get_ptr<unsigned char>();
	for(unsigned y = 0; y < 28; ++y) {
		for(unsigned x = 0; x < 28; ++x) {
			// Get linear index from 2d index
			unsigned idx = x + 28 * y;
			// Input image should be black and white, so we only need to get one component
			unsigned char value = pixels_ptr[num_components*idx];
			pixels.push_back(static_cast<float>(value));
		}
	}

	// Tensor size has 4 dimensions {batch_size, channels, height, width}
	// Example for a tensor with only 1s: torch::ones({ 1, 1, 28, 28 });
	at::Tensor input_tensor = torch::from_blob(pixels.data(), { 1, 1, 28, 28 }, torch::kFloat32);
	//std::cout << input_tensor << std::endl;

	// Define inputs for the network
	std::vector<torch::jit::IValue> inputs;
	inputs.push_back(input_tensor);

	try {
		at::Tensor output = neural_net->forward(inputs).toTensor();
		
		//std::cout << output << std::endl;

		std::vector<float> out = cgv::nn::to_vector<float>(output);
		
		float abs_min = std::numeric_limits<float>::max();
		int min_idx = -1;

		std::cout << "Result" << std::endl;

		for(unsigned i = 0; i < 10; ++i) {
			float val = abs(out[i]);
			
			if(val < abs_min) {
				abs_min = val;
				min_idx = i;
			}

			std::cout << i << ": " << out[i] << std::endl;
		}

		if(abs_min < 5.0f)
			std::cout << "Input is " << min_idx << ". Pretty sure." << std::endl;
		else
			std::cout << "Input is " << min_idx << ". Not so sure." << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << e.what();
	}

}

#include <cgv/base/register.h>
extern cgv::base::object_registration<libtorch_viewer> teqreg("torch reg");
