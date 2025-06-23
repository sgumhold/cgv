#pragma once

#include "library.h"

#include <Cesium3DTilesSelection/IPrepareRendererResources.h>
#include <future>
#include <iostream>
#include "CesiumElevationTile.h"

// A simple implementation of IPrepareRendererResources that does nothing.
class CESIUM_TILES_API SimplePrepareRenderResources : public Cesium3DTilesSelection::IPrepareRendererResources
{
  public:
	SimplePrepareRenderResources(cgv::render::context& ctx, cgv::render::shader_program& elevationShader)
	{
		_ctx = &ctx;
		_elevationShader = &elevationShader;
	}

	
	virtual CesiumAsync::Future<Cesium3DTilesSelection::TileLoadResultAndRenderResources>
		prepareInLoadThread(
			const CesiumAsync::AsyncSystem& asyncSystem,
			Cesium3DTilesSelection::TileLoadResult&& tileLoadResult,
			const glm::dmat4& transform,
			const std::any& rendererOptions) override {
		//std::cout << "Prepare in Load Thread called.\n";
		
		// Create a CesiumElevationTileData object to hold the extracted data
		auto elevationTileData = std::make_unique<CesiumElevationTileData>();
		
		// Check if the contentKind is a CesiumGltf::Model
		if (std::holds_alternative<CesiumGltf::Model>(tileLoadResult.contentKind)) {
			const CesiumGltf::Model& model = std::get<CesiumGltf::Model>(tileLoadResult.contentKind);

			// Iterate over all meshes and primitives
			for (const auto& mesh : model.meshes) {
				for (const auto& primitive : mesh.primitives) {
					// TODO: Check if extraction is indeed correct
					// Find the POSITION attribute
					auto posIt = primitive.attributes.find("POSITION");
					if (posIt != primitive.attributes.end()) {
						int32_t accessorIndex = posIt->second;
						if (accessorIndex >= 0 && static_cast<size_t>(accessorIndex) < model.accessors.size()) {
							const auto& accessor = model.accessors[accessorIndex];

							// Get the bufferView for this accessor
							if (accessor.bufferView >= 0 && static_cast<size_t>(accessor.bufferView) < model.bufferViews.size()) {
								const auto& bufferView = model.bufferViews[accessor.bufferView];
								if (bufferView.buffer >= 0 && static_cast<size_t>(bufferView.buffer) < model.buffers.size()) {
									const auto& buffer = model.buffers[bufferView.buffer];

									// Calculate the start of the data
									size_t byteOffset = size_t(bufferView.byteOffset) + size_t(accessor.byteOffset);
									size_t stride = bufferView.byteStride.value_or(sizeof(float) * 3);
									size_t count = size_t(accessor.count);

									// Extract positions (assuming VEC3, float)
									for (size_t i = 0; i < count; ++i) {
										size_t offset = byteOffset + i * stride;
										if (offset + sizeof(float) * 3 <= buffer.cesium.data.size()) {
											const float* pos = reinterpret_cast<const float*>(&buffer.cesium.data[offset]);
											elevationTileData->_positions.emplace_back(pos[0], pos[1], pos[2]);
										}
									}
								}
							}
						}
					}

					// Find the Normal attribute
					posIt = primitive.attributes.find("NORMAL");
					if (posIt != primitive.attributes.end()) {
						int32_t accessorIndex = posIt->second;
						if (accessorIndex >= 0 && static_cast<size_t>(accessorIndex) < model.accessors.size()) {
							const auto& accessor = model.accessors[accessorIndex];

							// Get the bufferView for this accessor
							if (accessor.bufferView >= 0 &&
								static_cast<size_t>(accessor.bufferView) < model.bufferViews.size())
							{
								const auto& bufferView = model.bufferViews[accessor.bufferView];
								if (bufferView.buffer >= 0 &&
									static_cast<size_t>(bufferView.buffer) < model.buffers.size())
								{
									const auto& buffer = model.buffers[bufferView.buffer];

									// Calculate the start of the data
									size_t byteOffset = size_t(bufferView.byteOffset) + size_t(accessor.byteOffset);
									size_t stride = bufferView.byteStride.value_or(sizeof(float) * 3);
									size_t count = size_t(accessor.count);

									// Extract positions (assuming VEC3, float)
									for (size_t i = 0; i < count; ++i) {
										size_t offset = byteOffset + i * stride;
										if (offset + sizeof(float) * 3 <= buffer.cesium.data.size()) {
											const float* normal =
												  reinterpret_cast<const float*>(&buffer.cesium.data[offset]);
											elevationTileData->_normals.emplace_back(normal[0], normal[1], normal[2]);
										}
									}
								}
							}
						}
					}

					// Find the Texcoords attribute
					posIt = primitive.attributes.find("_CESIUMOVERLAY_0");
					if (posIt != primitive.attributes.end()) {
						int32_t accessorIndex = posIt->second;
						if (accessorIndex >= 0 && static_cast<size_t>(accessorIndex) < model.accessors.size()) {
							const auto& accessor = model.accessors[accessorIndex];

							// Get the bufferView for this accessor
							if (accessor.bufferView >= 0 &&
								static_cast<size_t>(accessor.bufferView) < model.bufferViews.size())
							{
								const auto& bufferView = model.bufferViews[accessor.bufferView];
								if (bufferView.buffer >= 0 &&
									static_cast<size_t>(bufferView.buffer) < model.buffers.size())
								{
									const auto& buffer = model.buffers[bufferView.buffer];

									// Calculate the start of the data
									size_t byteOffset = size_t(bufferView.byteOffset) + size_t(accessor.byteOffset);
									size_t stride = bufferView.byteStride.value_or(sizeof(float) * 2);
									size_t count = size_t(accessor.count);

									// Extract positions (assuming VEC2, float)
									for (size_t i = 0; i < count; ++i) {
										size_t offset = byteOffset + i * stride;
										if (offset + sizeof(float) * 2 <= buffer.cesium.data.size()) {
											const float* texcoord =
												  reinterpret_cast<const float*>(&buffer.cesium.data[offset]);
											elevationTileData->_texCoords.emplace_back(texcoord[0], texcoord[1]);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	
		// TODO: DO we need to transform the positions with the 'transform' matrix?

		// Return the result, passing ownership of ElevationTileData as a void*
		return asyncSystem.createResolvedFuture<Cesium3DTilesSelection::TileLoadResultAndRenderResources>(
			Cesium3DTilesSelection::TileLoadResultAndRenderResources{
				std::move(tileLoadResult), static_cast<void*>(elevationTileData.release())
			}
		);
	}

	virtual void* prepareInMainThread(Cesium3DTilesSelection::Tile& tile, void* pLoadThreadResult) override {
		//std::cout << "Prepare in Main Thread called.\n";
		
		// retrieve the loaded data from the prepareInLoadThread method
		CesiumElevationTileData* elevationTileData = static_cast<CesiumElevationTileData*>(pLoadThreadResult);
		if (!elevationTileData) {
			std::cerr << "Error: CesiumElavationTileData is null in prepareInMainThread.\n";
			return nullptr;
		}
		CesiumElevationTileRender* elevationTileRender = nullptr;

		if (elevationTileData) {
			// Create a CesiumElevationTileRender object from the loaded data
			elevationTileRender = new CesiumElevationTileRender(*_ctx, *elevationTileData, *_elevationShader);
		}

		if (elevationTileRender == nullptr)
			std::cout << "Error: CesiumElevationTileRender is null in prepareInMainThread.\n";
		return elevationTileRender;
	}

	virtual void free(
		Cesium3DTilesSelection::Tile& tile,
		void* pLoadThreadResult,
		void* pMainThreadResult) noexcept override {
		//std::cout << "Free called.\n";
		
		if (pLoadThreadResult)
		{
			auto* pElevationTileData = static_cast<CesiumElevationTileData*>(pLoadThreadResult);
			delete pElevationTileData;
		}

		if (pMainThreadResult) {
			auto* pElevationTileRender = static_cast<CesiumElevationTileRender*>(pLoadThreadResult);
			delete pElevationTileRender;
		}
	}

	virtual void attachRasterInMainThread(
		const Cesium3DTilesSelection::Tile& tile,
		int32_t overlayTextureCoordinateID,
		const CesiumRasterOverlays::RasterOverlayTile& rasterTile,
		void* pMainThreadRendererResources,
		const glm::dvec2& translation,
		const glm::dvec2& scale) override {


		auto& content = tile.getContent();

		// Check if the content is a TileRenderContent
		if (!content.isRenderContent()) {
			return;
		}

		CesiumElevationTileRender* pElevationTile =
			  static_cast<CesiumElevationTileRender*>(content.getRenderContent()->getRenderResources());
		if (!pElevationTile)
		{
			//std::cerr << "Error: ElevationTileRender is null in attachRasterInMainThread.\n";
			return;
		}

		auto image = rasterTile.getImage();
		if (!image)
		{
			//std::cerr << "Error: Image is null in attachRasterInMainThread.\n";
			return;
		}

		//std::cout << "Attaching raster overlay to tile \n";
		const unsigned char* buffer = reinterpret_cast<const unsigned char*>(image->pixelData.data());
		int width = image->width;
		int height = image->height;

		// Create a texture from the image data
		pElevationTile->SetTexture(buffer, height, width);
	}

	// Override the detachRasterInMainThread method to do nothing
	virtual void detachRasterInMainThread(
		const  Cesium3DTilesSelection::Tile& tile,
		int32_t overlayTextureCoordinateID,
		const CesiumRasterOverlays::RasterOverlayTile& rasterTile,
		void* pMainThreadRendererResources) noexcept override {
		//std::cout << "Detach Raster in Main Thread called.\n";
		// No detachment needed
	}

	virtual void* prepareRasterInLoadThread(
		CesiumGltf::ImageAsset& image,
		const std::any& rendererOptions) override {
		//std::cout << "Prepare Raster in Load Thread called.\n";
		return nullptr;
	}

	virtual void* prepareRasterInMainThread(
		CesiumRasterOverlays::RasterOverlayTile& rasterTile,
		void* pLoadThreadResult) override {
		//std::cout << "Prepare Raster in Main Thread called.\n";
		return nullptr;
	}

	virtual void freeRaster(
		const CesiumRasterOverlays::RasterOverlayTile& rasterTile,
		void* pLoadThreadResult,
		void* pMainThreadResult) noexcept override {
		//std::cout << "Free Raster in called.\n";
	}

private:
	cgv::render::context* _ctx;
	cgv::render::shader_program* _elevationShader = nullptr;
};