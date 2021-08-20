#pragma once

#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/texture.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_glutil/frame_buffer_container.h>
#include <cgv_glutil/overlay.h>
#include <cgv_glutil/shader_library.h>

#include "lib_begin.h"

namespace cgv {
namespace glutil{

class CGV_API transfer_function : public cgv::render::render_types {
public:
	typedef std::pair<float, rgb> color_node;
	typedef std::pair<float, float> opacity_node;

private:
	std::vector<color_node> color_nodes;
	std::vector<opacity_node> opacity_nodes;

public:
	transfer_function() {

		color_nodes.clear();
		opacity_nodes.clear();
	}

	~transfer_function() {

		color_nodes.clear();
		opacity_nodes.clear();
	}

	void clear() {

		color_nodes.clear();
		opacity_nodes.clear();
	}

	void add_color_point(float t, rgb color) {

		// make sure t is in the interval [0,1]
		t = cgv::math::clamp(t, 0.0f, 1.0f);
		color_nodes.push_back(std::make_pair(t, color));
		std::sort(color_nodes.begin(), color_nodes.end(), [](const color_node& a, const color_node& b) { return a.first < b.first; });
	}

	void add_opacity_point(float t, float opacity) {

		t = cgv::math::clamp(t, 0.0f, 1.0f);
		opacity_nodes.push_back(std::make_pair(t, opacity));
		std::sort(opacity_nodes.begin(), opacity_nodes.end(), [](const opacity_node& a, const opacity_node& b) { return a.first < b.first; });
	}

	const std::vector<color_node>& ref_color_nodes() { return color_nodes; }
	const std::vector<opacity_node>& ref_opacity_nodes() { return opacity_nodes; }

	template<typename T>
	T interpolate_type(std::vector<std::pair<float, T>>& nodes, float t) {

		unsigned count = nodes.size();

		if(count == 0)
			return (T)0;

		if(count == 1)
			return nodes[0].second;

		t = cgv::math::clamp(t, 0.0f, 1.0f);

		if(t > nodes[0].first) {
			unsigned idx = 0;

			for(unsigned i = 1; i < count; ++i) {
				if(nodes[idx].first <= t && nodes[i].first > t)
					break;
				idx = i;
			}

			if(idx < count - 1) {
				std::pair<float, T> n0 = nodes[idx];
				std::pair<float, T> n1 = nodes[idx + 1];

				float t0 = n0.first;
				float t1 = n1.first;

				float a = (t - t0) / (t1 - t0);

				return (T)((1.0f - a) * n0.second + a * n1.second);
			} else {
				return nodes[idx].second;
			}
		} else {
			return nodes[0].second;
		}
	}

	rgb interpolate_color(float t) {

		return interpolate_type<rgb>(color_nodes, t);
	}

	float interpolate_alpha(float t) {

		return interpolate_type<float>(opacity_nodes, t);
	}

	rgba interpolate(float t) {

		rgb color = interpolate_type<rgb>(color_nodes, t);
		float opacity = interpolate_type<float>(opacity_nodes, t);

		return rgba(color.R(), color.G(), color.B(), opacity);
	}
};








class CGV_API transfer_function_editor : public overlay {
protected:
	cgv::glutil::frame_buffer_container fbc;
	cgv::glutil::shader_library shaders;

	bool show;
	
	float opacity_scale_exponent;

	std::string file_name;
	std::string save_file_name;
	bool has_unsaved_changes = false;

	struct layout_attributes {
		int padding;
		int total_height;
		int color_scale_height;

		// dependent members
		rect editor_rect;
		rect color_scale_rect;

		void update(const ivec2& parent_size) {

			editor_rect.set_pos(ivec2(padding) + ivec2(0, color_scale_height + 1)); // add one for a small border
			editor_rect.set_size(parent_size - 2 * padding - ivec2(0, color_scale_height + 1));

			color_scale_rect.set_pos(ivec2(padding));
			color_scale_rect.set_size(ivec2(parent_size.x() - 2*padding, color_scale_height));
		}
	} layout;
	
	struct point {
		static constexpr float radius = 10.0f;
		vec2 val;
		vec2 pos;

		rgb col;

		vec2 center() const { return pos + radius; }

		void update_val(const layout_attributes& la, const float scale_exponent) {

			vec2 min = la.editor_rect.pos() - radius;
			vec2 max = min + la.editor_rect.size();

			pos.x() = cgv::math::clamp(pos.x(), min.x(), max.x());
			pos.y() = cgv::math::clamp(pos.y(), min.y(), max.y());

			vec2 p = (pos + radius) - la.editor_rect.pos();
			val = p / la.editor_rect.size();
			val.x() = cgv::math::clamp(val.x(), 0.0f, 1.0f);

			val.y() = cgv::math::clamp(val.y(), 0.0f, 1.0f);

			val.y() = std::pow(val.y(), scale_exponent);
			val.y() = cgv::math::clamp(val.y(), 0.0f, 1.0f);
		}

