#pragma once

#include "library.h"

#include <cgv/render/context.h>

#include <Cesium3DTilesSelection/Tileset.h>
#include <Cesium3DTilesSelection/LoadedTileEnumerator.h>

const int TERRAIN_ASSET_ID = 1; // cesium-world-terrain
const int IMAGE_ASSET_ID = 2;	// bing-satellite-imagery

class CESIUM_TILES_API TileManager
{
public:
	TileManager();
	~TileManager();

	void Init(cgv::render::context& ctx, cgv::render::shader_program& elevationShader);

	// Update the tileset with the current context
	// Uses the modelview and projection matrices from the context to create a ViewState that represents the current
	// view.
	void Update(cgv::render::context& ctx);

	// Get the loaded tiles from the tileset
	Cesium3DTilesSelection::LoadedTileEnumerator GetTiles();

	// Get the number of loaded tiles
	int GetTileCount() const;

private:
	// TODO: Remove hardcoded token 
	const std::string TOKEN = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
							  "eyJqdGkiOiI2NThhMGE1NC0wMDcxLTQ2MGEtODBkNy0wZjEyYTNiODRhYjYiLCJpZCI6Mjg3ODYwLCJpYXQiOjE3"
							  "NDc2MzgzMTV9.u9j-fqomKrBqpUCVtN121onjUBomQfG7-jFTC1aQfBg";
	
	std::unique_ptr<Cesium3DTilesSelection::Tileset> _tileset;
};
