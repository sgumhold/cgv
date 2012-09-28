// ppp defines the following variables
// - SYSTEM ... one of "windows" or "linux"
//
// the external ppp file make.ppp defines the following variables
//
// - INPUT_DIR  ... directory containing the currently processed project file
// - INPUT_PATH ... full path to the currently processed project file
// - INPUT_FILE ... name of the project file with extension
//
// - CGV_DIR     ... from the obligatory environment variable CGV_DIR
// - CGV_INSTALL ... from environment variable CGV_INSTALL or if not defined from CGV_DIR
// - CGV_BUILD   ... from environment variable CGV_BUILD or if not defined from CGV_DIR/build
//
// - CGV_COMPILER ... from environment variable CGV_COMPILER or if not defined the defaults
//                    are "vs9" for "windows" and "gcc" for "linux"
// - CGV_COMPILER_VERSION ... 8 or 9 for the visual studio compilers "vs8" and "vs9"
//
@exclude<cgv/config/make.ppp>


//====================================================================
//    the following are obligatory defines
//====================================================================

//globally unique identifier of project
@define(projectGUID="E4E3902D-4700-4EE9-A64E-6BD4DD762E4D")

//====================================================================
// now the optional defines follow, all these are preset in make.ppp
//====================================================================

//projectType can take the values
// - "library" ... library that can be compiled to static and shared version
// - "static_library" ... library that can be compiled only in static version
// - "shared_library" ... library that can be compiled only in shared version
// - "plugin" ... same as library but used as plugin to an application
// - "shared_plugin" ... same as shared_library but used as plugin to an application
// - "application" ... executable that can take arguments of the form "plugin:my_plugin"
// - "tool" ... executable used during the build process like ppp
// - "test" ... plugin to the tester tool that provides boolean functions to test code
//if not specified, the projectType defaults to "plugin"
@define(projectType="library")

//name of project defaults to INPUT_NAME without extension
@define(projectName="my_project")

//list of individual source files
@define(sourceFiles=[INPUT_DIR."my_source.cxx", INPUT_DIR."my_resource.png"])

//Specify a list of files that should be excluded from the project.
//The files must be specified by the same path as in the sourceDirs. 
//If you specify INPUT_PATH, the "*.pj"-file is excluded from the project.
//
//If a file is listed in sourceFiles AND excludeSourceFiles it is INCLUDED.
@define(excludeSourceFiles=[INPUT_PATH])

//list of directories that are scanned recursively for source files, 
//defaults to [] if sourceFiles are defined or to [INPUT_DIR] if 
//sourceFiles is not defined
@define(sourceDirs=[INPUT_DIR."/my_subdir"])

//specify subdirs in the source directories that should be excluded
@define(excludeSourceDirs=[])

// define additional directories, in which project files are located. 
// By default the directories CGV_DIR/cgv, CGV_DIR/apps and CGV_DIR/tool
// are project directories.
@define(addProjectDirs=[CGV_DIR."/3rd", CGV_DIR."/plugins", CGV_DIR."/libs", env::CGV_SUPPORT_DIR."/plugins"])

//Define projects on which this project depends. All projects that are
//handled by the cgv-framework need to be added as addProjectDeps and
//not as addDependencies even if the resulting libraries are installed
//on the system. Please add the following libraries which are installed
//under linux to addProjectDeps:
//- zlib
//- libjpeg
//- libpng
//- libtiff
//- fltk
//ppp will automatically classify them as dependencies under linux and
//as dependent projects under windows.
//
//If a plugin depends on an application, this application 
//is used as executable for running and debugging the plugin.
//
//A "test" project is always dependent on the "tester" tool.
//Default dependencies are the tools used during the build process.
//The projects in this list are added to the solution recursively.
@define(addProjectDeps=["cgv","cgv_viewer","fltk"])

// add additional command line arguments. The most common usage of this
// is to add a default config file as in the given example
@define(addCommandLineArguments=['config:"'.INPUT_DIR.'/my_config.def"'])

// working directory in which the resulting program should be started
// this defaults to the platform specific default directories
@define(workingDirectory=INPUT_DIR)

//additional include paths, which are appended to the default paths
//that include CGV_DIR and the list of paths defined in the INCLUDE
//environment variable that are automatically recognized by the compiler.
//To export include paths to projects that depend on this project, specify
//the include path in a list of two strings with the second entry equal to "all"
@define(addIncDirs=[INPUT_DIR."/my_include_subdir", [env::CGV_SUPPORT."/libs/capture", "all"] ])