		void update_pos(const layout_attributes& la, const float scale_exponent) {

			val.x() = cgv::math::clamp(val.x(), 0.0f, 1.0f);
			val.y() = cgv::math::clamp(val.y(), 0.0f, 1.0f);

			vec2 t = val;

			t.y() = std::pow(t.y(), 1.0f/scale_exponent);
			t.y() = cgv::math::clamp(t.y(), 0.0f, 1.0f);

			pos = la.editor_rect.pos() + t * la.editor_rect.size() - radius;
		}

		bool is_hit(const vec2& mp) const {

			float dist = length(mp - (pos + radius));
			return dist <= radius;
		}
	};

	struct vertex {
		vec2 pos;
		rgba col;
	};

	struct vertex2 {
		vec2 pos;
		rgb col;
	};

	texture bg_tex;
	texture tf_tex;

	template<typename PosType, typename ColType>
	struct plain_geometry {
		struct vertex_type {
			PosType pos;
			ColType col;
		};

		std::vector<vertex_type> vertices;

		type_descriptor pos_type_descriptor = cgv::render::element_descriptor_traits<PosType>::get_type_descriptor(PosType());
		type_descriptor col_type_descriptor = cgv::render::element_descriptor_traits<ColType>::get_type_descriptor(ColType());

		vertex_buffer vb;
		attribute_array_binding aab;

		size_t size() { return vertices.size(); }

		void clear(context& ctx) {
			vertices.clear();

			if(vb.is_created())
				vb.destruct(ctx);
			if(aab.is_created())
				aab.destruct(ctx);
		}

		bool create(context& ctx, const shader_program& prog) {
			bool success = true;
			success &= vb.create(ctx, &(vertices[0]), vertices.size());
			success &= aab.create(ctx);
			success &= aab.set_attribute_array(ctx, prog.get_position_index(), pos_type_descriptor, vb, 0, vertices.size(), sizeof(vertex_type));
			success &= aab.set_attribute_array(ctx, prog.get_color_index(), col_type_descriptor, vb, sizeof(PosType), vertices.size(), sizeof(vertex_type));
			return success;
		}

		void add(const PosType& pos, const ColType& col) {
			vertices.push_back({ pos, col });
		}

		void render(context& ctx, PrimitiveType type, shader_program& prog) {
			render(ctx, type, 0, size(), prog);
		}

		void render(context& ctx, PrimitiveType type, int offset, size_t count, shader_program& prog) {
			if(aab.is_created()) {
				prog.enable(ctx);
				aab.enable(ctx);
				GLenum mode = gl::map_to_gl(type);
				glDrawArrays(mode, (GLint)offset, (GLsizei)count);
				aab.disable(ctx);
				prog.disable(ctx);
			}
		}
	};

	struct tf_container {
		std::vector<point> points;

		transfer_function tf;
		texture tex;

		texture hist_tex;
		unsigned hist_max;

		plain_geometry<vec2, rgba> triangles;
		plain_geometry<vec2, rgb> lines;

		tf_container() {

			points.resize(2);
			points[0].val = vec2(0.0f);
			points[0].col = rgb(0.0f);
			points[1].val = vec2(1.0f);
			points[1].col = rgb(1.0f);
		}
	} tfc;

	point* selected_point = nullptr;
	point* dragged_point = nullptr;
	vec2 offset_pos;

	void add_point(const vec2& pos);
	void remove_point(const point* ptr);
	point* get_hit_point(const vec2& pos);
	void init_transfer_function_texture(context& ctx);
	void update();
	void update_transfer_function();

	bool load_from_xml(const std::string& file_name);
	bool save_to_xml(const std::string& file_name);

public:
	transfer_function_editor();
	std::string get_type_name() const { return "transfer_function_editor"; }

	bool on_exit_request();
	void clear(cgv::render::context& ctx);

	bool self_reflect(cgv::reflect::reflection_handler& _rh);
	void stream_help(std::ostream& os) {}

	bool handle_event(cgv::gui::event& e);
	void on_set(void* member_ptr);
	//void on_overlay_layout_change();

	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
	
	void create_gui();
	void create_gui(cgv::gui::provider& p);

	void is_visible(bool visible);
	void toggle_visibility();

	texture& ref_tex();

	bool set_histogram(const std::vector<unsigned>& data);
};

}
}

#include <cgv/config/lib_end.h>
