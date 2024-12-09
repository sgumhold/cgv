/// @file main.h
///
/// The import declaration for the Framework @ref main(int,char**) function. Can be used to link to the cgv_viewer_lib
/// in service builds.



//////
//
// Functions
//

/// The main entry point to the control loop of the CGV Framework.
extern "C"
#if (defined(_WIN32) || defined(WIN32))
	__declspec(dllexport)
#endif
int main (int argc, char** argv);
