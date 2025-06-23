#include "TileManager.h"

#include <iostream>

#include "SimpleTaskProcessor.h"
#include "SimplePrepareRenderResources.h"

#include <CesiumCurl/CurlAssetAccessor.h>
#include <CesiumAsync/GunzipAssetAccessor.h>
#include <CesiumUtility/CreditSystem.h>
#include <CesiumRasterOverlays/IonRasterOverlay.h>
#include <CesiumRasterOverlays/RasterOverlay.h>

TileManager::TileManager()
{ 
	std::cout << "TileManager initialized." << std::endl; 
}

TileManager::~TileManager() {}

void TileManager::Init(cgv::render::context& ctx, cgv::render::shader_program& elevationShader)
{ 
	auto taskProcessor = std::make_shared<SimpleTaskProcessor>();
	auto assetAccesor = std::make_shared<CesiumCurl::CurlAssetAccessor>();
	auto gunzipAssetAccessor = std::make_shared<CesiumAsync::GunzipAssetAccessor>(assetAccesor);
	auto prepareRendererResources = std::make_shared<SimplePrepareRenderResources>(ctx, elevationShader);
	auto creditSystem = std::make_shared<CesiumUtility::CreditSystem>();

	CesiumAsync::AsyncSystem asyncSystem(taskProcessor);

	Cesium3DTilesSelection::TilesetExternals externals{gunzipAssetAccessor, prepareRendererResources, asyncSystem,
													   creditSystem};

	_tileset = std::make_unique<Cesium3DTilesSelection::Tileset>(externals, TERRAIN_ASSET_ID, TOKEN);

	CesiumRasterOverlays::RasterOverlay* satelliteImagery =
		  new CesiumRasterOverlays::IonRasterOverlay("BING", IMAGE_ASSET_ID, TOKEN);

	_tileset->getOverlays().add(satelliteImagery);
}

void TileManager::Update(cgv::render::context& ctx)
{
	glm::dmat4 modelviewMatrix, projectionMatrix;

	auto ctxModelViewMatrix = ctx.get_modelview_matrix();
	auto ctxProjectionMatrix = ctx.get_projection_matrix();

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++) {
			modelviewMatrix[i][j] = ctxModelViewMatrix(i, j);
			projectionMatrix[i][j] = ctxProjectionMatrix(i, j);
		}
	}

	/* Uncomment once the camera is set to the correct position
	//TODO: Check if viewState is actually correct
	Cesium3DTilesSelection::ViewState viewState(
		modelviewMatrix,
		projectionMatrix,
		{ctx.get_width(), ctx.get_height()},
		_tileset->getEllipsoid());

	_tileset->updateView({viewState});
	*/


	// For now, we use a dummy ViewState pretending we are in Dresden, Germany
	const double latitude = 51.0504; // Dresden latitude
	const double longitude = 13.7373; // Dresden longitude
	const double height = 0.0;		  // Height above the ellipsoid in meters

	CesiumGeospatial::Ellipsoid ellipsoid = _tileset->getEllipsoid();
	glm::dvec3 pos =
		  ellipsoid.cartographicToCartesian(CesiumGeospatial::Cartographic::fromDegrees(longitude, latitude, height));
	glm::dvec3 direction = {0.0, 0.0, -1.0}; // Looking down the negative Z-axis
	glm::dvec3 up = {0.0, 1.0, 0.0};		 // Up vector
	glm::dvec2 viewportSize = {ctx.get_width(), ctx.get_height()};
	double horizontalFOV = 0.785398; // 45 degrees in radians
	double verticalFOV = 0.785398;	 // 45 degrees in radians


	Cesium3DTilesSelection::ViewState dummyState(pos,
												 direction, // Looking down the negative Z-axis
												 up,		// Up vector
												 viewportSize, horizontalFOV, verticalFOV );
	_tileset->updateView({dummyState});
}

Cesium3DTilesSelection::LoadedTileEnumerator TileManager::GetTiles()
{
	return _tileset->loadedTiles(); }

int TileManager::GetTileCount() const 
{ 
	return _tileset->getNumberOfTilesLoaded(); 
}
