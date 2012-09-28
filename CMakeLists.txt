cmake_minimum_required(VERSION 2.6)

# Set CMake policy 17 to prefer local find scripts
cmake_policy(SET CMP0017 OLD)

option(BUILD_SHARED_LIBS "Create shared libraries" FALSE)

# set(BUILD_SHARED_LIBS TRUE)

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

# The framework depends on the tools and on 3rd libraries
add_dependencies(cgv tools)
add_dependencies(cgv 3rd)
# The tools depend on the 3rd libraries
add_dependencies(tools 3rd)
# The apps depend on the CGV framework
add_dependencies(apps cgv)

# FIXME: delete this line
add_subdirectory(plugins/cg_fltk)
add_subdirectory(plugins/crg_stereo_view)
add_subdirectory(plugins/crg_grid)

