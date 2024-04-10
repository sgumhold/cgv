#include <cgv/gui/trigger.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/node.h>
#include <cgv/utils/file.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/progression.h>
#include <cgv/base/register.h>
#include <cgv/math/fvec.h>
#include <cgv/math/ftransform.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/gl/gl.h>
#include <random>

class huge :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider
{
protected:
	bool aam_out_of_date;
	cgv::render::attribute_array_manager aam;
	// per vertex location
	std::vector<cgv::vec4> spheres;
	std::vector<cgv::rgba> colors;
	bool sort_points;
	bool blend;
	size_t nr_primitives;
	// render objects
	std::vector<unsigned> indices;
	cgv::render::view* view_ptr;

	cgv::render::sphere_render_style sphere_style;
	cgv::render::sphere_renderer s_renderer;

	void generate_spheres()
	{
		spheres.clear();
		colors.clear();
		// generate random geometry
		std::default_random_engine g;
		std::uniform_real_distribution<float> d(0.0f, 1.0f);
		float radius = 0.1f* float(1.0/pow(double(nr_primitives), 0.3333333));
		cgv::utils::progression prog("generate", nr_primitives, 10);
		spheres.resize(nr_primitives);
		colors.resize(nr_primitives);
		for (size_t i = 0; i < nr_primitives; ++i) {
			prog.step();
			spheres[i] = cgv::vec4(d(g), d(g), d(g), (d(g)+0.2f)*radius);
			colors[i] = cgv::rgba(d(g), d(g), d(g), 0.5f*d(g) + 0.5f);
		}
		aam_out_of_date = true;
	}
public:
	/// define format and texture filters in constructor
	huge() : cgv::base::node("huge")
	{
		sort_points = false;
		blend = false;
		nr_primitives = 16000;
		generate_spheres();
	}
	std::string get_type_name() const
	{
		return "huge";
	}
	
	void on_set(void* member_ptr)
	{
		if (member_ptr == &nr_primitives) {
			if (nr_primitives > spheres.max_size())
				nr_primitives = spheres.max_size();
			if (nr_primitives > colors.max_size())
				nr_primitives = colors.max_size();
			generate_spheres();
		}
		update_member(member_ptr);
		post_redraw();
	}

	bool init(cgv::render::context& ctx)
	{
		if ((view_ptr = find_view_as_node()))
			view_ptr->set_focus(cgv::vec3(0.5f, 0.5f, 0.5f));
		ctx.set_bg_clr_idx(4);

		if (!s_renderer.init(ctx))
			return false;
		aam.init(ctx);
		s_renderer.set_y_view_angle(float(view_ptr->get_y_view_angle()));
		s_renderer.set_render_style(sphere_style);
		s_renderer.enable_attribute_array_manager(ctx, aam);
		return true;
	}

	void render_points(cgv::render::context& ctx, cgv::render::renderer& R)
	{
		if (sort_points) {
			indices.resize(spheres.size());
			for (unsigned i = 0; i < indices.size(); ++i)
				indices[i] = i;
			struct sort_pred {
				const std::vector<cgv::vec4>& spheres;
				const cgv::vec3& view_dir;
				bool operator () (GLint i, GLint j) const {
					return dot(cgv::vec3(spheres[i]), view_dir) > dot(cgv::vec3(spheres[j]), view_dir);
				}
				sort_pred(const std::vector<cgv::vec4>& _spheres, const cgv::vec3& _view_dir) : spheres(_spheres), view_dir(_view_dir) {}
			};
			cgv::vec3 view_dir = view_ptr->get_view_dir();
			std::sort(indices.begin(), indices.end(), sort_pred(spheres, view_dir));
			R.set_indices(ctx, indices);
		}
		else
			R.remove_indices(ctx);
		R.render(ctx, 0, spheres.size());
	}
	void draw(cgv::render::context& ctx)
	{

		if (blend) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		if (aam_out_of_date) {
			s_renderer.set_sphere_array(ctx, spheres);
			s_renderer.set_color_array(ctx, colors);
			aam_out_of_date = false;
		}
		render_points(ctx, s_renderer);
		if (blend)
			glDisable(GL_BLEND);
	}
	void clear(cgv::render::context& ctx)
	{
		s_renderer.clear(ctx);
		aam.destruct(ctx);
	}
	void create_gui()
	{
		add_decorator("huge", "heading");
		add_member_control(this, "nr_primitives", nr_primitives, "value_slider", "min=1;max=494967296;log=true;ticks=true");
		add_member_control(this, "sort", sort_points, "toggle", "shortcut='S'");
		add_member_control(this, "blend", blend, "toggle", "shortcut='B'");
		if (begin_tree_node("Sphere Rendering", sphere_style, false)) {
			align("\a");
			add_gui("sphere_style", sphere_style);
			align("\b");
			end_tree_node(sphere_style);
		}
	}
};

cgv::base::factory_registration<huge> fr_huge("New/Render/Huge", 'H', true);