//additional preprocessor definitions for all configurations. By default the
//following symbols are defined:
// - INPUT_DIR ... is defined to the input directory of the project
// - CGV_DIR ... is defined to the cgv directory active during compilation
//Both predefined paths come without enclosing double quotes. You can use the
//macro QUOTE_SYMBOL_VALUE defined in <cgv/defines/quote.h> to enclose the directories
//in double quotes.
// 
//To export defines to projects that depend on this project, specify
//the define in a list of two strings with the second entry equal to "all"
@define(addDefines=["NORMAL_DEFINE", ["EXPORTED_DEFINE","all"]])

//additional preprocessor definitions for shared configurations
//To export shared defines to projects that depend on this project, specify
//the shared define in a list of two strings with the second entry equal to "all"
@define(addSharedDefines=["CGV_FORCE_EXPORT", ["FL_SHARED", "all"] ])

//additional preprocessor definitions for static configurations
//To export static defines to projects that depend on this project, specify
//the static define in a list of two strings with the second entry equal to "all"
@define(addStaticDefines=[])

//additional library paths. The default library paths include
//CGV_DIR/lib and the path list defined in the environment 
//variable LIBRARIES that is automatically recognized by the
//linker. 
//To export lib dirs to projects that depend on this project, specify
//the lib dir in a list of two strings with the second entry equal to "all"
@define(addLibDirs=["regular_lib_dir", ["exported_lib_dir", "all"] ])

//external libraries on which the project depends. If one project
//depends on another, it is sufficient to add the project to the
//addProjectDeps list without entry in addDendencies. As library names
//are platform dependent, the platform and configuration specific
//library names of the following libraries are tabulated and translated
//from the given name correctly:
// - "opengl"
// - "glu"
// - "glew"
//In order to extend the translation mapping use the syntax
@define(addMapDeps = [ ["glut", ["freeglut_static","freeglut_static_debug","freeglut_shared","freeglut_shared_debug"] ],
                       ["ann", ["annf_s","annf_ds","annf", "annf_d"] ]
                     ])
//
//or if for example the translation mapping of "ann" should be inherited by projects that depend on "ann"
@define(addMapDeps = [ ["glut", ["freeglut_static","freeglut_static_debug","freeglut_shared","freeglut_shared_debug"] ],
                       [ ["ann", ["annf_s","annf_ds","annf", "annf_d"] ], "all" ] 
                     ])

// In order to convert a project dependency that includes the project into the workspace into a 
// dependency that only uses the compiled versions of the projects, use the following
@define(addExcludeProjectDeps = ["fltk", "pthread"]
// this is used by default under linux for the libraries fltk, pthread, libjpeg, libpng, zlib
// and libtiff
//
//The library paths to the corresponding libs need to be present
//in the environment variable LIBRARIES
//To export dependencies to projects that depend on this project, specify
//the dependency in a list of two strings with the second entry equal to 
// - "static" ... dependency exported only to static configurations 
// - "shared" ... dependency exported only to shared configurations 
// - "all"    ... dependency exported to all configurations
@define(addDependencies=["opengl","glu","glew",["Vfw32", "static"] ])

//set the name of the definition file, in which the symbols are 
//defined that should be exported in a dll which has been implemented
//with extern "C" declarations. An example usage is in the project
//file for libtiff.
@define(defFile=INPUT_PATH."/my_definition_file.def")

//Add the entries of the given list as rule files to the project.
//To export a rule file to projects that depend on this project, specify
//the rule file in a list of two strings with the second entry equal to "all"
@define(addRuleFiles=[])

//Add new rules that specify how to handle extensions during makefile
//generation. Each rule is defined by a MAP with the following entries:
// - extensions : LIST of STRING  ... list of extensions covered by this rule
// - folder : STRING              ... project folder into which the corresponding 
//                                    files should be sorted
// - tool : optional STRING       ... project name of tool used to transform the file
// - rules : optional LIST of MAP ... list of rules to be applied to the source file in order 
//                                    to generate new source files. Each list entry of type
//                                    MAP can have the following entries:
//   - keep_extension : optional BOOL ... whether to keep extension of source file name
//   - suffix : optional STRING ... suffix added to the source file name before adding the extension
//   - extension : STRING ... extension of file generated with the tool from the source file
//   - path : STRING ... path to the directory where the generated file can be found
@define(addRules=[])