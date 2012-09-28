#include <cgv/base/named.h>
#include <cgv/gui/base_provider.h>
#include <cgv/render/drawable.h>
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
		glPushAttrib(GL_LINE_BIT|GL_CURRENT_BIT|GL_LIGHTING_BIT);
		glDisable(GL_LIGHTING);
		glPushMatrix();
		glScaled(aspect, 1, 1);
		glColor3fv(&col[0]);
		glLineWidth(line_width);

		glBegin(GL_LINES);
		for (unsigned i=0; i<nr_lines; ++i) {
			glVertex2f((float)i/(nr_lines-1),0);	
			glVertex2f((float)i/(nr_lines-1),1);	
		}
		glEnd();
		glPopMatrix();
		glPopAttrib();
	}
	void on_set(void* member_ptr)
	{
		post_redraw();
	}
};

#include <cgv/base/register.h>

cgv::base::factory_registration_1<base_provider_test,const char*> bpt_1("base_provider_test", "first test", "menu_text=\"new/first base provider test\"");
cgv::base::factory_registration_1<base_provider_test,const char*> bpt_2("base_provider_test", "test_the_provider", "menu_text=\"new/second base provider test\"");
