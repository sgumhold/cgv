#include "opencl_opengl_interop.h"

opengl_opencl_interop::opengl_opencl_interop(const std::string& _name) : cgv::base::node(_name)
{
}

std::string opengl_opencl_interop::get_type_name() const
{
	return "opengl_opencl_interop";
}
	
bool opengl_opencl_interop::init(cgv::render::context& ctx)
{
	tex.create(ctx, cgv::render::TT_2D, 640, 640);
	ocl = new cgv::compute::opencl::opencl_base(ctx, CL_DEVICE_TYPE_GPU);
	if (!ocl->is_ready()) {
		std::cerr << "gpu device not ready" << std::endl;
		return false;
	}
	img = ocl->create_image_2d(tex);
	ocl->
	return true;
}

void opengl_opencl_interop::init_frame(cgv::render::context& ctx)
{

}

void opengl_opencl_interop::draw(cgv::render::context& ctx)
{
}

void opengl_opencl_interop::clear(cgv::render::context& ctx)
{
}

void opengl_opencl_interop::create_gui()
{
}


#include <iostream>
#include "hello_opencl.h"
#include "cgv/utils/file.h"
#include "cgv/math/vec.h"
#include <opencl_commons.h>
#include <opencl_code.h>
#include "cgv/defines/quote.h"

using namespace std;
using namespace cgv::compute::opencl;

hello_opencl::hello_opencl(const char *name)
{
	// orchestration of a simple opencl app
	setup_cl();
	load_and_compile_programs();
	prepare_data();
	create_and_run_kernel();
	gather_result();
}

bool hello_opencl::setup_cl(){
	cl_int err;
	
	// obtain the opencl platform
	std::vector<cl::Platform> platforms;
	err = cl::Platform::get(&platforms);
	if(checkError(err, "cl::Platform::get()", __LINE__) || platforms.empty())
		return false;
	cl::Platform platform = platforms.back();

	// obtain a list of available devices on the platform
	err = platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
	// optional: select and filter list of devices depending on capabilities
	// ... 
	// configure and create the cl context for the selected devices and platform
	cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties) platform(), 0 };
	cl_ctx = cl::Context(devices, properties, 0, 0, &err);

	if(checkError(err, "cl::Context()", __LINE__))
		return false;

	return true;
}
bool hello_opencl::load_and_compile_programs()
{
	opencl_code prog(cl_ctx);
	std::string path = QUOTE_SYMBOL_VALUE(INPUT_DIR);
	prog.add_path(path);
	prog.load_source_file("simple_test.cl");
	bool success = prog.build(devices);
	if(success)
		program = prog.get_program();
	return success;

// conventional approach without opencl_program.h
	/*
	cl_int err;
	
	// filename of source file
#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)
	std::string path = QUOTEME(INPUT_DIR);
	std::string file_name = "/simple_test.cl";
	file_name = path.append(file_name);
	cout << file_name << endl;

	// read the file and create sources object
	std::string source_str;
	cgv::utils::file::read(file_name, source_str);
	cl::Program::Sources sources;
	sources.push_back( std::make_pair(source_str.c_str(), source_str.length()) );
	
	// create program object
	program = program_ptr(new cl::Program(*cl_ctx, sources, &err));
	
	// compile for all devices
	err |= program->build(devices, 0, 0, 0);
	for(std::vector<cl::Device>::iterator it = devices.begin(); it != devices.end(); it++){
		std::string buildlog = program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*it);
		if(program->getBuildInfo<CL_PROGRAM_BUILD_STATUS>(*it) == CL_BUILD_PROGRAM_FAILURE)
			cerr << "Build failed" << buildlog << endl;
		else
			cout << "Build on device " << it->getInfo<CL_DEVICE_NAME>() << " succeeded" << endl;
	}
	return err == CL_SUCCESS;
	*/
}

bool hello_opencl::prepare_data()
{
	cl_int err;
	// create a simple long vector with increasing values
	cgv::math::vec<cl_float> v(PROBLEM_SIZE);
	for(unsigned i = 0; i < PROBLEM_SIZE; i++){
		v(i) = (cl_float) i;
	}
	size_t size = PROBLEM_SIZE * sizeof(cl_float);
	input_buffer = cl::Buffer(cl_ctx, CL_MEM_READ_WRITE, size, 0, &err);
	// copy the data to all devices (asynchronously)
	for(std::vector<cl::Device>::iterator it = devices.begin(); it != devices.end(); it++){
		cl::CommandQueue queue(cl_ctx, *it, CL_QUEUE_PROFILING_ENABLE, &err);
		err |= queue.enqueueWriteBuffer(input_buffer, false, 0, size, &v(0));
		queue.finish();
	}
	return err == CL_SUCCESS;
}

bool hello_opencl::create_and_run_kernel()
{
	cl_int err;
	// create kernel object from program by function name
	kernel = cl::Kernel(program, "square", &err);
	// set the kernel function's argument
	err |= kernel.setArg(0, input_buffer);
	// execute the kernel on the devices (asynchronously)
	for(std::vector<cl::Device>::iterator it = devices.begin(); it != devices.end(); it++){
		cl::CommandQueue queue(cl_ctx, *it, CL_QUEUE_PROFILING_ENABLE, &err);
		err |= queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(PROBLEM_SIZE), cl::NullRange);
		queue.finish();
	}
	return err == CL_SUCCESS;
}

bool hello_opencl::gather_result()
{
	cl_int err;
	size_t size = PROBLEM_SIZE * sizeof(cl_float);
	cgv::math::vec<cl_float> v(PROBLEM_SIZE);
	// copy the data from all devices to the host (synchronously) and display
	for(std::vector<cl::Device>::iterator it = devices.begin(); it != devices.end(); it++){
		cl::CommandQueue queue(cl_ctx, *it, CL_QUEUE_PROFILING_ENABLE, &err);
		queue.enqueueReadBuffer(input_buffer, true, 0, size, &v(0));
		cout << v << endl;
	}
	return err == CL_SUCCESS;
}

cgv::base::factory_registration_1<hello_opencl,const char*> ocltest("hello_opencl", 'O', "hello_opencl");