cmake_minimum_required(VERSION 2.6)

option(BUILD_SHARED_LIBS "Create shared libraries" TRUE)
option(BUILD_EXAMPLE_PLUGIN "Build the example plugin" TRUE)

# Load general build settings
include("cmake/buildSettings.cmake")

# First create the 3rd party libs
add_subdirectory(3rd)

# Add the tool subdirectory
add_subdirectory(tool)
# Add the basic cgv framework
add_subdirectory(cgv)
# Add the libraries subdirectory
add_subdirectory(libs)
# Add apps
add_subdirectory(apps)
# Add plugins
add_subdirectory(plugins)

# The framework depends on the tools and on 3rd libraries
add_dependencies(cgv tools 3rd)
# The tools depend on the 3rd libraries
add_dependencies(tools 3rd)
# The apps depend on the CGV framework and 3rd party libs
add_dependencies(apps cgv 3rd)
# The plugins depend on everything
add_dependencies(plugins cgv tools 3rd)
