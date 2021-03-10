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
		struct CGV_API clod_point_render_style : public group_render_style
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

		class CGV_API clod_point_renderer : public render_types {
		public:
			struct Point {
				vec3 position;
				rgb8 colors;
				uint8_t level = 0; //LOD
			};
			
		private:
			// stores parameters generated for the draw shaders
			struct DrawParameters {
				GLuint  count = 0; //element count
				GLuint  primCount = 1;
				GLuint  first = 0;
				GLuint  baseInstance = 0;
			};

			shader_program reduce_prog;		// writes reduced input buffer to render_buffer (compute shader)
			shader_program draw_prog;		// draws render_buffer (vertex, geometry, fragment shader)

			std::vector<Point> input_buffer_data;
			
			GLuint vertex_array = 0;
			GLuint input_buffer = 0, render_buffer = 0, draw_parameter_buffer = 0;
			const int input_pos = 0, render_pos = 1, drawp_pos = 3;

			bool buffers_outofdate = true;

			/// default render style
			mutable render_style* default_render_style = nullptr;
			/// current render style, can be set by user
			const render_style* rs = nullptr;

		protected:

			void generate_lods_poisson();

			void draw_and_compute_impl(context& ctx, PrimitiveType type, size_t start, size_t count);

			const render_style* get_style_ptr() const;

			template <typename T>
			const T& get_style() const { return *static_cast<const T*>(get_style_ptr()); }
		public:
			clod_point_renderer() = default;

			render_style* create_render_style() const;

			bool init(context& ctx);

			bool enable(context& ctx);
			
			bool disable(context& ctx);

			void clear(const cgv::render::context& ctx);

			/// @param use_strips : unused
			/// @param use_adjacency : unused
			/// @param strip_restart_index : unused
			void draw(context& ctx, size_t start=0, size_t count=0);
			/// @param use_strips : unused
			/// @param use_adjacency : unused
			/// @param strip_restart_index : unused
			bool render(context& ctx, size_t start, size_t count);

			// this method can overwrite and reorder the elements of input_buffer_data
			void generate_lods(const LoDMode mode = LoDMode::RANDOM_POISSON);

			void set_positions(context& ctx, const std::vector<vec3>& positions);

			// set point colors
			template<typename T>
			void set_colors(const context& ctx, const std::vector<T>& colors) {				
				for (int i = 0; i < input_buffer_data.size(); ++i) {
					input_buffer_data[i].colors = rgb8(colors[i]);
				}
				buffers_outofdate = true;
			}

			// add lod information for each point
			template<typename T>
			void set_lods(const std::vector<T>& lod) {
				//input_buffer_data.resize(lod.size());
				for (int i = 0; i < lod.size(); ++i) {
					input_buffer_data[i].level = lod[i]; //set LOD level (lower levels should be more coarse than higher levels)
				}
				buffers_outofdate = true;
			}

			void set_render_style(const render_style& rs);

			uint8_t& point_lod(const int i);
			rgb8& point_color(const int i);
			vec3& point_position(const int i);
		private:
			void add_shader(context& ctx, shader_program& prog, const std::string& sf, const cgv::render::ShaderType st);
			void fill_buffers(context& ctx);
			void clear_buffers(const context& ctx);
		};

		class octree_lod_generator : public render_types {
		public:
			using Vertex = clod_point_renderer::Point;

			struct PointCloud {
				std::vector<Vertex> vertices;

				PointCloud() = default;
				PointCloud(int size) : vertices(size){}
			};

			//lookup table for converting cell indices to linear indices
			struct NodeLUT {
				int64_t grid_size;
				// grid contains index of node in nodes
				std::vector<int> grid;
			};

			//nodes creted by chunking phase
			struct ChunkNode {
				int64_t level = 0;
				int64_t x = 0;
				int64_t y = 0;
				int64_t z = 0;
				int64_t size = 0; //cube edge length
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

			struct Chunks {
				std::vector<ChunkNode> nodes;
				vec3 min, max;
			};

			struct IndexNode {

				std::vector<std::shared_ptr<IndexNode>> children;

				//accepted points, empty if sampled == false
				std::shared_ptr<std::vector<Vertex>> points;
				
				std::vector<rgb8> accumulated_colors;
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

				std::mutex mtx_write;

				Indexer(std::vector<Vertex>* const ptr) {
					output = ptr;
				}

				void write(IndexNode& node) {
					assert(node.sampled);
					std::lock_guard<std::mutex> lock(mtx_write);
					for (auto& vert : *node.points) {
						vert.level = node.level();
						output->push_back(vert);
					}
				}

				void flushChunkRoot(std::shared_ptr<IndexNode> chunkRoot) {

					std::lock_guard<std::mutex> lock(mtx_chunkRoot);

					for (auto& vert : *(chunkRoot->points)) {
						output->push_back(vert);
					}
					
					chunkRoot->points = nullptr;
				}
			};

			struct Sampler {

				Sampler() {}

				virtual void sample(std::shared_ptr<IndexNode> node, double baseSpacing, std::function<void(IndexNode*)> callbackNodeCompleted) = 0;
			};
			
			std::vector<PointCloud> point_clouds;

			static constexpr int max_points_per_index_node = 10'000;
			int max_points_per_chunk;
			int grid_size;
			int currentPass;

			//const std::vector<vec3>* positions;
			//const std::vector<rgba8>* colors;

			Vertex* source_data;
			size_t source_data_size;

		protected:
			std::string to_node_id(int level, int gridSize, int64_t x, int64_t y, int64_t z);
			
			//void lod_chunking(const std::vector<vec3>& positions, const vec3& min, const vec3& max);
			//std::vector<octree_lod_generator::ChunkNode> lod_chunking(const Vertex* vertices, const size_t num_points,const vec3& min, const vec3& max);
			Chunks lod_chunking(const Vertex* vertices, const size_t num_points,const vec3& min, const vec3& max, const float& size);

			std::vector<std::atomic_int32_t> lod_counting(const Vertex* vertices, const int64_t num_points, int64_t grid_size, const vec3& min, const vec3& max, const float& size);
			
			NodeLUT lod_createLUT(std::vector<std::atomic_int32_t>& grid, int64_t grid_size,std::vector<ChunkNode>& nodes);
			
			//create chunk nodes
			void distributePoints(vec3 min, vec3 max, float cube_size, NodeLUT& lut, const Vertex* vertices, const int64_t num_points, const std::vector<ChunkNode>& nodes);
			//inout chunks, out vertices
			void lod_indexing(Chunks& chunks, std::vector<Vertex>& vertices, Sampler& sampler);
			
			void buildHierarchy(Indexer* indexer, IndexNode* node, std::shared_ptr<std::vector<Vertex>> points, int64_t numPoints, int64_t depth = 0);

			static box3 child_bounding_box_of(const vec3& min, const vec3& max, const int index);
			
			int64_t grid_index(const vec3& position, const vec3& min, const float& cube_size, const int& grid_size) const;
			cgv::render::render_types::ivec3 grid_index_vec(const vec3& position, const vec3& min, const float& cube_size, const int& grid_size) const;

		public:
			//lod stored in alpha channel of point color
			//void generate_lods(const std::vector<vec3>& positions, const std::vector<rgba8>& colors);
			std::vector<Vertex> generate_lods(const std::vector<Vertex>& vertices);
		};
	}
}
#include <cgv/config/lib_end.h>