/* service_host: a minimal utility for launching service targets.
   It simply loads the shared library provided to it via the command line and calls its main() function.
   This is the extent of the protocol for CGV service targets for now - they just need to export their
   main() function (typically provided by the cgv_viewer_main static library) in C language mode, and the
   host will simply forward all its arguments after the shared library name to the service and hand over
   control flow to it. */


//////
//
// Includes
//

// C++ STL
#include <iostream>
#include <thread>
#include <future>

// Platform SDKs
#ifdef _WIN32
	#define NOMINMAX
	#include <windows.h>
#else
	#include <dlfcn.h>
#endif



//////
//
// Globals
//

// The function pointer type of the service entry point
typedef int(*service)(int, char**);

// The service entry point.
service service_main;



//////
//
// Functions
//

// The service runner thread entry point
static void service_runner (std::promise<int> &&retval, int argc, char** argv)
{
	// Output debug info
	std::cout << "Handing over control flow to the service '"<<argv[0]<<'\'' << std::endl
	          << "- number of arguments: "<<argc-1 << std::endl;
	for (int i=1; i<argc; i++)
		std::cout << "  "<<i<<": "<<argv[i] << std::endl;
	std::cout << std::endl;

	// Start hosting the service
	retval.set_value(service_main(argc, argv));
}

// Application entry point
int main (int argc, char** argv)
{
	// Validate command line
	if (argc < 2) {
		std::cout << "No service library to host was provided in the command line!" << std::endl<<std::endl;
		return -1;
	}

	// Load the service library
	#ifdef _WIN32
		HMODULE hlib = LoadLibrary(argv[1]);
	#else
		void *hlib = dlopen(argv[1], RTLD_LAZY);
	#endif

	// Check if service could be loaded
	if (hlib == nullptr) {
		std::cout << "Unable to load service library \""<<argv[1]<<"\"!" << std::endl<<std::endl;
		return -1;
	}

	// Load the "main" symbol as our entry point
	#ifdef _WIN32
		service_main = (service)GetProcAddress(hlib, "main");
	#else
		service_main = (service)dlsym(hlib, "main");
	#endif

	// Check if the entry point could be found
	if (service_main == nullptr) {
		std::cout << "Library \""<<argv[1]<<"\" does not export a function 'main'!" << std::endl<<std::endl;
		return -1;
	}

	// Forward control to the service
	// - obtain a future for the return value
	std::promise<int> retval_promise;
	std::future<int> retval = retval_promise.get_future();
	// - spawn runner thread executing the service main function
	std::thread service_thread(service_runner, std::move(retval_promise), argc-1, argv+1);

	// Wait for service to exit and report exit code
	service_thread.join();
	return retval.get();
}
