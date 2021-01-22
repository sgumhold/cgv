
// C++ STL
#include <vector>
#include <set>

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
#include "traj_loader.h"


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

	/// path of the dataset to load - can be either a directory or a single file
	std::string datapath;

	/// rendering-related configurable fields
	struct
	{
		// renderer to use
		Renderer renderer = SPLINE_TUBE;

		/// render style for the rounded cone renderer
		cgv::render::surface_render_style render_style;
	} render_cfg;	

	/// misc configurable fields
	struct
	{
		/// proxy for controlling fltk_gl_view::instant_redraw
		bool instant_redraw_proxy = false;
	} misc_cfg;

	/// rendering state fields
	struct
	{
		/// style for the rounded cone renderer
		cgv::render::rounded_cone_render_style rounded_cone_rstyle;

		/// style for the spline tube renderer
		cgv::render::spline_tube_render_style spline_tube_rstyle;
	} render;

	/// drag-n-drop state fields
	struct
	{
		/// current mouse position
		ivec2 pos;

		/// current drag-n-drop string
		std::string text;

		/// list of filenames extracted from @ref #text
		std::vector<std::string> filenames;
	} dnd;

	/// dataset state fields
	struct
	{
		/// set of filepaths for loading
		std::set<std::string> files;
	} dataset;

	/// trajectory data manager
	traj_manager<float> traj_mgr;


