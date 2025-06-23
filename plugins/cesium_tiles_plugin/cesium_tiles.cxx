#include "cesium_tiles.h"
#include "cesium_tiles/CesiumElevationTile.h"

cesium_tiles::cesium_tiles() : node("Cesium Tiles") {

}

cesium_tiles::~cesium_tiles() {}

bool cesium_tiles::init(cgv::render::context& ctx) 
{
	// initialize the shader program
	std::string elevation_shader_source = "elevation_tile.glpr";
	_elevationShader.build_program(ctx, elevation_shader_source);
	_elevationShader.specify_standard_uniforms(true, false, false, true);
	_elevationShader.specify_standard_vertex_attribute_names(ctx, false, true, true);
	_elevationShader.allow_context_to_set_color(true);

	// initialize the tile manager
	tileManager.Init(ctx, _elevationShader);

	// TODO: Set the camera to the correct position
	// Not sure if the camera is accessible at this point
	_camera = dynamic_cast<cgv::render::stereo_view*>(find_view_as_node());

	if (_camera)
	{
		const double latitude = 51.0504;  // Dresden latitude
		const double longitude = 13.7373; // Dresden longitude
		const double height = 0.0;		  // Height above the ellipsoid in meters
	
		CesiumGeospatial::Ellipsoid ellipsoid = CesiumGeospatial::Ellipsoid::WGS84;
		glm::dvec3 position =
			  ellipsoid.cartographicToCartesian(CesiumGeospatial::Cartographic(longitude, latitude, height));
		glm::dvec3 pos = ellipsoid.cartographicToCartesian(
			  CesiumGeospatial::Cartographic::fromDegrees(longitude, latitude, height));

		_camera->set_focus(pos[0], pos[1], pos[2]);
	}

	return true; 
}

void cesium_tiles::init_frame(cgv::render::context& ctx) {}

void cesium_tiles::clear(cgv::render::context& ctx) {}

void cesium_tiles::draw(cgv::render::context& ctx) 
{ 

	tileManager.Update(ctx);
	std::cout << "Number of tiles: " << tileManager.GetTileCount() << std::endl;

	for (auto& tile : tileManager.GetTiles()) {
		Cesium3DTilesSelection::TileRenderContent* renderContent = tile.getContent().getRenderContent();
		if (!renderContent)
			continue;

		void* pRenderResource = renderContent->getRenderResources();
		if (!pRenderResource)
			continue;

		CesiumElevationTileRender* elevationTile = static_cast<CesiumElevationTileRender*>(pRenderResource);

		if (elevationTile)
			tileRenderer.Draw(ctx, *elevationTile, _elevationShader);
	}
}

std::string cesium_tiles::get_type_name() const 
{ 
	return "cesium_tiles"; 
}

void cesium_tiles::stream_help(std::ostream& os)
{
	os << "Cesium Tiles:\a\n"
	   << "";
}

bool cesium_tiles::handle(cgv::gui::event& e) { return false; }

void cesium_tiles::on_set(void* member_ptr) {}

void cesium_tiles::create_gui() {}
