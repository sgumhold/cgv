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
    
public:
    spline_tubes() : application_plugin("Spline Tubes")
    { 
		/*
		float factor = 10;
        points.push_back(cgv::vec3(-0.5, 0, 0)*factor);
		points.push_back(cgv::vec3(-0.25, 0, 0.33)*factor);
		points.push_back(cgv::vec3( 0,   0, 0.5)*factor);
		points.push_back(cgv::vec3(0.25, 0, 0.5)*factor);
		points.push_back(cgv::vec3(0.5, 0, 0.33)*factor);
		points.push_back(cgv::vec3( 0.75, 0, 0)*factor);

        tangents.push_back(cgv::vec3(1.0, 0, 0));
		tangents.push_back(cgv::math::normalize(cgv::vec3(1.0, 0, 1.0)));
		tangents.push_back(cgv::vec3(1.0, 0, 0));
		tangents.push_back(cgv::vec3(1.0, 0, 0));
		tangents.push_back(cgv::math::normalize(cgv::vec3(1.0, 0, 1.0)));
		tangents.push_back(cgv::vec3(1.0, 0, 0));

        colors.push_back(cgv::vec4(1.0, 0, 0, 1));
		colors.push_back(cgv::vec4(0, 1.0, 0, 1));
		colors.push_back(cgv::vec4(0, 1.0, 0, 1));
		colors.push_back(cgv::vec4(0, 0, 1.0, 1));
		colors.push_back(cgv::vec4(0, 0, 1.0, 1));
		colors.push_back(cgv::vec4(0, 0, 1.0, 1));
		
        radii.push_back(0.2);
		radii.push_back(0.2);
		radii.push_back(0.2);
		radii.push_back(0.2);
		radii.push_back(0.2);
		radii.push_back(0.2);

        indices.push_back(0);
		indices.push_back(1);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(2);
		indices.push_back(3);
		indices.push_back(3);
		indices.push_back(4);
		indices.push_back(4);
		indices.push_back(5);
		*/

		/*
		points.push_back(cgv::vec3(-2, 0, 0));
		points.push_back(cgv::vec3(-1, 0.5, 0.5));
		points.push_back(cgv::vec3(0, 0, 1));
		points.push_back(cgv::vec3(1, -0.5, 0.5));
		points.push_back(cgv::vec3(2, 0, 0));

		tangents.push_back(cgv::vec4(cgv::math::normalize(cgv::vec3(1, 1, 1)),  0));
		tangents.push_back(cgv::vec4(cgv::math::normalize(cgv::vec3(1, -1, 1)), 0));
		tangents.push_back(cgv::vec4(cgv::math::normalize(cgv::vec3(1, -1, 0)), 0));
		tangents.push_back(cgv::vec4(cgv::math::normalize(cgv::vec3(1, 1, -1)), 0));
		tangents.push_back(cgv::vec4(cgv::math::normalize(cgv::vec3(1, 1, -1)), 0));

		colors.push_back(cgv::vec4(1.0, 0, 0, 1));
		colors.push_back(cgv::vec4(1.0, 1, 0, 1));
		colors.push_back(cgv::vec4(0, 1.0, 0, 1));
		colors.push_back(cgv::vec4(0, 1.0, 1, 1));
		colors.push_back(cgv::vec4(0, 0, 1, 1));
		
		radii.push_back(.2f);
		radii.push_back(.2f);
		radii.push_back(.2f);
		radii.push_back(.2f);
		radii.push_back(.2f);

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(2);
		indices.push_back(3);
		indices.push_back(3);
		indices.push_back(4);
		*/

		//std::random_device dev;
		//std::mt19937 rng(dev());
		
		std::mt19937 rng(0);
		std::uniform_int_distribution<std::mt19937::result_type> dist6(0, 20); 

		std::cout << dist6(rng) << std::endl;

		unsigned int index_count = 0;
		for (int i = 0; i < 2000; i++) {
			points.push_back(cgv::vec3(dist6(rng), dist6(rng), dist6(rng)) - cgv::vec3(10, 10, 40));
			points.push_back(cgv::vec3(dist6(rng), dist6(rng), dist6(rng)) - cgv::vec3(10, 10, 40));
			tangents.push_back(cgv::vec4(cgv::vec3(dist6(rng), dist6(rng), dist6(rng)), 0));
			tangents.push_back(cgv::vec4(cgv::vec3(dist6(rng), dist6(rng), dist6(rng)), 0));
			colors.push_back(cgv::vec4(dist6(rng) / 20.0, dist6(rng) / 20.0, dist6(rng) / 20.0, 1));
			colors.push_back(cgv::vec4(dist6(rng) / 20.0, dist6(rng) / 20.0, dist6(rng) / 20.0, 1));
			radii.push_back(.2);
			radii.push_back(.2);
			indices.push_back(index_count++);
			indices.push_back(index_count++);
		}

		/*
		points.push_back(cgv::vec3(-2, 0, 0));
		points.push_back(cgv::vec3(2, 0, 0));
		tangents.push_back(cgv::vec4(cgv::math::normalize(cgv::vec3(-1, 0, 1)), 0));
		tangents.push_back(cgv::vec4(cgv::math::normalize(cgv::vec3(-1, 0, -1)), 0));

		colors.push_back(cgv::vec4(1.0, 0, 0, 1));
		colors.push_back(cgv::vec4(0, 0, 1, 1));

		radii.push_back(.2);
		radii.push_back(.2);

		indices.push_back(0);
		indices.push_back(1);
		*/
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
        renderer.render(ctx, 0, indices.size());
        renderer.disable_attribute_array_manager(ctx, manager);
    }

    void on_set(void* member_ptr)
    {

    }

    void create_gui() override
    {
        add_decorator("Spline Tubes GUI", "heading");
    }
};

cgv::base::object_registration<spline_tubes> spline_tubes_object("");