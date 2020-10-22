/********************

This example demonstrates the definition of user interfaces through
a description file with extension *.gui - in the examples plugin the 
file is examples.gui and contains two gui definitions for the simple
class base_provider_test implemented here. One is defined per type and
one specifically for the name "named_provider". 

At the end of this file two factories are added to the menu that create 
a base_provider_test instance with the specific name "named_provider" and
one with a different name ending up with the type based gui definition.

********************/

#include <cgv/base/named.h>
#include <cgv/reflect/method_interface_impl.h>
#include <cgv/math/ftransform.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_reflect_types/media/color.h>
#include <cgv_gl/gl/gl.h>

class base_provider_test : public cgv::base::named, public cgv::render::drawable
{
protected:
	// define some parameters that control rendering
	unsigned nr_lines;
	float line_width;
	rgb line_color;
	double layout_aspect;
	std::string some_text;
public:
	base_provider_test(const std::string& name) : cgv::base::named(name)
	{
		nr_lines = 10;
		line_width = 3;
		line_color = rgb(1, 0, 1);
		layout_aspect = 4.0/3;
		some_text = "Hello World";
	}
	std::string get_type_name() const
	{
		return "base_provider_test";
	}
	void test()
	{}
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return
			rh.reflect_member("nr_lines", nr_lines) &&
			rh.reflect_member("line_width", line_width) &&
			rh.reflect_member("line_color", line_color) &&
			rh.reflect_member("layout_aspect", layout_aspect) &&
			rh.reflect_member("some_text", some_text) &&
			rh.reflect_method("on_set", &base_provider_test::test);
	}
	void draw(cgv::render::context& ctx)
	{
		// compute end vertex locations of nr_lines lines
		static std::vector<vec2> P;
		if (P.size() != 2 * nr_lines) {
			P.resize(2 * nr_lines);
			for (unsigned i = 0; i < nr_lines; ++i) {
				P[2 * i] = vec2((float)i / (nr_lines - 1), 0);
				P[2 * i + 1] = vec2((float)i / (nr_lines - 1), 1);
			}
		}
		ctx.push_modelview_matrix();
			// non uniform scaling to aspect ratio of layout
			ctx.mul_modelview_matrix(cgv::math::scale4<double>(layout_aspect, 1, 1));
			// use default shader without illumination
			cgv::render::shader_program& prog = ctx.ref_default_shader_program();
			prog.enable(ctx);
				// set color attribute of shader program
				ctx.set_color(line_color);
				// bind line position attribute
				cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_position_index(), P);
				cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_position_index());
					// set line with and draw
					glLineWidth(line_width);
					glDrawArrays(GL_LINES, 0, 2 * nr_lines);
				cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_position_index());
			prog.disable(ctx);
		ctx.pop_modelview_matrix();
	}
	void on_set(void* member_ptr)
	{
		post_redraw();
	}
};

#include <cgv/base/register.h>

cgv::base::factory_registration_1<base_provider_test,const char*> bpt_1("base_provider_test", "provider with default type based gui", "menu_text=\"new/gui/base provider default\"");
cgv::base::factory_registration_1<base_provider_test,const char*> bpt_2("base_provider_test", "named_provider", "menu_text=\"new/gui/base provider named\"");
