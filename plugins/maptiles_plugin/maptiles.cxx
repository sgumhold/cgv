
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
#include <cgv/render/stereo_view.h>


//////
//
// Class definitions / implelentations
//

/// Example application plugin showcasing the maptiles library inside the CGV Framework.
class maptiles : public cgv::app::application_plugin // inherit from application plugin to enable overlay support
{
	typedef cgv::math::fvec<double, 4> vec4;
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
	bool auto_recenter;
  public:

	////
	// Object construction / destruction

	/// The default constructor.
	maptiles() : application_plugin("Map Tiles")
	{ 
		render_raster_tile = true;
		render_tile3D = true;
		auto_recenter = true;
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

	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{ 
		bool success = true;

		success = rh.reflect_member("latitude", latitude) && success;
		success = rh.reflect_member("longitude", longitude) && success;

		return success;
	}

	virtual bool init(cgv::render::context &ctx) override
	{ 
		//config.ReferencePoint = {51.02596, 13.7230};
		config.ReferencePoint = {latitude, longitude};
		
		renderer.Init(ctx);
		manager.Init(config.ReferencePoint.lat, config.ReferencePoint.lon, 10, &config);
		connect_copy(manager.tile_downloaded, cgv::signal::rebind(this, &maptiles::tile_download_callback));
		//manager.Update(ctx);
		initialize_view_ptr();
		return true;
	}

	virtual void init_frame(cgv::render::context& ctx) override
	{
		
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
		auto& original_mv = ctx.get_modelview_matrix();
		
		auto original_mv_inverse = inv(original_mv);
		x = original_mv_inverse(0, 3);
		y = original_mv_inverse(1, 3);

		ctx.push_modelview_matrix();
		ctx.set_modelview_matrix(offset * original_mv);
		
		auto& mv = ctx.get_modelview_matrix();
		auto cam_pos = inv(mv) * vec4(0.0, 0.0, 0.0, 1.0);

		if (auto_recenter && (cam_pos[0] > config.AutoRecenterDistance || cam_pos[1] > config.AutoRecenterDistance))
			recenter();

		std::cout << "ModelView (OG): \n" << original_mv << std::endl;
		std::cout << "ModelView: \n" << mv << std::endl;
		std::array<double, 2> cameraPosWGS84 =
			  wgs84::fromCartesian({config.ReferencePoint.lat, config.ReferencePoint.lon}, {cam_pos[0], cam_pos[1]});

		manager.CalculateViewFrustum(ctx.get_projection_matrix() * mv);

		std::cout << "Camera: " << cam_pos << " (" << cameraPosWGS84[0] << ", " << cameraPosWGS84[1] << ")\n";
		std::cout << "Offset Matrix\n" << offset << std::endl;
		std::cout << "Offset: (" << x << ", " << y << ")\n";
		//std::cout << "MVP (OG): \n" << ctx.get_projection_matrix() * original_mv << std::endl;
		//std::cout << "MVP: \n" << ctx.get_projection_matrix() * mv << std::endl;

		latitude = cameraPosWGS84[0];
		longitude = cameraPosWGS84[1];
		altitude = std::max(cam_pos[2], 1.0);

		manager.SetPosition(cameraPosWGS84[0], cameraPosWGS84[1], std::max((cam_pos[2] * 0.25), 1.0));
		manager.Update(ctx);

		{
			auto camera = dynamic_cast<cgv::render::stereo_view*>(view_ptr);
			cgv::math::vec<double> minp(0.0, 0.0, 0.0), maxp(0.0f, 0.0, 40.0);
			auto& tile3Ds = manager.GetActiveTile3Ds();
			double lat_min, lat_max, lon_min, lon_max;
			if (!tile3Ds.empty()) {
				lat_min = (*tile3Ds.begin()).first.lat;
				lon_min = (*tile3Ds.begin()).first.lon;
				lat_max = (*tile3Ds.rbegin()).first.lat + config.Tile3DSize;
				lon_max = (*tile3Ds.rbegin()).first.lon + config.Tile3DSize;

				std::array<double, 2> min_pos =
					  wgs84::toCartesian({config.ReferencePoint.lat, config.ReferencePoint.lon}, {lat_min, lon_min});
				std::array<double, 2> max_pos =
					  wgs84::toCartesian({config.ReferencePoint.lat, config.ReferencePoint.lon}, {lat_max, lon_max});

				minp[0] = min_pos[0];
				minp[1] = min_pos[1];
				maxp[0] = max_pos[0];
				maxp[1] = max_pos[1];
				cgv::dbox3 box(minp, maxp);
				camera->set_scene_extent(box);
			}
		}

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
			cgv::math::fvec<double, 3> camera_pos = {cam_pos[0], cam_pos[1], cam_pos[2]};
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
		if (member_ptr == &latitude || member_ptr == &longitude)
		{
			config.ReferencePoint = {latitude, longitude};
			manager.ReInit(latitude, longitude, altitude, &config);
		}

		update_member(member_ptr);
		post_redraw();
	}

	void recenter() 
	{ 
		std::cout << "recentering at (" << latitude << ", " << longitude << ")\n ";

		offset(0, 3) = x;
		offset(1, 3) = y;
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
		add_member_control(this, "Auto Recenter", auto_recenter, "check");
	}
};



//////
//
// Plugins registration
//

// Our maptiles plugin
//cgv::base::factory_registration<maptiles> maptiles_factory("New/Demo/Map Tiles", "", true);
cgv::base::object_registration<maptiles> maptiles_object("");
