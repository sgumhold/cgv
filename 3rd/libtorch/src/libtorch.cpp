#include "include/libtorch.h"

#include <iostream>

TNPtr CGV_API load_net_from_file(std::string const& filepath, bool* const success) {
	// TODO: Limit the possible filetype to only TorchScript?

	try
	{
		TNPtr ret = std::make_shared<TNPtr::element_type>();
		*ret = torch::jit::load(filepath);
		if (nullptr != success) *success = true;
		return ret;
	}
	catch (const c10::Error & e)
	{
		std::cerr << "Could not load the model!" << std::endl;
		if (nullptr != success) *success = false;
		return {};
	}
}