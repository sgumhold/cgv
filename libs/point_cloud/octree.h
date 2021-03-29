#pragma once
#include <cgv/render/render_types.h>

#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>

#include "lib_begin.h"

namespace cgv {
	namespace pointcloud {

struct LODPoint : public cgv::render::render_types {
	vec3 position;
	rgb8 color;
	uint8_t level = 0;
};

class CGV_API octree_lod_generator : public cgv::render::render_types {
	public:
		struct PointCloud {
			std::vector<LODPoint> vertices;
			std::atomic_int numPointsWritten = 0;
			PointCloud() = default;
			PointCloud(int size) : vertices(size){}

			//write points in a thread safe way
			void write_points(const LODPoint* points, const int size) {
				int start = numPointsWritten.fetch_add(size);
				assert(vertices.size() - start >= size);
				memcpy(vertices.data()+ start, points, size * sizeof(LODPoint));
			}
		};

		//lookup table used to converting cell indices to linear indices
		struct NodeLUT {
			int64_t grid_size;
			// grid contains index of node in nodes
			std::vector<int> grid;
		};

		//nodes creted by chunking phase
		struct ChunkNode {
			int level = 0;
			int x = 0;
			int y = 0;
			int z = 0;
			int size = 0; //cube edge length
			int numPoints;
			//point cloud data
			std::shared_ptr<PointCloud> pc_data;
			std::string id;

			ChunkNode(std::string node_id,int numPoints) {
				this->numPoints = numPoints;
				this->id = node_id;
				this->pc_data = std::make_shared<PointCloud>(numPoints);
			}
		};

		struct Chunks {
			std::vector<ChunkNode> nodes;
			vec3 min, max;
		};

		struct IndexNode {

			std::vector<std::shared_ptr<IndexNode>> children;

			//accepted points, empty if sampled == false
			std::shared_ptr<std::vector<LODPoint>> points;
				
			//std::vector<rgb8> accumulated_colors;
			vec3 min;
			vec3 max;
			std::string name;

			int64_t index_start = 0;
			//number of accepted points
			int64_t num_points = 0;

			bool sampled = false;

			IndexNode() {}

			IndexNode(const std::string& name,const vec3& min, const vec3& max);

			void traverse_pre(std::function<void(IndexNode*)> callback);

			void traverse_post(std::function<void(IndexNode*)> callback);

			bool is_leaf();

			void add_descendant(std::shared_ptr<IndexNode> descendant);

			int64_t level() {
				return name.size() - 1;
			}
		};

		struct Indexer {
			//pointer to result vector
			std::vector<LODPoint>* output;

			std::shared_ptr<IndexNode> root;

			std::vector<std::shared_ptr<IndexNode>> detachedParts;

			std::atomic_int64_t byteOffset = 0;

			double scale = 0.001;
			double spacing = 1.0;

			std::atomic_int64_t dbg = 0;

			std::mutex mtx_depth;
			int64_t octreeDepth = 0;
				
			std::mutex mtx_chunkRoot;

			std::mutex mtx_write;

			Indexer(std::vector<LODPoint>* const ptr) {
				output = ptr;
			}

			// lock and write node to out
			void write(IndexNode& node, bool unload = false) {
				assert(node.sampled);
				std::lock_guard<std::mutex> lock(mtx_write);
				for (auto& vert : *node.points) {
					vert.level = node.level();
					output->push_back(vert);
				}
				if (unload)
					node.points = nullptr;
			}
		};

		struct Sampler {

			Sampler() {}

			virtual void sample(std::shared_ptr<IndexNode> node, double baseSpacing, std::function<void(IndexNode*)> callbackNodeCompleted) = 0;
		};

		static constexpr int max_points_per_index_node = 10'000;
		int max_points_per_chunk;
		int grid_size;
		int currentPass;

		LODPoint* source_data;
		size_t source_data_size;

	protected:
		std::string to_node_id(int level, int gridSize, int64_t x, int64_t y, int64_t z);
			
		//void lod_chunking(const std::vector<vec3>& positions, const vec3& min, const vec3& max);
		//std::vector<octree_lod_generator::ChunkNode> lod_chunking(const LODPoint* vertices, const size_t num_points,const vec3& min, const vec3& max);
		Chunks lod_chunking(const LODPoint* vertices, const size_t num_points,const vec3& min, const vec3& max, const float& size);

		std::vector<std::atomic_int32_t> lod_counting(const LODPoint* vertices, const int64_t num_points, int64_t grid_size, const vec3& min, const vec3& max, const float& size);
			
		NodeLUT lod_createLUT(std::vector<std::atomic_int32_t>& grid, int64_t grid_size,std::vector<ChunkNode>& nodes);
			
		//create chunk nodes
		void distribute_points(vec3 min, vec3 max, float cube_size, NodeLUT& lut, const LODPoint* vertices, const int64_t num_points, const std::vector<ChunkNode>& nodes);
		//inout chunks, out vertices
		void lod_indexing(Chunks& chunks, std::vector<LODPoint>& vertices, Sampler& sampler);
			
		void build_hierarchy(Indexer* indexer, IndexNode* node, std::shared_ptr<std::vector<LODPoint>> points, int64_t numPoints, int64_t depth = 0);

		static box3 child_bounding_box_of(const vec3& min, const vec3& max, const int index);
			
		int64_t grid_index(const vec3& position, const vec3& min, const float& cube_size, const int& grid_size) const;
		cgv::render::render_types::ivec3 grid_index_vec(const vec3& position, const vec3& min, const float& cube_size, const int& grid_size) const;

		//only needed for debug purposes
		int counthierarchy(IndexNode* node) {
			if (node == nullptr)
				return 0;
			int count = (node->points == nullptr) ? 0 : node->points->size();
			for (auto& child : node->children) {
				count += counthierarchy(child.get());
			}
			return count;
		};

		//only needed for debug purposes
		int count_zeros_hierarchy(IndexNode* node) {
			if (node == nullptr)
				return 0;
			int count = 0;
			if (node->points != nullptr) {
				for (auto& pnt : *node->points)
					if (pnt.position == vec3(0))
						++count;	
			}
			for (auto& child : node->children) {
				count += count_zeros_hierarchy(child.get());
			}
			return count;
		};

	public:
		//lod stored in alpha channel of point color
		std::vector<LODPoint> generate_lods(const std::vector<LODPoint>& vertices);
};

	}
}

#include <cgv/config/lib_end.h>