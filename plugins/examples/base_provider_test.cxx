#include <cgv/base/named.h>
#include <cgv/math/ftransform.h>
#include <cgv/gui/base_provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_reflect_types/media/color.h>
#include <cgv_gl/gl/gl.h>

using namespace cgv::base;
using namespace cgv::reflect;
using namespace cgv::render;
using namespace cgv::media;

class base_provider_test : public named, public drawable
{
protected:
	float line_width;
	unsigned nr_lines;
	double aspect;
	std::string text;
	color<float,RGB> col;
public:
	base_provider_test(const std::string& name) : named(name)
	{
		line_width = 3;
		nr_lines = 10;
		aspect = 4.0/3;
		text = "Hello World";
		col.R() = 1;
		col.G() = 0;
		col.B() = 1;
	}
	std::string get_type_name() const
	{
		return "base_provider_test";
	}
	bool self_reflect(reflection_handler& rh)
	{
		return
			rh.reflect_member("line_width", line_width) &&
			rh.reflect_member("nr_lines", nr_lines) &&
			rh.reflect_member("aspect", aspect) &&
			rh.reflect_member("text", text) &&
			rh.reflect_member("color", col);
	}
	void draw(context& ctx)
	{
		static std::vector<vec2> P;
		if (P.size() != 2 * nr_lines) {
			P.resize(2 * nr_lines);
			for (unsigned i = 0; i < nr_lines; ++i) {
				P[2 * i] = vec2((float)i / (nr_lines - 1), 0);
				P[2 * i + 1] = vec2((float)i / (nr_lines - 1), 1);
			}
		}
		ctx.push_modelview_matrix();
			ctx.mul_modelview_matrix(cgv::math::scale4<double>(aspect, 1, 1));
			shader_program& prog = ctx.ref_default_shader_program();
			prog.enable(ctx);
				ctx.set_color(col);
				attribute_array_binding::set_global_attribute_array(ctx, prog.get_position_index(), P);
				attribute_array_binding::enable_global_array(ctx, prog.get_position_index());
					glLineWidth(line_width);
					glDrawArrays(GL_LINES, 0, 2 * nr_lines);
				attribute_array_binding::disable_global_array(ctx, prog.get_position_index());
			prog.disable(ctx);
		ctx.pop_modelview_matrix();
	}
	void on_set(void* member_ptr)
	{
		post_redraw();
	}
};

#include <cgv/base/register.h>

cgv::base::factory_registration_1<base_provider_test,const char*> bpt_1("base_provider_test", "first test", "menu_text=\"new/first base provider test\"");
cgv::base::factory_registration_1<base_provider_test,const char*> bpt_2("base_provider_test", "test_the_provider", "menu_text=\"new/second base provider test\"");
