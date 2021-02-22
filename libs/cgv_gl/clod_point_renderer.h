#pragma once
#include <cgv/render/context.h>
#include <cgv_gl/point_renderer.h>
#include <atomic>
#include <mutex>

#include "gl/lib_begin.h"

// [WIP] clod point cloud renderer

namespace cgv {
	namespace render {

		/** render style for sphere rendere */
		struct CGV_API clod_point_render_style : public point_render_style
		{
			/*@name clod rendering attributes*/
			//@{
			float CLOD = 1.f;
			float spacing = 1.f;
			float scale = 1.f;
			float min_millimeters = 1.f;
			float pointSize = 1.f;
			//@}

			/// construct with default values
			clod_point_render_style();
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};

		enum class LoDMode {
			POTREE = 1,
			RANDOM_POISSON = 2,
			INVALID = -1
		};

		class CGV_API clod_point_renderer : public cgv::render::renderer {
		public:
			struct Vertex {
				vec3 position;
				rgb8 colors;
				uint8_t level = 0;
			};
		private:
			struct DrawParameters {
				GLuint  count = 0; //element count
				GLuint  primCount = 1;
				GLuint  first = 0;
				GLuint  baseInstance = 0;
			};

			shader_program reduce_prog;
			shader_program draw_prog;

			std::vector<Vertex> input_buffer_data;
			
			GLuint vertex_array = 0;
			GLuint input_buffer = 0, render_buffer = 0, draw_parameter_buffer = 0;
			const int input_pos = 0, render_pos = 1, drawp_pos = 3;
			bool buffers_outofdate = true;

		protected:

			void generate_lods_poisson();

			void draw_and_compute_impl(context& ctx, PrimitiveType type, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index);
		public:
			render_style* create_render_style() const;

			bool init(context& ctx);

			bool enable(context& ctx);
			
			bool disable(context& ctx);

			/// @param use_strips : unused
			/// @param use_adjacency : unused
			/// @param strip_restart_index : unused
			void draw(context& ctx, size_t start=0, size_t count=0,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1) override;
			/// @param use_strips : unused
			/// @param use_adjacency : unused
			/// @param strip_restart_index : unused
			bool render(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1) override;

			void generate_lods(const LoDMode mode = LoDMode::RANDOM_POISSON);

			void set_positions(context& ctx, std::vector<vec3> positions) {
				input_buffer_data.resize(positions.size());
				for (int i = 0; i < positions.size(); ++i) {
					input_buffer_data[i].position = positions[i];
				}
				buffers_outofdate = true;
			}
			template<typename T>
			void set_colors(const context& ctx, const std::vector<T>& colors) {				
				for (int i = 0; i < input_buffer_data.size(); ++i) {
					input_buffer_data[i].colors = rgb8(colors[i]);
				}
				buffers_outofdate = true;
			}

			template<typename T>
			void set_lods(const std::vector<T>& lod) {
				//input_buffer_data.resize(lod.size());
				for (int i = 0; i < lod.size(); ++i) {
					input_buffer_data[i].level = lod[i]; //set LOD level (lower levels should be more coarse than higher levels)
				}
				buffers_outofdate = true;
			}

			uint8_t& point_lod(const int i);
			rgb8& point_color(const int i);
			vec3& point_position(const int i);
		private:
			void add_shader(context& ctx, shader_program& prog, const std::string& sf, const cgv::render::ShaderType st);
			void fill_buffers(context& ctx);
			void clear_buffers(context& ctx);
		};

		class octree_lod_generator : public render_types {
		public:
			using Vertex = clod_point_renderer::Vertex;

			struct PointCloud {
				std::vector<Vertex> vertices;

				PointCloud() = default;
				PointCloud(int size) : vertices(size){}
			};

			struct State {
				std::atomic<int64_t> pointsTotal = 0;
				std::atomic<int64_t> pointsProcessed = 0;

