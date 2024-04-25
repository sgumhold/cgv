#include <cgv/utils/file.h>
#include <cgv/media/image/image_reader.h>
#include <cgv_gl/gl/gl.h>
#include "gltf_support.h"

namespace cgv {
	namespace render {

cgv::render::TextureFilter map_gl_to_tex_filter(GLenum tf)
{
	switch (tf) {
	case GL_NEAREST: return cgv::render::TF_NEAREST;
	case GL_LINEAR: return cgv::render::TF_LINEAR;
	case GL_NEAREST_MIPMAP_NEAREST: return cgv::render::TF_NEAREST_MIPMAP_NEAREST;
	case GL_LINEAR_MIPMAP_NEAREST: return cgv::render::TF_LINEAR_MIPMAP_NEAREST;
	case GL_NEAREST_MIPMAP_LINEAR: return cgv::render::TF_NEAREST_MIPMAP_LINEAR;
	case GL_LINEAR_MIPMAP_LINEAR: return cgv::render::TF_LINEAR_MIPMAP_LINEAR;
	case GL_TEXTURE_MAX_ANISOTROPY_EXT: return cgv::render::TF_ANISOTROP;
	}
	return cgv::render::TF_LAST;
}

cgv::render::TextureWrap map_gl_to_tex_wrap(GLenum tw)
{
	switch (tw) {
	case GL_REPEAT: return cgv::render::TW_REPEAT;
	case GL_CLAMP: return cgv::render::TW_CLAMP;
	case GL_CLAMP_TO_EDGE: return cgv::render::TW_CLAMP_TO_EDGE;
	case GL_CLAMP_TO_BORDER: return cgv::render::TW_CLAMP_TO_BORDER;
	case GL_MIRROR_CLAMP_EXT: return cgv::render::TW_MIRROR_CLAMP;
	case GL_MIRROR_CLAMP_TO_EDGE_EXT: return cgv::render::TW_MIRROR_CLAMP_TO_EDGE;
	case GL_MIRROR_CLAMP_TO_BORDER_EXT: return cgv::render::TW_MIRROR_CLAMP_TO_BORDER;
	case GL_MIRRORED_REPEAT: return cgv::render::TW_MIRRORED_REPEAT;
	}
	return cgv::render::TW_LAST;
}

cgv::render::PrimitiveType map_gl_to_primitive_type(GLenum pt)
{
	switch (pt) {
	case GL_POINTS: return cgv::render::PT_POINTS;
	case GL_LINES: return cgv::render::PT_LINES;
	case GL_LINES_ADJACENCY: return cgv::render::PT_LINES_ADJACENCY;
	case GL_LINE_STRIP: return cgv::render::PT_LINE_STRIP;
	case GL_LINE_STRIP_ADJACENCY: return cgv::render::PT_LINE_STRIP_ADJACENCY;
	case GL_LINE_LOOP: return cgv::render::PT_LINE_LOOP;
	case GL_TRIANGLES: return cgv::render::PT_TRIANGLES;
	case GL_TRIANGLES_ADJACENCY: return cgv::render::PT_TRIANGLES_ADJACENCY;
	case GL_TRIANGLE_STRIP: return cgv::render::PT_TRIANGLE_STRIP;
	case GL_TRIANGLE_STRIP_ADJACENCY: return cgv::render::PT_TRIANGLE_STRIP_ADJACENCY;
	case GL_TRIANGLE_FAN: return cgv::render::PT_TRIANGLE_FAN;
	case GL_QUADS: return cgv::render::PT_QUADS;
	case GL_QUAD_STRIP: return cgv::render::PT_QUAD_STRIP;
	case GL_POLYGON: return cgv::render::PT_POLYGON;
	}
	return cgv::render::PT_LAST;
}

cgv::render::type_descriptor get_element_type(const fx::gltf::Accessor& accessor)
{
	cgv::render::type_descriptor td(0);
	switch (accessor.componentType) {
	case fx::gltf::Accessor::ComponentType::Byte:         td.coordinate_type = cgv::type::info::TI_INT8; break;
	case fx::gltf::Accessor::ComponentType::UnsignedByte: td.coordinate_type = cgv::type::info::TI_UINT8; break;
	case fx::gltf::Accessor::ComponentType::Short:        td.coordinate_type = cgv::type::info::TI_INT16; break;
	case fx::gltf::Accessor::ComponentType::UnsignedShort:td.coordinate_type = cgv::type::info::TI_UINT16; break;
	case fx::gltf::Accessor::ComponentType::Float:        td.coordinate_type = cgv::type::info::TI_FLT32; break;
	case fx::gltf::Accessor::ComponentType::UnsignedInt:  td.coordinate_type = cgv::type::info::TI_UINT32; break;
		break;
	}
	switch (accessor.type)
	{
	case fx::gltf::Accessor::Type::Mat2: td.element_type = cgv::render::ET_MATRIX; td.nr_columns = td.nr_rows = 2; break;
	case fx::gltf::Accessor::Type::Mat3: td.element_type = cgv::render::ET_MATRIX; td.nr_columns = td.nr_rows = 3; break;
	case fx::gltf::Accessor::Type::Mat4: td.element_type = cgv::render::ET_MATRIX; td.nr_columns = td.nr_rows = 4; break;
	case fx::gltf::Accessor::Type::Scalar:td.element_type = cgv::render::ET_VALUE; td.nr_columns = td.nr_rows = 1; break;
	case fx::gltf::Accessor::Type::Vec2: td.element_type = cgv::render::ET_VECTOR; td.nr_columns = 1; td.nr_rows = 2; break;
	case fx::gltf::Accessor::Type::Vec3: td.element_type = cgv::render::ET_VECTOR; td.nr_columns = 1; td.nr_rows = 3; break;
	case fx::gltf::Accessor::Type::Vec4: td.element_type = cgv::render::ET_VECTOR; td.nr_columns = 1; td.nr_rows = 4; break;
	}
	td.normalize = accessor.normalized;
	td.is_row_major = false;
	td.is_array = false;
	return td;
}

void add_attribute(cgv::render::attribute_array& aa,
	const fx::gltf::Document& doc, const fx::gltf::Accessor& accessor,
	cgv::render::VertexAttributeID va_id, const std::string& name = "")
{
	const fx::gltf::BufferView& bufferView = doc.bufferViews[accessor.bufferView];
	aa.add_attribute(get_element_type(accessor), bufferView.buffer,
		bufferView.byteOffset + accessor.byteOffset, accessor.count,
		bufferView.byteStride, va_id, name);
}

void extract_additional_information(const fx::gltf::Document& doc,
	box3& box,
	size_t& vertex_count)
{
	vertex_count = 0;
	box.invalidate();
	// construct meshes
	for (const auto& M : doc.meshes) {
		for (const auto& p : M.primitives) {
			for (const auto& a : p.attributes) {
				if (a.first == "POSITION") {
					const fx::gltf::Accessor& accessor = doc.accessors[a.second];
					if (accessor.componentType != fx::gltf::Accessor::ComponentType::Float)
						continue;
					const fx::gltf::BufferView& bufferView = doc.bufferViews[accessor.bufferView];
					const uint8_t* data_ptr = &doc.buffers[bufferView.buffer].data[0] + accessor.byteOffset + bufferView.byteOffset;
					vertex_count += accessor.count;
					for (size_t i = 0; i < accessor.count; ++i) {
						switch (accessor.type) {
						case fx::gltf::Accessor::Type::Vec2:
							box.add_point(vec3(reinterpret_cast<const vec2*>(data_ptr)[i], 0.0f));
							break;
						case fx::gltf::Accessor::Type::Vec3:
							box.add_point(reinterpret_cast<const vec3*>(data_ptr)[i]);
							break;
						case fx::gltf::Accessor::Type::Vec4:
							box.add_point(*reinterpret_cast<const vec3*>(
								reinterpret_cast<const vec4*>(data_ptr) + i));
							break;
						}
					}
				}
			}
		}
	}
}

bool build_render_info(const std::string& file_name, const fx::gltf::Document& doc,
	cgv::render::context& ctx, cgv::render::render_info& R)
{
	std::string file_path = cgv::utils::file::get_path(file_name);

	if (!ctx.make_current()) {
		std::cerr << "need valid context for read_gltf" << std::endl;
		return false;
	}

	R.destruct(ctx);

	// construct buffers
	for (const auto& b : doc.buffers) {
		cgv::render::vertex_buffer* vbo = new cgv::render::vertex_buffer();
		vbo->create(ctx, &b.data[0], b.byteLength);
		R.ref_vbos().push_back(vbo);
	}

	// read images images
	std::vector<cgv::data::data_view*> dvs;
	std::vector<cgv::data::data_format*> dfs;
	for (const auto& i : doc.images) {
		if (i.IsEmbeddedResource()) {
			std::cerr << "could not deal with embedded image" << std::endl;
			return false;
		}
		else {
			dfs.push_back(new cgv::data::data_format());
			cgv::media::image::image_reader ir(*dfs.back());
			std::string image_file_name = file_path + "/" + i.uri;
			if (!ir.open(image_file_name)) {
				std::cerr << "could not open image " << image_file_name << std::endl;
				return false;
			}
			dvs.push_back(new cgv::data::data_view(dfs.back()));
			if (!ir.read_image(*dvs.back())) {
				std::cerr << "could not read image " << image_file_name << std::endl;
				return false;
			}
			ir.close();
		}
	}

	// construct textures
	for (const auto& t : doc.textures) {
		cgv::render::texture* tex = new cgv::render::texture();
		tex->create(ctx, *dvs[t.source]);
		const auto& s = doc.samplers[t.sampler];
		if (s.magFilter != fx::gltf::Sampler::MagFilter::None)
			tex->set_mag_filter(map_gl_to_tex_filter(GLenum(s.magFilter)));
		if (s.minFilter != fx::gltf::Sampler::MinFilter::None)
			tex->set_min_filter(map_gl_to_tex_filter(GLenum(s.minFilter)));
		tex->set_wrap_s(map_gl_to_tex_wrap(GLenum(s.wrapS)));
		tex->set_wrap_t(map_gl_to_tex_wrap(GLenum(s.wrapT)));
		R.ref_textures().push_back(tex);
	}

	// construct materials
	for (const auto& m : doc.materials) {
		cgv::render::textured_material* tm = new cgv::render::textured_material();
		tm->set_name(m.name);

		bool found_KHR_mat = false;
		m.extensionsAndExtras.count("extensions");
		if (m.extensionsAndExtras.count("extensions")) {
			auto& ext = m.extensionsAndExtras["extensions"];
			if (ext.count("KHR_materials_pbrSpecularGlossiness")) {
				auto& mat = ext["KHR_materials_pbrSpecularGlossiness"];
				found_KHR_mat = true;
				if (mat.count("diffuseFactor")) {
					auto& df = mat["diffuseFactor"];
					tm->set_diffuse_reflectance(rgb(
						df[0].get<float>(), df[1].get<float>(), df[2].get<float>()));
					tm->set_transparency(1.0f - df[3].get<float>());
				}
				if (mat.count("diffuseTexture")) {
					int ti = tm->add_texture_reference(*R.ref_textures()[mat["diffuseTexture"]["index"].get<int>()]);
					tm->set_diffuse_index(ti);
				}
				if (mat.count("glossinessFactor"))
					tm->set_roughness(1 - mat["glossinessFactor"].get<float>());
				if (mat.count("specularFactor")) {
					auto& sf = mat["specularFactor"];
					tm->set_diffuse_reflectance(rgb(
						sf[0].get<float>(), sf[1].get<float>(), sf[2].get<float>()));
				}
				if (mat.count("specularGlossinessTexture")) {
					int ti = tm->add_texture_reference(*R.ref_textures()[mat["specularGlossinessTexture"]["index"].get<int>()]);
					tm->set_specular_index(ti);
				}
			}
		}
		if (!found_KHR_mat) {
			tm->set_diffuse_reflectance(rgb(
				m.pbrMetallicRoughness.baseColorFactor[0],
				m.pbrMetallicRoughness.baseColorFactor[1],
				m.pbrMetallicRoughness.baseColorFactor[2]));
			tm->set_transparency(1.0f - m.pbrMetallicRoughness.baseColorFactor[3]);
			if (m.pbrMetallicRoughness.baseColorTexture.index != -1) {
				int ti = tm->add_texture_reference(*R.ref_textures()[m.pbrMetallicRoughness.baseColorTexture.index]);
				tm->set_diffuse_index(ti);
			}
			tm->set_roughness(m.pbrMetallicRoughness.roughnessFactor);
			tm->set_metalness(m.pbrMetallicRoughness.metallicFactor);
			if (m.pbrMetallicRoughness.metallicRoughnessTexture.index != -1) {
				int ti = tm->add_texture_reference(*R.ref_textures()[m.pbrMetallicRoughness.metallicRoughnessTexture.index]);
				tm->set_roughness_index(ti);
				tm->set_metalness_index(ti);
			}
			if (m.alphaMode == fx::gltf::Material::AlphaMode::Mask)
				tm->set_alpha_test(cgv::render::textured_material::AT_GREATER, m.alphaCutoff);
			tm->set_emission(rgba(
				m.emissiveFactor[0],
				m.emissiveFactor[1],
				m.emissiveFactor[2]));
			if (m.emissiveTexture.index != -1) {
				int ti = tm->add_texture_reference(*R.ref_textures()[m.emissiveTexture.index]);
				tm->set_emission_index(ti);
			}
		}
		R.ref_materials().push_back(tm);
	}

	// construct meshes
	for (const auto& M : doc.meshes) {
		for (const auto& p : M.primitives) {
			R.ref_aas().push_back(cgv::render::attribute_array());
			auto& aa = R.ref_aas().back();
			aa.aab_ptr = new cgv::render::attribute_array_binding();
			aa.aab_ptr->create(ctx);
			cgv::render::draw_call dc;
			for (const auto& a : p.attributes) {
				if (a.first == "POSITION") {
					add_attribute(aa, doc, doc.accessors[a.second], cgv::render::VA_POSITION);
					dc.count = doc.accessors[a.second].count;
				}
				else if (a.first == "NORMAL")
					add_attribute(aa, doc, doc.accessors[a.second], cgv::render::VA_NORMAL);
				else if (a.first == "TANGENT")
					add_attribute(aa, doc, doc.accessors[a.second], cgv::render::VA_BY_NAME, "tangent");
				else if (a.first == "TEXCOORD_0")
					add_attribute(aa, doc, doc.accessors[a.second], cgv::render::VA_TEXCOORD);
			}
			dc.indices = 0;
			dc.aa_index = uint32_t(R.ref_aas().size() - 1);
			dc.instance_count = 1;
			dc.draw_call_type = cgv::render::RCT_ARRAYS;
			dc.vertex_offset = 0;
			if (p.indices >= 0) {
				dc.draw_call_type = cgv::render::RCT_INDEXED;
				const fx::gltf::Accessor& accessor = doc.accessors[p.indices];
				const fx::gltf::BufferView& bufferView = doc.bufferViews[accessor.bufferView];
				dc.count = accessor.count;
				R.ref_aas().back().aab_ptr->set_element_array(ctx, *R.ref_vbos()[bufferView.buffer]);
				if (accessor.type != fx::gltf::Accessor::Type::Scalar) {
					std::cerr << "index type must by scalar" << std::endl;
					return false;
				}
				switch (accessor.componentType) {
				case fx::gltf::Accessor::ComponentType::UnsignedByte:
					dc.index_type = cgv::type::info::TI_UINT8;
					break;
				case fx::gltf::Accessor::ComponentType::UnsignedShort:
					dc.index_type = cgv::type::info::TI_UINT16;
					break;
				case fx::gltf::Accessor::ComponentType::UnsignedInt:
					dc.index_type = cgv::type::info::TI_UINT32;
					break;
				default:
					std::cerr << "index component type must be unsigned integer type" << std::endl;
					return false;
				}
				dc.indices = (void*)(unsigned long long)(accessor.byteOffset + bufferView.byteOffset);
			}
			dc.primitive_type = map_gl_to_primitive_type(GLenum(p.mode));
			if (p.material >= 0) {
				dc.material_index = p.material;
				dc.alpha_mode = cgv::render::AlphaMode(doc.materials[p.material].alphaMode);
				dc.alpha_cutoff = doc.materials[p.material].alphaCutoff;
				// std::cout << doc.materials[p.material].name << ": " << (int)doc.materials[p.material].alphaMode << "|" << doc.materials[p.material].alphaCutoff << std::endl;
			}
			else {
				dc.material_index = -1;
				dc.alpha_mode = cgv::render::AM_OPAQUE;
				dc.alpha_cutoff = 0.0f;
			}
			dc.prog = 0;
			R.ref_draw_calls().push_back(dc);
		}
	}
	return true;
}

void extract_mesh(const std::string& file_name, const fx::gltf::Document& doc,
	cgv::media::mesh::simple_mesh<float>& mesh, int mesh_index, int primitive_index)
{
	std::string file_path = cgv::utils::file::get_path(file_name);
	mesh.clear();

	// read images images
	std::vector<std::string> image_file_names;
	for (const auto& i : doc.images) {
		if (i.IsEmbeddedResource()) {
			std::cerr << "cannot not deal with embedded image" << std::endl;
			return;
		}
		else
			image_file_names.push_back(file_path + "/" + i.uri);
	}

	// construct materials
	for (const auto& m : doc.materials) {
		auto& mm = mesh.ref_material(mesh.new_material());
		mm.set_name(m.name);
		mm.set_diffuse_reflectance(rgba(
			m.pbrMetallicRoughness.baseColorFactor[0],
			m.pbrMetallicRoughness.baseColorFactor[1],
			m.pbrMetallicRoughness.baseColorFactor[2],
			m.pbrMetallicRoughness.baseColorFactor[3]));
		if (m.pbrMetallicRoughness.baseColorTexture.index != -1) {
			int ii = mm.add_image_file(image_file_names[
				doc.textures[m.pbrMetallicRoughness.baseColorTexture.index].source]);
			mm.set_diffuse_index(ii);
		}
		mm.set_roughness(m.pbrMetallicRoughness.roughnessFactor);
		mm.set_metalness(m.pbrMetallicRoughness.metallicFactor);
		if (m.pbrMetallicRoughness.metallicRoughnessTexture.index != -1) {
			int ii = mm.add_image_file(image_file_names[
				doc.textures[m.pbrMetallicRoughness.metallicRoughnessTexture.index].source]);
			mm.set_roughness_index(ii);
			mm.set_metalness_index(ii);
		}
		mm.set_emission(rgba(
			m.emissiveFactor[0],
			m.emissiveFactor[1],
			m.emissiveFactor[2]));
		if (m.emissiveTexture.index != -1) {
			int ii = mm.add_image_file(image_file_names[
				doc.textures[m.emissiveTexture.index].source]);
			mm.set_emission_index(ii);
		}
	}

	// construct mesh
	int prim = 0;
	for (const auto& M : doc.meshes) {
		if (mesh_index != -1 && (&M - &doc.meshes[0]) != mesh_index)
			continue;

		for (const auto& p : M.primitives) {
			if (primitive_index != -1 && (&p - &M.primitives[0]) != primitive_index)
				continue;

			int gi = mesh.new_group(M.name + "[" + cgv::utils::to_string(prim) + "]");
			int pi = mesh.get_nr_positions();
			int ni = mesh.get_nr_normals();
			int ti = mesh.get_nr_tex_coords();

			bool has_nmls = false;
			bool has_tcs = false;
			for (const auto& a : p.attributes) {
				const fx::gltf::Accessor& accessor = doc.accessors[a.second];
				const fx::gltf::BufferView& bufferView = doc.bufferViews[accessor.bufferView];
				const fx::gltf::Buffer& buffer = doc.buffers[bufferView.buffer];
				const uint8_t* data_ptr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				const float* float_ptr = reinterpret_cast<const float*>(data_ptr);
				if (a.first == "POSITION") {
					for (size_t i = 0; i < accessor.count; ++i) {
						mesh.new_position(cgv::media::mesh::simple_mesh<float>::vec3(3, float_ptr));
						float_ptr += 3;
					}
				}
				else if (a.first == "NORMAL") {
					has_nmls = true;
					for (size_t i = 0; i < accessor.count; ++i) {
						mesh.new_normal(cgv::media::mesh::simple_mesh<float>::vec3(3, float_ptr));
						float_ptr += 3;
					}
				}
				else if (a.first == "TEXCOORD_0") {
					has_tcs = true;
					for (size_t i = 0; i < accessor.count; ++i) {
						mesh.new_tex_coord(cgv::media::mesh::simple_mesh<float>::vec2(2, float_ptr));
						float_ptr += 2;
					}
				}
			}
			int mi = p.material;
			if (p.indices >= 0) {
				const fx::gltf::Accessor& accessor = doc.accessors[p.indices];
				const fx::gltf::BufferView& bufferView = doc.bufferViews[accessor.bufferView];
				const fx::gltf::Buffer& buffer = doc.buffers[bufferView.buffer];
				const uint8_t* data_ptr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				const uint16_t* ushort_ptr = reinterpret_cast<const uint16_t*>(data_ptr);
				const uint32_t* uint_ptr = reinterpret_cast<const uint32_t*>(data_ptr);
				if (p.mode == fx::gltf::Primitive::Mode::Triangles) {
					int fi;
					for (size_t i = 0; i < accessor.count; ++i) {
						if (i % 3 == 0) {
							fi = mesh.start_face();
							mesh.group_index(fi) = prim;
							mesh.material_index(fi) = mi;
						}
						uint32_t idx = -1;
						switch (accessor.componentType) {
						case fx::gltf::Accessor::ComponentType::UnsignedByte: idx = data_ptr[i]; break;
						case fx::gltf::Accessor::ComponentType::UnsignedShort: idx = ushort_ptr[i]; break;
						case fx::gltf::Accessor::ComponentType::UnsignedInt: idx = uint_ptr[i]; break;
						}
						mesh.new_corner(pi + idx, has_nmls ? ni + idx : -1, has_tcs ? ti + idx : -1);
					}
				}
			}
			++prim;
		}
	}
}

bool write_gltf(const std::string& file_name, const cgv::render::render_info& R)
{
	return false;
}

	}
}
