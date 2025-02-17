//////
//
// Includes
//

// CGV Framework core
#include "cgv/math/fvec.h"
#include "cgv/render/context.h"
#include <cgv/base/base.h>
#include <cgv/base/register.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv/math/ftransform.h>
#include <cgv/render/drawable.h>

#include <cgv_gl/spline_tube_renderer.h>

#include <cgv_app/application_plugin.h>

#include <random>

class spline_tubes : public cgv::app::application_plugin
{
protected:
	std::vector<cgv::vec3> points;
    std::vector<cgv::vec4> colors;
	std::vector<cgv::vec4> tangents;
	std::vector<float> radii;
	std::vector<unsigned int> indices;
	cgv::render::spline_tube_render_style spline_style;
	cgv::render::attribute_array_manager manager;

	float f0, df0, f1, df1;
	int iteration;
    
public:
    spline_tubes() : application_plugin("Spline Tubes")
    {
		//std::random_device dev;
		//std::mt19937 rng(dev());
		
		// fix seed for performance testing
		std::mt19937 rng(0);
		std::uniform_int_distribution<std::mt19937::result_type> dist(0, 20); 

		std::cout << dist(rng) << std::endl;

		int index_count = 0;
		for (int i = 0; i < 2000; i++) {
			if (1)
			{
				points.push_back(cgv::vec3(dist(rng), dist(rng), dist(rng)) - cgv::vec3(10, 10, 40));
				points.push_back(cgv::vec3(dist(rng), dist(rng), dist(rng)) - cgv::vec3(10, 10, 40));
				tangents.push_back(cgv::vec4(cgv::vec3(dist(rng), dist(rng), dist(rng)), 0));
				tangents.push_back(cgv::vec4(cgv::vec3(dist(rng), dist(rng), dist(rng)), 0));
				colors.push_back(cgv::vec4(dist(rng) / 20.0, dist(rng) / 20.0, dist(rng) / 20.0, 1));
				colors.push_back(cgv::vec4(dist(rng) / 20.0, dist(rng) / 20.0, dist(rng) / 20.0, 1));
				radii.push_back(.2);
				radii.push_back(.2);
				indices.push_back(index_count++);
				indices.push_back(index_count++);
			}
			else
			{
				cgv::vec3(dist(rng), dist(rng), dist(rng));
				cgv::vec3(dist(rng), dist(rng), dist(rng));
				cgv::vec4(cgv::vec3(dist(rng), dist(rng), dist(rng)), 0);
				cgv::vec4(cgv::vec3(dist(rng), dist(rng), dist(rng)), 0);
				cgv::vec4(dist(rng) / 20.0, dist(rng) / 20.0, dist(rng) / 20.0, 1);
				cgv::vec4(dist(rng) / 20.0, dist(rng) / 20.0, dist(rng) / 20.0, 1);
			}
		}

		f0 = f1 = df0 = df1 = 1.0;
		iteration = 10;
	}

    virtual ~spline_tubes()
    {

    }

    virtual bool init(cgv::render::context &ctx) override
    {
		if (!manager.init(ctx))
			return false;

        cgv::render::ref_spline_tube_renderer(ctx, 1);
        return true;
    }

    virtual void clear(cgv::render::context& ctx) override 
    { 
        manager.destruct(ctx);
		cgv::render::ref_spline_tube_renderer(ctx, -1);
    }

    virtual void handle_member_change(const cgv::utils::pointer_test& m) override 
    {}

    void stream_help(std::ostream& os) override
	{
		os << "Spline Tubes:\a\n"
		   << "";
	}

    virtual bool handle_event(cgv::gui::event& e) override 
    { 
        return false; 
    }

    virtual void draw(cgv::render::context& ctx) override
    { 
        cgv::render::spline_tube_renderer& renderer = cgv::render::ref_spline_tube_renderer(ctx);
		renderer.enable_attribute_array_manager(ctx, manager);
		renderer.set_position_array(ctx, &points.front(), points.size(), sizeof(cgv::vec3));
		renderer.set_color_array(ctx, &colors.front(), colors.size(), sizeof(cgv::vec4));
		renderer.set_indices(ctx, indices);
		renderer.set_radius_array(ctx, &radii.front(), radii.size(), sizeof(float));
		renderer.set_tangent_array(ctx, &tangents.front(), tangents.size(), sizeof(cgv::vec4));

		auto& prog = renderer.ref_prog();
		
		prog.set_uniform(ctx, prog.get_uniform_location(ctx, "f0"),		f0);
		prog.set_uniform(ctx, prog.get_uniform_location(ctx, "df0"),	df0);
		prog.set_uniform(ctx, prog.get_uniform_location(ctx, "f1"),		f1);
		prog.set_uniform(ctx, prog.get_uniform_location(ctx, "df1"),	df1);
		prog.set_uniform(ctx, prog.get_uniform_location(ctx, "iteration_count"), iteration);
        
		renderer.render(ctx, 0, indices.size());
        renderer.disable_attribute_array_manager(ctx, manager);
    }

    void on_set(void* member_ptr) 
	{ 
		update_member(member_ptr);
		post_redraw();
    }

    void create_gui() override
    {
        add_decorator("Spline Tubes GUI", "heading");
		add_member_control(this, "iterations", iteration, "value", "min=0;max=20");
		add_decorator("Newtons Method Tuning", "heading");
		add_decorator("RootFindD1", "heading", "level=2");
		add_member_control(this, "f", f1, "value", "min=0.0001;max=100");
		add_member_control(this, "df", df1, "value", "min=0.0001;max=100");
		add_decorator("RootFindD0", "heading", "level=2");
		add_member_control(this, "f", f0, "value", "min=0.0001;max=100");
		add_member_control(this, "df", df0, "value", "min=0.0001;max=100");
    }
};

cgv::base::object_registration<spline_tubes> spline_tubes_object("");