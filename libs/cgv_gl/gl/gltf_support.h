#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv/media/mesh/simple_mesh.h>
#include "render_info.h"
#include "fx/gltf.h"

#include "lib_begin.h"

namespace cgv {
	namespace render {

/// construct render info from gltf document
extern CGV_API bool build_render_info(const std::string& file_name, const fx::gltf::Document& doc,
	cgv::render::context& ctx, cgv::render::render_info& R);
/// extract bounding box and vertex count from gltf document
extern CGV_API void extract_additional_information(const fx::gltf::Document& doc,
	box3& box,
	size_t& vertex_count);
//! extract simple mesh form gltf document
/*! extraction can be restricted to a mesh and within the mesh to a primitive by specifying the 
    corresponsing indices as parameters to this function. The number of meshes can be extracted
	from the gltf document via doc.meshes.size() and the number of primitives for a given mesh
	with doc.meshes[mesh_index].primitives.size().
	CAREFULL: works only in cases were all meshes and primitives have same vertex attribute 
	configuration.*/
extern CGV_API void extract_mesh(const std::string& file_name, const fx::gltf::Document& doc,
	cgv::media::mesh::simple_mesh<float>& mesh, int mesh_index = -1, int primitive_index = -1);
/// NOT IMPLEMENTED YET
extern CGV_API bool write_gltf(const std::string& file_name, const cgv::render::render_info& R);

	}
}

#include <cgv/config/lib_end.h>