				int numPasses = 3;
				int currentPass = 0; // starts with index 1! interval: [1,  numPasses]

				std::mutex mtx;

				double progress() {
					return double(pointsProcessed) / double(pointsTotal);
				}
			};

			//lookup table for converting cell indexes to chunk
			struct NodeLUT {
				int64_t grid_size;
				// grid contains index of node in nodes
				std::vector<int> grid;

				inline int index(const int& ix, const int& iy, const int& iz) {
					return ix + iy * grid_size + iz * grid_size * grid_size;
				}
			};

			//nodes creted by chunking phase
			struct ChunkNode {
				int64_t level = 0;
				int64_t x = 0;
				int64_t y = 0;
				int64_t z = 0;
				int64_t size = 0;
				int64_t numPoints;
				//point cloud data
				std::shared_ptr<PointCloud> pc_data;
				std::string id;

				ChunkNode(std::string node_id,int numPoints) {
					this->numPoints = numPoints;
					this->id = node_id;
					this->pc_data = std::make_shared<PointCloud>();
				}
			};

			struct IndexNode {

				std::vector<std::shared_ptr<IndexNode>> children;

				std::shared_ptr<std::vector<Vertex>> points;
				std::vector<rgb8> accumulated_colors;
				vec3 min;
				vec3 max;
				std::string name;

				int64_t index_start = 0;
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
				std::vector<Vertex>* output;

				std::shared_ptr<IndexNode> root;

				std::vector<std::shared_ptr<IndexNode>> detachedParts;

				std::atomic_int64_t byteOffset = 0;

				double scale = 0.001;
				double spacing = 1.0;

				std::atomic_int64_t dbg = 0;

				std::mutex mtx_depth;
				int64_t octreeDepth = 0;
				
				std::mutex mtx_chunkRoot;

				Indexer(std::vector<Vertex>* const ptr) {
					output = ptr;
				}
			};

			struct Sampler {

				Sampler() {}

				virtual void sample(std::shared_ptr<IndexNode> node, double baseSpacing, std::function<void(IndexNode*)> callbackNodeCompleted) = 0;
			};

			// stores chunks created by the chunking phase
			std::vector<ChunkNode> nodes;

			std::vector<PointCloud> point_clouds;

			int max_points_per_chunk = 5'000'000;
			int grid_size;
			int currentPass;

			//const std::vector<vec3>* positions;
			//const std::vector<rgba8>* colors;

			Vertex* source_data;
			size_t source_data_size;

		protected:
			std::string to_node_id(int level, int gridSize, int64_t x, int64_t y, int64_t z);
			
			//void lod_chunking(const std::vector<vec3>& positions, const vec3& min, const vec3& max);
			void lod_chunking(const Vertex* vertices, const size_t num_points,const vec3& min, const vec3& max);

			std::vector<std::atomic_int32_t> lod_counting(const Vertex* vertices, const int64_t num_points, int64_t grid_size, const vec3& min, const vec3& max);
			
			NodeLUT lod_createLUT(std::vector<std::atomic_int32_t>& grid, int64_t grid_size);
			
			//create chunk nodes
			void distributePoints(vec3 min, vec3 max, NodeLUT& lut, const Vertex* vertices, const int64_t num_points);
			//in chunks, out vertices
			void lod_indexing(const std::vector<ChunkNode>& chunks, std::vector<Vertex>& vertices);

			void buildHierarchy(Indexer* indexer, IndexNode* node, std::shared_ptr<std::vector<Vertex>> points, int64_t numPoints, int64_t depth = 0);

			static box3 child_bounding_box_of(const vec3& min, const vec3& max, const int index);

		public:
			//lod stored in alpha channel of point color
			//void generate_lods(const std::vector<vec3>& positions, const std::vector<rgba8>& colors);
			std::vector<Vertex> generate_lods(const std::vector<Vertex>& vertices);
		};
	}
}
#include <cgv/config/lib_end.h>