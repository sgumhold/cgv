
// C++ STL
#include <random>
#include <fstream>

// CGV framework core
#include <cgv/base/node.h>
#include <cgv/base/register.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/utils/advanced_scan.h>

// fltk_gl_view for controlling instant redraw
#include <plugins/cg_fltk/fltk_gl_view.h>

// renderer headers include self reflection helpers for render styles
#include <cgv_gl/rounded_cone_renderer.h>
#include <cgv_gl/spline_tube_renderer.h>

// local includes
#include "bezdat.h"


/// plugin for testing the various tube renderers
class tubes :
	public cgv::base::node,             // derive from node to integrate into global tree structure and to store a name
	public cgv::base::argument_handler, // derive from argument handler to be able to process custom arguments
	public cgv::gui::provider,          // derive from provider to obtain a GUI tab
	public cgv::gui::event_handler,     // derive from event handler to be able to directly react to user interaction
	public cgv::render::drawable        // derive from drawable for being able to render
{
public:

	/// floating point type
	typedef float real;

	/// renderer type
	enum Renderer { ROUNDED_CONE=0, SPLINE_TUBE=1 };


protected:

	/// filename of the dataset
	std::string dataset_fn;

	// renderer to use
	Renderer renderer = SPLINE_TUBE;

	/// proxy field for controlling fltk_gl_view::instant_redraw
	bool instant_redraw_proxy = false;

	/// style for the rounded cone renderer
	cgv::render::rounded_cone_render_style rounded_cone_rstyle;

	/// style for the spline tube renderer
	cgv::render::spline_tube_render_style spline_tube_rstyle;

	/// current drag-n-drop mouse position
	ivec2 dnd_pos;

	/// current drag-n-drop string
	std::string dnd_text;

	/// current drag-n-drop file
	std::string dnd_file;

	/// trajectory data loader
	bezdat_handler<float> loader;


public:
	tubes() : node("tubes_instance")
	{
		// adjusted rounded cone renderer defaults
		rounded_cone_rstyle.enable_ambient_occlusion = true;

		// adjusted spline tube renderer defaults
		/* none_so_far */;
	}

	void handle_args (std::vector<std::string> &args)
	{
		// look for our custom argument
		signed idx=-1;
		for (unsigned i=0; i<(unsigned)args.size(); i++)
		{
			auto &arg = args[i];
			if (arg[0] != '-' && arg[0] != '/')
			{
				// this appears to not be a command line switch, grab it as the filename of a dataset to load
				dataset_fn = arg;
				idx = (signed)i;
				break;
			}
		}

		// process our argument (if any)
		if (idx > -1)
		{
			// remove the argument we grabbed
			args.erase(args.begin() + ((unsigned)idx));
			// announce change in dataset_fn
			on_set(&dataset_fn);
		}
	}

	std::string get_type_name (void) const
	{
		return "tubes";
	}

	bool self_reflect (cgv::reflect::reflection_handler &rh)
	{
		return
			rh.reflect_member("dataset_fn", dataset_fn) &&
			rh.reflect_member("renderer", renderer) &&
			rh.reflect_member("instant_redraw_proxy", instant_redraw_proxy) &&
			rh.reflect_member("rounded_cone_rstyle", rounded_cone_rstyle) &&
			rh.reflect_member("spline_tube_rstyle", spline_tube_rstyle);
	}

	bool init (cgv::render::context &ctx)
	{
		// increase reference count of both renderers by one
		auto &rcr = cgv::render::ref_rounded_cone_renderer(ctx, 1);
		auto &str = cgv::render::ref_spline_tube_renderer(ctx, 1);

		// report init status
		return rcr.ref_prog().is_linked() && str.ref_prog().is_linked();
	}

	void clear (cgv::render::context &ctx)
	{
		// decrease reference count of both renderers by one
		cgv::render::ref_spline_tube_renderer(ctx, -1);
		cgv::render::ref_rounded_cone_renderer(ctx, -1);
	}

	void stream_help (std::ostream &os)
	{
		os << "tubes" << std::endl;
	}

	bool handle (cgv::gui::event &e)
	{
		if (e.get_kind() == cgv::gui::EID_MOUSE)
		{
			cgv::gui::mouse_event &me = static_cast<cgv::gui::mouse_event&>(e);
			// select drag and drop events only
			if ((me.get_flags() & cgv::gui::EF_DND) != 0) switch (me.get_action())
			{
				case cgv::gui::MA_ENTER:
				{
					// store dnd text and file (i.e. the first item in the text) on enter event, since
					// it's not available in drag events
					dnd_text = me.get_dnd_text();
					std::vector<cgv::utils::line> lines;
					cgv::utils::split_to_lines(dnd_text, lines, true);
					dnd_file = cgv::utils::to_string(lines.front());
					return true;
				}

				case cgv::gui::MA_DRAG:
				{
					// during dragging keep track of drop position and redraw
					dnd_pos = ivec2(me.get_x(), me.get_y());
					post_redraw();
					return true;
				}

				case cgv::gui::MA_LEAVE:
				{
					// when mouse leaves window, cancel the dnd operation
					dnd_text.clear();
					dnd_file.clear();
					post_redraw();
					return true;
				}

				case cgv::gui::MA_RELEASE:
				{
					// release corresponds to drop
					dataset_fn = dnd_file;
					dnd_text.clear();
					dnd_file.clear();
					on_set(&dataset_fn);
					return true;
				}

				default:
					/* DoNothing() */;
			}
		}
		return false;
	}

	void draw (cgv::render::context &ctx)
	{
		// display drag-n-drop information, if a dnd operation is in progress
		if (!dnd_text.empty())
		{
			static const rgb dnd_col(1, 0.5f, 0.5f);
			std::vector<cgv::utils::line> lines;
			cgv::utils::split_to_lines(dnd_text, lines);
			ctx.push_pixel_coords();
				ctx.set_color(dnd_col);
				ctx.set_cursor(vecn(float(dnd_pos.x()), float(dnd_pos.y())), "", cgv::render::TA_TOP_LEFT);
				ctx.output_stream() << "Load dataset \""<<cgv::utils::to_string(lines.front())<<"\"" << std::endl;
			ctx.pop_pixel_coords();
		}

		// prepare common database
		/*std::vector<vec3> P;
		std::vector<real> R;
		std::vector<rgb> C;
		P.emplace_back(vec3{0, 0, 0}); R.emplace_back(real(0.125)); C.emplace_back(rgb(1, 0, 0));
		P.emplace_back(vec3{2, 0, 0}); R.emplace_back(real(0.25)); C.emplace_back(rgb(0, 1, 0));
		P.emplace_back(vec3{2, 0, 2}); R.emplace_back(real(0.375)); C.emplace_back(rgb(0, 0, 1));
		std::vector<unsigned> Idx = {0, 1, 1, 2};*/
		bool dataset_loaded = loader.has_data();
		const std::vector<vec3> &P = (dataset_loaded ? loader.positions() : std::vector<vec3>());
		const std::vector<vec4> &dP = (dataset_loaded ? loader.tangents() : std::vector<vec4>());
		const std::vector<real> &R = (dataset_loaded ? loader.radii() : std::vector<real>());
		const std::vector<rgb> &C = (dataset_loaded ? loader.colors() : std::vector<rgb>());
		const std::vector<unsigned> &Idx = (dataset_loaded ? loader.indices() : std::vector<unsigned>());

		// draw dataset using selected renderer
		if (dataset_loaded) switch (renderer)
		{
			case ROUNDED_CONE:
			{
				auto &rcr = cgv::render::ref_rounded_cone_renderer(ctx);
				rcr.set_render_style(rounded_cone_rstyle);
				rcr.set_position_array(ctx, P);
				rcr.set_radius_array(ctx, R);
				rcr.set_color_array(ctx, C);
				rcr.set_indices(ctx, Idx);
				rcr.render(ctx, 0, Idx.size());
				break;
			}

			case SPLINE_TUBE:
			{
				auto &str = cgv::render::ref_spline_tube_renderer(ctx);
				str.set_render_style(spline_tube_rstyle);
				str.set_position_array(ctx, P);
				str.set_tangent_array(ctx, dP);
				str.set_radius_array(ctx, R);
				str.set_color_array(ctx, C);
				str.set_indices(ctx, Idx);
				str.render(ctx, 0, Idx.size());
			}
		}
	}

	void create_gui (void)
	{
		add_decorator("Dataset", "heading", "level=1");
		add_member_control(this, "dataset_fn", dataset_fn);
		add_decorator("Rendering", "heading", "level=1");
		add_member_control(this, "renderer", renderer, "dropdown", "enums='ROUNDED_CONE=0,SPLINE_TUBE=1';tooltip='The built-in renderer to use for drawing the tubes.'");
		add_decorator("Misc. settings (persistent by default)", "heading", "level=1");
		add_member_control(this, "instant_redraw_proxy", instant_redraw_proxy, "toggle", "tooltip='Controls the instant redraw state of the FLTK GL window.'");
	}

	void on_set (void *member_ptr)
	{
		// Dataset settings
		// - filename
		if (member_ptr == &dataset_fn && !dataset_fn.empty())
		{
			std::ifstream file(dataset_fn);
			if (!file.is_open())
				return;
			loader.read(file);
		}

		// Misc settings
		// - instant redraw
		if (member_ptr == &instant_redraw_proxy)
		{
			// ToDo: handle the (virtually impossible) case that some other plugin than cg_fltk provides the gl_context
			dynamic_cast<fltk_gl_view*>(get_context())->set_void("instant_redraw", "bool", member_ptr);
		}

		// default implementation for all members
		update_member(member_ptr);
		post_redraw();
	}
};


#include <cgv/base/register.h>
cgv::base::object_registration<tubes> reg_tubes("");

#ifdef CGV_FORCE_STATIC
	cgv::base::registration_order_definition ro_def("stereo_view_interactor;tubes");
#endif
