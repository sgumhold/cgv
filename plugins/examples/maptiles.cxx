
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

// CGV OpenGL library
#include <cgv_gl/sphere_renderer.h>

// CGV application library
#include <cgv_app/application_plugin.h>

// Maptiles library
#include <3rd/maptiles/include/TileManagerData.h>



//////
//
// Class definitions / implelentations
//

/// Example application plugin showcasing the maptiles library inside the CGV Framework.
class maptiles : public cgv::app::application_plugin // inherit from application plugin to enable overlay support
{

public:

	////
	// Object construction / destruction

	/// The default constructor.
	maptiles() : application_plugin("Map Tiles")
	{}

	/// The destructor.
	virtual ~maptiles()
	{}


	////
	// Interfrace: cgv::app::application_plugin

	virtual bool init(cgv::render::context &ctx) override
	{
		return true;
	}

	virtual void clear(cgv::render::context &ctx) override
	{}

	virtual void handle_member_change(const cgv::utils::pointer_test &m) override
	{}

	std::string get_type_name() const override { return "maptiles"; }

	void stream_help(std::ostream &os) override
	{
		os << "Map Tiles:\a\n"
		   << "";
	}

	virtual bool handle_event (cgv::gui::event &e) override
	{
		return true;
	}

	virtual void draw(cgv::render::context &ctx) override
	{}

	void create_gui() override
	{
		add_decorator("Map Tiles GUI", "heading");
	}
};



//////
//
// Plugins registration
//

// Our maptiles plugin
cgv::base::factory_registration<maptiles> maptiles_factory("New/Demo/Map Tiles", "", true);
