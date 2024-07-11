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

// C standard library
#include <stdio.h>

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

// Called as an in-between to actually handing over control flow to the service, for the
// purpose of being able to debug argument forwarding.
static inline int main_runner_proxy (int argc, char** argv)
{
	// Output debug info
	printf("Handing over control flow to the service '%s'\n", argv[0]);
	printf("- number of arguments: %i\n", argc-1);
	for (int i=1; i<argc; i++)
		printf("  %i: %s\n", i, argv[i]);
	printf("\n");

	// Start hosting the service
	return service_main(argc, argv);
}

int main (int argc, char** argv)
{
	// Validate command line
	if (argc < 2) {
		printf("No service library to host was provided in the command line!\n\n");
		return -1;
	}

	// Load the service library
	#ifdef _WIN32
		HMODULE hlib = LoadLibrary(argv[1]);
	#else
		void *hlib = dlopen(argv[1], RTLD_LAZY);
	#endif

	// Check if service could be loaded
	if (hlib == NULL) {
		printf("Unable to load service library \"%s\"!\n\n", argv[1]);
		return -1;
	}

	// Load the "main" symbol as our entry point
	#ifdef _WIN32
		service_main = (service)GetProcAddress(hlib, "main");
	#else
		service_main = dlsym(hlib, "main");
	#endif

	// Check if the entry point could be found
	if (service_main == NULL) {
		printf("Library \"%s\" does not export a function 'main'!\n\n", argv[1]);
		return -1;
	}

	// Forward control to the service
	return main_runner_proxy(argc-1, argv+1);
}
