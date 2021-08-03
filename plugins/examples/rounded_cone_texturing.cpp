#include <random>
#include <unordered_map>

#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/math/ftransform.h>
#include <cgv/media/image/image.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/render/drawable.h>
#include <cgv/render/texture.h>
#include <cgv_glutil/cone_render_data.h>
#include <cgv_glutil/sphere_render_data.h>

class rounded_cone_texturing : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider {
protected:
	cgv::render::rounded_cone_render_style cone_style;
	cgv::glutil::cone_render_data<> rd;

	cgv::render::texture tex;
public:
	rounded_cone_texturing() : cgv::base::node("rounded cone texturing test") {		
		cone_style.radius = 0.1f;
		cone_style.surface_color = rgb(1.0f, 0.5f, 0.2f);
		cone_style.enable_texturing = true;
		cone_style.texture_blend_mode = cgv::render::rounded_cone_render_style::TBM_TINT;
		rd = cgv::glutil::cone_render_data<>(true);
	}
	void on_set(void* member_ptr) {
		post_redraw();
		update_member(member_ptr);
	}
	std::string get_type_name() const {
		return "rounded_cone_texturing";
	}
	void clear(cgv::render::context& ctx) {
		cgv::render::ref_rounded_cone_renderer(ctx, -1);
		rd.destruct(ctx);
		tex.destruct(ctx);
	}
	bool init(cgv::render::context& ctx) {
		cgv::render::ref_rounded_cone_renderer(ctx, 1);
		if(!rd.init(ctx))
			return false;

		for(unsigned i = 0; i < 3; ++i) {
			float x = 0.5f * static_cast<float>(i + 1);
			float y = 0.3f * static_cast<float>(i);
			rd.add(vec3(-x, y, 0.0f), vec3(x, y, 0.0f));
		}
		rd.set_out_of_date();

		cgv::data::data_format tex_format;
		cgv::media::image::image_reader image(tex_format);
		cgv::data::data_view tex_data;

		std::string file_name = "res://plus.png";
		if(!image.read_image(file_name, tex_data)) {
			std::cout << "Error: Could not read image file " << file_name << std::endl;
			return false;
		} else {
			tex.create(ctx, tex_data, 0);
			tex.set_min_filter(cgv::render::TextureFilter::TF_LINEAR);
			tex.set_mag_filter(cgv::render::TextureFilter::TF_LINEAR);
			tex.set_wrap_s(cgv::render::TextureWrap::TW_REPEAT);
			tex.set_wrap_t(cgv::render::TextureWrap::TW_REPEAT);
		}
		image.close();
		return true;
	}
	void draw(cgv::render::context& ctx) {
		if(!tex.is_created())
			return;

		auto& rcr = ref_rounded_cone_renderer(ctx);
		rcr.set_albedo_texture(&tex);
		rd.render(ctx, rcr, cone_style);
	}
	void create_gui() {
		add_decorator("rounded cone texturing", "heading");

		if(begin_tree_node("cone style", cone_style, true)) {
			align("\a");
			add_gui("", cone_style);
			align("\b");
			end_tree_node(cone_style);
		}
	}
};



class rounded_cone_tree : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider {
protected:
	cgv::render::rounded_cone_render_style stem_style;
	cgv::glutil::cone_render_data<> stems;
	cgv::render::sphere_render_style leave_style;
	cgv::glutil::sphere_render_data<> leaves;
	
	cgv::render::texture tex;

	unsigned seed = 1;
	unsigned n = 5;
	std::string axiom;
	std::unordered_map<char, std::string> rules;

