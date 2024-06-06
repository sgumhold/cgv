/* server_host: a minimal utility for testing server targets.
   It simply loads the shared library provided to it via the command line and calls its main() function.
   This is the extent of the protocol for CGV server targets for now - they just need to export their
   main() function (typically provided by the cgv_viewer_main static library) in C language mode, and the
   host will simply forward all its arguments after the shared library name to the server and hand over
   control flow to it. */

#include <stdio.h>

#ifdef _WIN32
	#define NO_MIN_MAX
	#include <windows.h>
#else
	#include <dlfcn.h>
#endif

// The function pointer type of the server entry point
typedef int(*server_entry_func_t)(int, char**);

// The server entry point.
server_entry_func_t server_main;

// Called as an in-between to actually handing over control flow to the server, for the
// purpose of being able to debug argument forwarding.
int main_runner_proxy (int argc, char** argv)
{
	// output debug info
	printf("Handing over control flow to the server '%s'\n", argv[0]);
	printf("- number of arguments: %i\n", argc-1);
	for (int i=1; i<argc; i++)
		printf("  %i: %s\n", i, argv[i]);
	printf("\n");

	// start hosting the server
	return server_main(argc, argv);
}

int main (int argc, char** argv)
{
	// validate command line
	if (argc < 2) {
		printf("No server to host was provided in the command line!\n\n");
		return -1;
	}

	// load the server library
	#ifdef _WIN32
		HMODULE hlib = LoadLibrary(argv[1]);
	#else
		void *hlib = dlopen(argv[1]);
	#endif

	// check if server could be loaded
	if (hlib == NULL) {
		printf("Unable to load server \"%s\"!\n\n", argv[1]);
		return -1;
	}

	// load the "main" symbol as our entry point
	#ifdef _WIN32
		server_main = (server_entry_func_t)GetProcAddress(hlib, "main");
	#else
		server_main = dlsym(hlib, "main");
	#endif

	// check if the entry point could be found
	if (server_main == NULL) {
		printf("Library \"%s\" does not export a function 'main'!\n\n", argv[1]);
		return -1;
	}

	// forward to server
	return main_runner_proxy(argc-1, argv+1);
}
