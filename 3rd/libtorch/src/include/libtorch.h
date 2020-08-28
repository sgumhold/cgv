#pragma once

#include <memory>
#include <string>

#include <torch/script.h>

#include "lib_begin.h"

using TNPtr = std::shared_ptr<torch::jit::script::Module>;

TNPtr CGV_API load_net_from_file(std::string const& filepath, bool* const success = nullptr);
