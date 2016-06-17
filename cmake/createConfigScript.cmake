get_filename_component(BASE_CM_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)
include("${BASE_CM_PATH}/CMakePackageConfigHelpers.cmake")


macro(cgv_find_package)

	set(ARGS "${ARGN}")
	list(GET ARGS 0 DEP_NAME)
	set(${PROJECT_NAME}_DEP_NAMES ${${PROJECT_NAME}_DEP_NAMES} ${DEP_NAME})

	set(DEP_DESCR ${PROJECT_NAME}_FIND_DEP_${DEP_NAME})

	set(${DEP_DESCR} "${ARGN}")
	set(${PROJECT_NAME}_FIND_DEPS ${${PROJECT_NAME}_FIND_DEPS} ${DEP_DESCR})

	if (NOT DEFINED ${DEP_NAME}_IS_INTERNAL)
		set(${DEP_NAME}_IS_INTERNAL TRUE)
	endif()
	find_package(${ARGN})	
	
endmacro()

macro(cgv_find_external_package)
	set(ARGS "${ARGN}")
	list(GET ARGS 0 DEP_NAME)
	set(${DEP_NAME}_IS_INTERNAL FALSE)
	cgv_find_package(${ARGN})
endmacro()



macro(cgv_write_find_file target)

	set(MACROS_BASE "${BASE_CM_PATH}")

	# Parse the argument list
	parse_arguments(FIND "ADDITIONAL_INCLUDES;SHARED_DEFINITIONS;STATIC_DEFINITIONS;DEFINITIONS;CMAKE_SRC_BASE;CMAKE_SRC_FILES" "CMAKE_REPLACE;IS_MODULE" ${ARGN})
		
	set(FIND_INIT "")

	if (${PROJECT_NAME}_FIND_DEPS)
		foreach(dep ${${PROJECT_NAME}_FIND_DEPS})
			string(REPLACE ";" " " DEP_SETTINGS "${${dep}}")
			set(FIND_INIT "${FIND_INIT}find_package(${DEP_SETTINGS})\n")
		endforeach()
	endif()

	string(REPLACE ";" " " DEP_NAMES "${${PROJECT_NAME}_DEP_NAMES}")
	set(FIND_INIT "${FIND_INIT}set(FIND_NAMES ${DEP_NAMES})\n")
	set(TARGET "${target}")

	get_target_property(TARGET_TYPE ${target} TYPE)

	if (TARGET_TYPE STREQUAL "MODULE_LIBRARY")
		set(TARGET_IS_MODULE 1)
	endif()	


	get_target_property(HEADER_LOCAL_PATH ${target} HEADER_LOCAL_PATH)
	get_target_property(HEADER_LOCAL_NAME ${target} HEADER_LOCAL_NAME)

	if (NOT HEADER_LOCAL_NAME)
		set(HEADER_LOCAL_NAME ${target})
	endif()

	set(LIB_PATH "${INSTALL_BASE}/${INSTALL_LIB_PATH}")
	set(BIN_PATH "${INSTALL_BASE}/${INSTALL_BIN_PATH}")
	set(HEADER_PATH "${INSTALL_BASE}/${INSTALL_HEADER_PATH}")

	set(BUILD_LIB_PATH "${BUILD_BASE}/${INSTALL_LIB_PATH}")
	set(BUILD_BIN_PATH "${BUILD_BASE}/${INSTALL_BIN_PATH}")
	set(SOURCE_HEADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
	
	# Check if the SOURCE_HEADER_PATH contains CGV_DIR
	if (NOT "$ENV{CGV_DIR}" STREQUAL "")
		string(REPLACE "\\" "/" CGV_DIR_UNIX "$ENV{CGV_DIR}")
		string(REPLACE "${CGV_DIR_UNIX}" "" HEADER_STRIPPED "${SOURCE_HEADER_PATH}")
		if (NOT HEADER_STRIPPED STREQUAL SOURCE_HEADER_PATH)
			string(REPLACE "//" "/" SOURCE_HEADER_PATH "\$ENV{CGV_DIR}/${HEADER_STRIPPED}")
		endif()
	endif()
	
	set(ADDITIONAL_LIST_INSTALL "")
	set(ADDITIONAL_LIST_SOURCE "")
	if (FIND_ADDITIONAL_INCLUDES)
		foreach(inc_add ${FIND_ADDITIONAL_INCLUDES})
			set(ADDITIONAL_LIST_INSTALL ${ADDITIONAL_LIST_INSTALL} "\${PACKAGE_PREFIX_DIR}/${inc_add}")
			set(ADDITIONAL_LIST_SOURCE ${ADDITIONAL_LIST_SOURCE} "${SOURCE_HEADER_PATH}/${inc_add}")
		endforeach()
	endif()
	
	set(STATIC_DEFINITIONS ${FIND_STATIC_DEFINITIONS} ${${target}_STATIC_DEFINITIONS})
	set(SHARED_DEFINITIONS ${FIND_SHARED_DEFINITIONS} ${${target}_SHARED_DEFINITIONS})
	set(COMMON_DEFINITIONS ${FIND_COMMON_DEFINITIONS} ${${target}_COMMON_DEFINITIONS})
	

	get_target_property(OUTPUT_NAME ${target} OUTPUT_NAME)
	if (NOT OUTPUT_NAME)
		set(OUTPUT_NAME ${target})
	endif()

	set(MACROS_NAME "${CMAKE_CURRENT_SOURCE_DIR}/${target}Macros.cmake")
	if (EXISTS "${MACROS_NAME}")
		file(READ "${MACROS_NAME}" MACRO_DEFINITIONS)
	endif()


	set(INPUT_FILE "PkgLibConfig.cmake.in")
	if (TARGET_TYPE STREQUAL "EXECUTABLE")
		set(INPUT_FILE "PkgBinConfig.cmake.in")
	endif()

	configure_package_config_file("${MACROS_BASE}/${INPUT_FILE}"
		"${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_CMAKE_PATH}/${target}Config.cmake"
		INSTALL_DESTINATION "${INSTALL_BASE}/${INSTALL_CMAKE_PATH}"
		PATH_VARS LIB_PATH BIN_PATH HEADER_PATH BUILD_LIB_PATH BUILD_BIN_PATH)

	configure_file("${MACROS_BASE}/FindPkg.cmake.in"
		"${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_CMAKE_PATH}/Find${target}.cmake" @ONLY)

	install(FILES "${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_CMAKE_PATH}/${target}Config.cmake"
				  "${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_CMAKE_PATH}/Find${target}.cmake"
			DESTINATION "${INSTALL_BASE}/${INSTALL_CMAKE_PATH}")

	# Process additional CMake files
	if (FIND_CMAKE_SRC_FILES)
		if (NOT FIND_CMAKE_SRC_BASE)
			set(CMAKE_SRC_BASE ${CMAKE_CURRENT_SOURCE_DIR})
		endif()
		
		set(ADDITIONAL_FILES "")
		foreach(f ${FIND_CMAKE_SRC_FILES})
			if (IS_ABSOLUTE "${f}")
				file(RELATIVE_PATH f "${FIND_CMAKE_SRC_BASE}" "${f}")
				if (IS_ABSOLUTE "${f}")
					message(FATAL_ERROR "The file ${f} that was given to cgv_write_find_file as a script dependency must be in a subdirectory of the project")
				endif()
			endif()
			
			if (NOT EXISTS "${FIND_CMAKE_SRC_BASE}/${f}")
				message(FATAL_ERROR "The find script for ${target} requires the file ${f} which was not found!")
			endif()
			
			# Copy the files to the cmake files location
			if (FIND_CMAKE_REPLACE)
				set(COPY_MODE "@ONLY")
			else()
				set(COPY_MODE "COPYONLY")
			endif()
						
			configure_file("${FIND_CMAKE_SRC_BASE}/${f}"
				"${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_CMAKE_PATH}/${f}"
				${COPY_MODE})
			get_filename_component(F_PATH "${f}" PATH)
			install(FILES "${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_CMAKE_PATH}/${f}"
					DESTINATION "${INSTALL_BASE}/${INSTALL_CMAKE_PATH}/${F_PATH}")
		endforeach()
	endif()
endmacro()
