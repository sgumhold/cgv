
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
#include <maptiles/TileManager2.h>
#include <maptiles/TileRenderer.h>
#include <maptiles/3rd/WGS84toCartesian/WGS84toCartesian.hpp>

//#include <cgv/render/texture.h>



//////
//
// Class definitions / implelentations
//

/// Example application plugin showcasing the maptiles library inside the CGV Framework.
class maptiles : public cgv::app::application_plugin // inherit from application plugin to enable overlay support
{
	typedef cgv::math::fvec<float, 4> vec4;
  protected:
	TileManager2 manager;
	TileRenderer renderer;
	GlobalConfig config;

	double latitude;
	double longitude;
	double altitude;

	cgv::dmat4 offset;
	double x, y;

	bool render_raster_tile;
	bool render_tile3D;
  public:

	////
	// Object construction / destruction

	/// The default constructor.
	maptiles() : application_plugin("Map Tiles")
	{ 
		render_raster_tile = true;
		render_tile3D = true;
		latitude = 0;
		longitude = 0;
		altitude = 0;
		x = 0; 
		y = 0;

		offset.zeros();
		offset(0, 0) = 1.0f;
		offset(1, 1) = 1.0f;
		offset(2, 2) = 1.0f;
		offset(3, 3) = 1.0f;

	}

	/// The destructor.
	virtual ~maptiles()
	{}


	////
	// Interfrace: cgv::app::application_plugin

	virtual bool init(cgv::render::context &ctx) override
	{ 
		config.ReferencePoint = {51.02596, 13.7230};
		renderer.Init(ctx);
		manager.Init(51.02596, 13.7230, 10, &config);
		connect_copy(manager.tile_downloaded, cgv::signal::rebind(this, &maptiles::tile_download_callback));
		manager.Update(ctx);

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
		return false;
	}

	virtual void draw(cgv::render::context &ctx) override 
	{ 
		// We need to keep track of the x and y coordinate of the original camera matrix for recentering
		auto& real_mv = ctx.get_modelview_matrix();
		x = real_mv(0, 3);
		y = real_mv(1, 3);

		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(offset);

		auto& mv = ctx.get_modelview_matrix();
		auto& cam_pos = inv(mv) * vec4(0.0f, 0.0f, 0.0f, 1.0f);
		std::array<double, 2> cameraPosWGS84 = wgs84::fromCartesian({config.ReferencePoint.lat, config.ReferencePoint.lon}, {cam_pos[0], cam_pos[1]});

		latitude = cameraPosWGS84[0];
		longitude = cameraPosWGS84[1];
		altitude = std::max(cam_pos[2], 1.0f);

		manager.SetPosition(cameraPosWGS84[0], cameraPosWGS84[1], std::max((cam_pos[2] * 0.25f), 1.0f));
		manager.Update(ctx);

		if (render_raster_tile)
		{
			auto& raster_tiles = manager.GetActiveRasterTiles();
			for (auto& pair : raster_tiles) {
				auto& raster_tile = pair.second;
				renderer.Draw(ctx, raster_tile);
			}
		}

		if (render_tile3D) 
		{
			auto& tile3Ds = manager.GetActiveTile3Ds();
			cgv::math::fvec<float, 3> camera_pos = {cam_pos[0], cam_pos[1], cam_pos[2]};
			for (auto& pair : tile3Ds)
			{
				auto& tile3D = pair.second;
				renderer.Draw(ctx, tile3D, camera_pos);
			}
		}

		ctx.pop_modelview_matrix();
	}

	void on_set(void* member_ptr)
	{ 
		update_member(member_ptr);
		post_redraw();
	}

	void recenter() 
	{ 
		std::cout << "recentering at (" << latitude << ", " << longitude << ")\n ";

		offset(0, 3) = -x;
		offset(1, 3) = -y;
		config.ReferencePoint = {latitude, longitude};
		manager.ReInit(latitude, longitude, altitude, &config);
		post_redraw();
	}

	void tile_download_callback() 
	{ 
		post_redraw();
	}

	void create_gui() override
	{
		add_decorator("Map Tiles GUI", "heading");
		//add_decorator("Position", "heading", "level=3");
		//add_member_control(this, "latitude", latitude, "value_slider", "min=-90;max=90;ticks=false");
		//add_member_control(this, "latitude", longitude, "value_slider", "min=-90;max=90;ticks=false");
		add_decorator("Rendering", "heading", "level=3");
		add_member_control(this, "Raster Tiles", render_raster_tile, "check");
		add_member_control(this, "Tile 3Ds", render_tile3D, "check");
		
		add_member_control(this, "Grid Size Raster Tile", config.NeighbourhoodFetchSizeRasterTile, "value_slider",
						   "min=0;max=5;ticks=false");
		add_member_control(this, "Grid Size Tile3D", config.NeighbourhoodFetchSizeTile3D, "value_slider",
						   "min=0;max=5;ticks=false");
		
		connect_copy(add_button("Re-Center")->click, cgv::signal::rebind(this, &maptiles::recenter));
	}
};



//////
//
// Plugins registration
//

// Our maptiles plugin
cgv::base::factory_registration<maptiles> maptiles_factory("New/Demo/Map Tiles", "", true);