public:
	tubes() : node("tubes_instance")
	{
		// adjusted rounded cone renderer defaults
		render.rounded_cone_rstyle.enable_ambient_occlusion = true;
	}

	void handle_args (std::vector<std::string> &args)
	{
		// look out for potential dataset files
		std::vector<unsigned> arg_ids;
		for (unsigned i=0; i<(unsigned)args.size(); i++)
		{
			const std::string &arg = args[i], first10 = arg.substr(0, 11);
			if (cgv::utils::to_lower(first10).compare("tubes_file:") == 0)
			{
				std::string file = arg.substr(11, arg.length());
				// this appears to be a dataset file we're supposed to load
				dataset.files.emplace(file);
				arg_ids.emplace_back(i);
			}
			// ToDo: add support for a "tubes_dir" kind of argument
		}

		// process our arguments (if any)
		if (!arg_ids.empty())
		{
			// remove the arguments we grabbed
			for (unsigned i=(unsigned)arg_ids.size()-1; i!=0; i--)
				args.erase(args.begin() + arg_ids[i]);
			// announce change in dataset_fn
			on_set(&dataset);
		}
	}

	std::string get_type_name (void) const
	{
		return "tubes";
	}

	bool self_reflect (cgv::reflect::reflection_handler &rh)
	{
		return
			rh.reflect_member("datapath", datapath) &&
			rh.reflect_member("renderer", render_cfg.renderer) &&
			rh.reflect_member("render_style", render_cfg.render_style) &&
			rh.reflect_member("instant_redraw_proxy", misc_cfg.instant_redraw_proxy);
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
					// store (and process) dnd text on enter event, since it's not available in drag events
					dnd.text = me.get_dnd_text();
					std::vector<cgv::utils::line> lines;
					cgv::utils::split_to_lines(dnd.text, lines, true);
					dnd.filenames.reserve(lines.size());
					for (const auto &line : lines)
						dnd.filenames.emplace_back(cgv::utils::to_string(line));
					return true;
				}

				case cgv::gui::MA_DRAG:
				{
					// during dragging keep track of drop position and redraw
					dnd.pos = ivec2(me.get_x(), me.get_y());
					post_redraw();
					return true;
				}

				case cgv::gui::MA_LEAVE:
				{
					// when mouse leaves window, cancel the dnd operation (and redraw to clear the dnd indicator
					// onscreen text)
					dnd.text.clear();
					dnd.filenames.clear();
					post_redraw();
					return true;
				}

				case cgv::gui::MA_RELEASE:
				{
					// process the files that where dropped onto the window
					dataset.files.clear();
					for (const std::string &filename : dnd.filenames)
						dataset.files.emplace(filename);
					dnd.filenames.clear();
					dnd.text.clear();
					on_set(&dataset);
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
		if (!dnd.text.empty())
		{
			static const rgb dnd_col(1, 0.5f, 0.5f);
			// compile the text we're going to draw and gather its approximate dimensions at the same time
			float w = 0, s = ctx.get_current_font_size();
			std::stringstream dnd_drawtext;
			dnd_drawtext << "Load dataset:" << std::endl;
			{
				std::string tmp;
				for (const std::string &filename : dnd.filenames)
				{
					tmp = "   "; tmp += filename;
					w = std::max(w, ctx.get_current_font_face()->measure_text_width(tmp, s));
					dnd_drawtext << tmp << std::endl;
				}
			}
			float h = dnd.filenames.size() * s + s;
			// gather our available screen estate
			GLint vp[4]; glGetIntegerv(GL_VIEWPORT, vp);
			// calculate actual position at which to place the text
			ivec2 pos = dnd.pos,
			      overflow(vp[0]+vp[2] - dnd.pos.x()-int(std::ceil(w)),
			               vp[1]+vp[3] - dnd.pos.y()-int(std::ceil(h)));
			// - first, try to prevent truncation at the right and bottom borders
			if (overflow.x() < 0)
				pos.x() = std::max(1, pos.x()+overflow.x());
			if (overflow.y() < 0)
				pos.y() = std::max(1, pos.y()+overflow.y());
			// - then, absolutely prevent truncation at the left and top borders
			pos.x() = std::max(vp[0]+2, pos.x());
			pos.y() = std::max(vp[1]+signed(s), pos.y());
			// draw the text
			ctx.push_pixel_coords();
				ctx.set_color(dnd_col);
				ctx.set_cursor(vecn(float(pos.x()), float(pos.y())), "", cgv::render::TA_TOP_LEFT);
				ctx.output_stream() << dnd_drawtext.str();
				ctx.output_stream().flush();
			ctx.pop_pixel_coords();
		}

		// draw dataset using selected renderer
		if (traj_mgr.has_data())
		{
			auto &vao = traj_mgr.get_attrib_array_binding();
			switch (render_cfg.renderer)
			{
				case ROUNDED_CONE:
				{
					auto &rcr = cgv::render::ref_rounded_cone_renderer(ctx);
					rcr.set_render_style(render.rounded_cone_rstyle);
					rcr.set_position_array(ctx, vao.positions);
					rcr.set_radius_array(ctx, vao.radii);
					rcr.set_color_array(ctx, vao.colors);
					rcr.set_indices(ctx, vao.indices);
					rcr.render(ctx, 0, vao.indices.size());
					break;
				}

				case SPLINE_TUBE:
				{
					auto &str = cgv::render::ref_spline_tube_renderer(ctx);
					str.set_render_style(render.spline_tube_rstyle);
					str.set_position_array(ctx, vao.positions);
					str.set_tangent_array(ctx, vao.tangents);
					str.set_radius_array(ctx, vao.radii);
					str.set_color_array(ctx, vao.colors);
					str.set_indices(ctx, vao.indices);
					str.render(ctx, 0, vao.indices.size());
				}
			}
		}
	}

	void create_gui (void)
	{
		// dataset settings
		add_decorator("Dataset", "heading", "level=1");
		add_member_control(this, "data file/path", datapath);

		// rendering settings
		add_decorator("Rendering", "heading", "level=1");
		add_member_control(
			this, "renderer", render_cfg.renderer, "dropdown",
			"enums='ROUNDED_CONE=0,SPLINE_TUBE=1';tooltip='The built-in renderer to use for drawing the tubes.'"
		);
		if (begin_tree_node("tube surface material", render_cfg, false))
		{
			align("\a");
			add_gui("render_style", render_cfg.render_style);
			align("\b");
			end_tree_node(render_cfg);
		}

		// Misc settings contractable section
		add_decorator("Miscellaneous", "heading", "level=1");
		if (begin_tree_node("tools (persistent by default)", misc_cfg, false))
		{
			align("\a");
			add_member_control(
				this, "instant_redraw_proxy", misc_cfg.instant_redraw_proxy, "toggle",
				"tooltip='Controls the instant redraw state of the FLTK GL window.'"
			);
			align("\b");
			end_tree_node(misc_cfg);
		}
	}

	void on_set (void *member_ptr)
	{
		// dataset settings
		// - configurable datapath
		if (member_ptr == &datapath && !datapath.empty())
		{
			if (traj_mgr.load(datapath))
			{
				dataset.files.clear();
				dataset.files.emplace(datapath);
			}
		}
		// - non-configurable dataset logic
		else if (member_ptr == &dataset)
		{
			datapath.clear();
			traj_mgr.clear();
			for (auto& file : dataset.files)
				traj_mgr.load(file);
			update_member(&datapath);
		}

		// misc settings
		// - instant redraw
		else if (member_ptr == &misc_cfg.instant_redraw_proxy)
		{
			// ToDo: handle the (virtually impossible) case that some other plugin than cg_fltk provides the gl_context
			dynamic_cast<fltk_gl_view*>(get_context())->set_void("instant_redraw", "bool", member_ptr);
		}

		// default implementation for all members
		// - dirty hack to catch GUI changes to render_cfg.render_style
		*dynamic_cast<cgv::render::surface_render_style*>(&render.rounded_cone_rstyle) = render_cfg.render_style;
		*dynamic_cast<cgv::render::surface_render_style*>(&render.spline_tube_rstyle) = render_cfg.render_style;
		// - remaining logic
		update_member(member_ptr);
		post_redraw();
	}
};


#include <cgv/base/register.h>
cgv::base::object_registration<tubes> reg_tubes("");

#ifdef CGV_FORCE_STATIC
	cgv::base::registration_order_definition ro_def("stereo_view_interactor;tubes");
#endif