	float growth_factor = 0.5f;
	float angle_factor = 0.85f;
	float radius_factor = 0.95f;

public:
	rounded_cone_tree() : cgv::base::node("rounded cone tree test") {
		leave_style.surface_color = rgb(0.2f, 0.48f, 0.2f);
		leave_style.radius = 0.2f;

		stem_style.radius_scale = 4.0f;
		stem_style.surface_color = rgb(1.0f, 0.5f, 0.2f);
		stem_style.enable_texturing = true;
		stem_style.texture_blend_mode = cgv::render::rounded_cone_render_style::TBM_MIX;
		stem_style.texture_tiling.y() = 2.0f;
		stem_style.texture_use_reference_length = true;
		stem_style.texture_reference_length = 0.75f;
		stems = cgv::glutil::cone_render_data<>(true);
		leaves = cgv::glutil::sphere_render_data<>(true);

		axiom = "X";
		rules['X'] = "F+[[X]-X]-F[-FX]+X";
		rules['F'] = "FF";

		//axiom = "F";
		//rules['F'] = "FF+[+F-F-F]-[-F+F+F]";
	}
	void on_set(void* member_ptr) {
		if (member_ptr == &seed ||
			member_ptr == &n ||
			member_ptr == &growth_factor ||
			member_ptr == &angle_factor ||
			member_ptr == &radius_factor) {
			n = cgv::math::clamp(n, 0u, 5u);
			std::string sentence = generate(n);
			//std::cout << sentence << std::endl;
			create_tree(sentence);
		}

		post_redraw();
		update_member(member_ptr);
	}
	std::string get_type_name() const {
		return "rounded_cone_tree";
	}
	void clear(cgv::render::context& ctx) {
		cgv::render::ref_sphere_renderer(ctx, -1);
		cgv::render::ref_rounded_cone_renderer(ctx, -1);
		stems.destruct(ctx);
		leaves.destruct(ctx);
		tex.destruct(ctx);
	}
	bool init(cgv::render::context& ctx) {
		cgv::render::ref_sphere_renderer(ctx, 1);
		cgv::render::ref_rounded_cone_renderer(ctx, 1);

		if(!stems.init(ctx))
			return false;
		if(!leaves.init(ctx))
			return false;
		
		create_tree(generate(n));

		cgv::data::data_format tex_format;
		cgv::media::image::image_reader image(tex_format);
		cgv::data::data_view tex_data;

		std::string file_name = "res://bark.png";
		if(!image.read_image(file_name, tex_data)) {
			std::cout << "Error: Could not read image file " << file_name << std::endl;
			return false;
		} else {
			tex.create(ctx, tex_data, 0);
			tex.set_min_filter(cgv::render::TextureFilter::TF_LINEAR);
			tex.set_mag_filter(cgv::render::TextureFilter::TF_LINEAR);
			tex.set_wrap_s(cgv::render::TextureWrap::TW_REPEAT);
			tex.set_wrap_t(cgv::render::TextureWrap::TW_REPEAT);
		}
		image.close();

		cgv::render::view* view_ptr = find_view_as_node();
		if(view_ptr) {
			view_ptr->set_eye_keep_view_angle(dvec3(0.0f, 20.0f, 40.0f));
			view_ptr->set_focus(vec3(0.0f, 20.0f, 0.0f));
		}

		return true;
	}
	void draw(cgv::render::context& ctx) {
		if(!tex.is_created())
			return;

		auto& rcr = ref_rounded_cone_renderer(ctx);
		rcr.set_albedo_texture(&tex);
		stems.render(ctx, rcr, stem_style);

		leaves.render(ctx, ref_sphere_renderer(ctx), leave_style);
	}
	void create_gui() {
		add_decorator("rounded cone texturing", "heading");

		add_member_control(this, "seed", seed, "value", "min=0;max=10000;step=1");
		add_member_control(this, "n", n, "value_slider", "min=0;max=5;step=1;ticks=true");
		add_member_control(this, "growth factor", growth_factor, "value_slider", "min=0.0;max=1.0;step=0.01;ticks=true");
		add_member_control(this, "angle factor", angle_factor, "value_slider", "min=0.0;max=1.0;step=0.01;ticks=true");
		add_member_control(this, "radius factor", radius_factor, "value_slider", "min=0.8;max=1.0;step=0.01;ticks=true");

		if(begin_tree_node("Stems", stem_style, true)) {
			align("\a");
			add_gui("", stem_style);
			align("\b");
			end_tree_node(stem_style);
		}
		if(begin_tree_node("Leaves", leave_style, true)) {
			align("\a");
			add_gui("", leave_style);
			align("\b");
			end_tree_node(leave_style);
		}
	}
	std::string generate(unsigned n = 0) {
		std::string sentence = axiom;
		std::string next_sentence = "";
		for(unsigned i = 0; i < n; ++i) {
			for(unsigned j = 0; j < sentence.length(); ++j) {
				char c = sentence[j];
				auto& it = rules.find(c);
				if(it != rules.end()) {
					auto& r = (*it);
					next_sentence += r.second;
				} else {
					next_sentence += c;
				}
			}
			sentence = next_sentence;
			next_sentence = "";
		}
		return sentence;
	}
	vec3 get_ortho_vec(const vec3& v) {
		return abs(v.x()) > abs(v.z()) ? vec3(-v.y(), v.x(), 0.0f) : vec3(0.0f, -v.z(), v.y());
	}
	void get_local_frame(const vec3& d, vec3& u, vec3& v) {
		vec3 dn = normalize(d);
		u = normalize(cross(dn, get_ortho_vec(dn)));
		v = cross(u, dn);
	}
	void create_tree(const std::string& sentence) {
		std::mt19937 rng(seed);
		std::uniform_real_distribution<float> distr(0.0f, 1.0f);

		stems.clear();
		leaves.clear();

		std::stack<vec3> pos_stack;
		std::stack<vec3> dir_stack;
		std::stack<float> rad_stack;

		//mat3 R_left = cgv::math::rotate3(25.0f, vec3(0.0f, 0.0f, 1.0f));
		//mat3 R_right = cgv::math::rotate3(-25.0f, vec3(0.0f, 0.0f, 1.0f));

		vec3 pos(0.0f);
		vec3 dir(0.0f, 1.0f, 0.0f);

		float l = growth_factor;// / static_cast<float>(n);
		float rad = 0.2f;// / (0.25f*static_cast<float>(n));

		vec3 start_pos = pos;
		float start_rad = rad;

		for(unsigned i = 0; i < sentence.length(); ++i) {
			const char& c = sentence[i];
			switch(c) {
			case 'F':
				//rd.add(pos, pos + l * dir);
				//rd.adds(rad);
				pos += l * dir;
				rad *= radius_factor;
				//rd.adds(rad);
				break;
			case '-':
			{
				vec3 u, v;
				get_local_frame(dir, u, v);
				dir -= angle_factor * distr(rng) * u;
				dir -= angle_factor * distr(rng) * v;
				//dir = R_left * dir;
				dir.normalize();

				stems.add(start_pos, pos);
				stems.add(start_rad, rad);
				start_pos = pos;
				start_rad = rad;

			} break;
			case '+':
			{
				vec3 u, v;
				get_local_frame(dir, u, v);
				dir += angle_factor * distr(rng) * u;
				dir += angle_factor * distr(rng) * v;
				//dir = R_right * dir;
				dir.normalize();

				stems.add(start_pos, pos);
				stems.add(start_rad, rad);
				start_pos = pos;
				start_rad = rad;

			} break;
			case '[':
				pos_stack.push(pos);
				dir_stack.push(dir);
				rad_stack.push(rad);
				break;
			case ']':
				leaves.add(pos);
				leaves.add(1.5f * (0.8f*distr(rng) + 0.2 * rad));

				pos = pos_stack.top();
				dir = dir_stack.top();
				rad = rad_stack.top();
				pos_stack.pop();
				dir_stack.pop();
				rad_stack.pop();

				start_pos = pos;
				start_rad = rad;

				break;
			default: break;
			}
		}

		stems.set_out_of_date();
		leaves.set_out_of_date();
	}
};

#include <cgv/base/register.h>

/// register a factory to create new rounded cone texturing tests
cgv::base::factory_registration<rounded_cone_texturing> test_rounded_cone_texturing_fac("new/demo/rounded_cone_texturing", '5');
cgv::base::factory_registration<rounded_cone_tree> test_rounded_cone_tree_fac("new/demo/rounded_cone_tree", '6');
