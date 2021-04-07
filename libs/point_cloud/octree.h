#pragma once
#include <cgv/render/render_types.h>

#include <cstdint>
#include <vector>
#include <array>
#include <memory>
#include <mutex>
#include <tuple>
#include <thread>
#include <random>
#include <chrono>
#include <unordered_map>
#include <sstream>

#include "concurrency.h"
#include "morton.h"
#include "lib_begin.h"

namespace cgv {
namespace pointcloud {
		namespace octree {

	/// a tuple with some extra methods 
	/// the template parameters POSITION,COLOR and LOD give the location of the respective attributes in the tuple
	/// Attribs are the types of the points attributes
	/// there has to be at least one vec3, rgb8 and uint8_t in Attribs refered to by POSITION,COLOR and LOD
	template <size_t POSITION, size_t LOD,typename... Attribs>
	struct GenericLODPoint : public cgv::render::render_types {
		std::tuple<Attribs...> data;

		inline vec3& position() {
			return std::get<POSITION>(data);
		}
		inline uint8_t& level() {
			return std::get<LOD>(data);
		}
		inline const vec3& position() const {
			return std::get<POSITION>(data);
		}
		inline const uint8_t& level() const {
			return std::get<LOD>(data);
		}
	};

	// LOD point with color as additional attribute
	struct SimpleLODPoint : public GenericLODPoint<0, 2, cgv::render::render_types::vec3, cgv::render::render_types::rgb8, uint8_t> {
		inline rgb8& color() {
			return std::get<1>(data);
		}
		inline const rgb8& color() const {
			return std::get<1>(data);
		}
	};

	using LODPoint = GenericLODPoint<0, 2, cgv::render::render_types::vec3,cgv::render::render_types::rgb8,uint8_t>;

	//lookup table used to converting cell indices to linear indices
	struct NodeLUT {
		int64_t grid_size;
		// grid contains index of node in nodes
		std::vector<int> grid;
	};

	template <typename point_t>
	struct IndexNode : public cgv::render::render_types{

		std::array<std::shared_ptr<IndexNode>, 8> children;
		//std::vector<std::shared_ptr<IndexNode>> children;

		//accepted points, empty if sampled == false
		std::shared_ptr<std::vector<point_t>> points;

		//std::vector<rgb8> accumulated_colors;
		vec3 min;
		vec3 max;
		std::string name;

		int64_t index_start = 0;
		//number of accepted points
		int64_t num_points = 0;

		bool sampled = false;

		IndexNode() {}

		IndexNode(const std::string& name, const vec3& min, const vec3& max);

		void traverse_pre(std::function<void(IndexNode*)> callback);

		void traverse_post(std::function<void(IndexNode*)> callback);

		bool is_leaf();

		void add_descendant(std::shared_ptr<IndexNode> descendant);

		int64_t level() {
			return name.size() - 1;
		}
	};

	template <typename point_t>
	struct ChunkPointCloud {
		std::vector<point_t> vertices;
		std::atomic_int numPointsWritten = 0;
		ChunkPointCloud() = default;
		ChunkPointCloud(int size) : vertices(size) {}

		//write points in a thread safe way
		void write_points(const point_t* points, const int size) {
			int start = numPointsWritten.fetch_add(size);
			assert(vertices.size() - start >= size);
			memcpy(vertices.data() + start, points, size * sizeof(point_t));
		}
	};

	//nodes creted in chunking phase
	template <typename point_t>
	struct ChunkNode {
		int level = 0;
		int x = 0;
		int y = 0;
		int z = 0;
		int size = 0; //cube edge length
		int numPoints;
		//point cloud data
		std::shared_ptr<ChunkPointCloud<point_t>> pc_data;
		std::string id;

		ChunkNode(std::string node_id, int numPoints) {
			this->numPoints = numPoints;
			this->id = node_id;
			this->pc_data = std::make_shared<ChunkPointCloud<point_t>>(numPoints);
		}

	};

	template <typename point_t>
	struct Chunks : public cgv::render::render_types{
		std::vector<ChunkNode<point_t>> nodes;
		vec3 min, max;
	};

	// sampler for bottom up sampling
	template <typename point_t>
	struct Sampler {

		Sampler() {}

		virtual void sample(std::shared_ptr<IndexNode<point_t>> node, double baseSpacing, std::function<void(IndexNode<point_t>*)> callbackNodeCompleted) = 0;
	};

/// generates octree based lods for point clouds, 
/// @param type point_t should provide two position() and level() methods like GenericLODPoint, these are used to read the point position and write the LOD
/// implementation based on https://github.com/potree/PotreeConverter which is avaiable under a BSD 2 clause License
template <typename point_t>
class octree_lod_generator : public cgv::render::render_types {
	public:

		struct Indexer {
			//pointer to result vector
			std::shared_ptr<IndexNode<point_t>> root;

			double scale = 0.001;
			double spacing = 1.0;

			std::mutex mtx_depth;
			int octreeDepth = 0;

			/// can be used to write a sampled node to an output (see flat indexer)
			/// finished nodes will not be modified by the indexer anymore
			virtual void finish_node(IndexNode<point_t>& node) {
				assert(node.sampled);
			};

			// returns the octrees root node
			std::shared_ptr<IndexNode<point_t>> get_root() {
				return root;
			}
		};

		struct FlatIndexer : public Indexer {
			std::vector<point_t>* output;
			std::mutex mtx_write;
			FlatIndexer(std::vector<point_t>* const ptr) {
				output = ptr;
			}

			// lock and write node to out
			void finish_node(IndexNode<point_t>& node) override {
				assert(node.sampled);
				std::lock_guard<std::mutex> lock(mtx_write);
				for (auto& vert : *node.points) {
					vert.level() = node.level();
					output->push_back(vert);
				}
				node.points = nullptr;
			}
		};

		static constexpr int max_points_per_index_node = 10'000;
		int max_points_per_chunk = -1;

	protected:
		inline std::string to_node_id(int level, int gridSize, int64_t x, int64_t y, int64_t z);

		inline int select_grid_size(const int64_t num_points) {
			int grid_size;
			if (num_points < 4'000'000) {
				grid_size = 32;
			}
			else if (num_points < 20'000'000) {
				grid_size = 64;
			}
			else if (num_points < 100'000'000) {
				grid_size = 128;
			}
			else if (num_points < 500'000'000) {
				grid_size = 256;
			}
			else {
				grid_size = 512;
			}
			return grid_size;
		}

		inline Chunks<point_t> lod_chunking(const point_t* vertices, const size_t num_points,const vec3& min, const vec3& max, const float& size);

		inline std::vector<std::atomic_int32_t> lod_counting(const point_t* vertices, const int64_t num_points, int64_t grid_size, const vec3& min, const vec3& max, const float& size);
			
		inline std::vector<std::atomic_int32_t> lod_counting(const vec3* positions, const int64_t num_points, int64_t grid_size, const vec3& min, const vec3& max, const float& cube_size);

		inline void lod_counting_core(std::function<void(int64_t first_point, int64_t num_points)>& processor, const int64_t num_points);


		inline NodeLUT lod_createLUT(std::vector<std::atomic_int32_t>& grid, int64_t grid_size,std::vector<ChunkNode<point_t>>& nodes);
			
		//create chunk nodes
		inline void distribute_points(vec3 min, vec3 max, float cube_size, int64_t grid_size, NodeLUT& lut, const point_t* vertices, const int64_t num_points, const std::vector<ChunkNode<point_t>>& nodes);
		//inout chunks
		inline void indexing(Chunks<point_t>& chunks, Indexer& indexer, Sampler<point_t>& sampler);
			
		inline void build_hierarchy(Indexer* indexer, IndexNode<point_t>* node, std::shared_ptr<std::vector<point_t>> points, int64_t numPoints, int64_t depth = 0);
			
		inline int64_t grid_index(const vec3& position, const vec3& min, const float& cube_size, const int& grid_size) const;
		inline cgv::render::render_types::ivec3 grid_index_vec(const vec3& position, const vec3& min, const float& cube_size, const int& grid_size) const;

	public:
		inline static box3 child_bounding_box_of(const vec3& min, const vec3& max, const int index);

		/// generate points with lod information out of the given vertices
		inline std::vector<point_t> generate_lods(const std::vector<point_t>& points);

		//creates a octree structure out of IndexNodes and returns a shared pointer to the root
		inline std::shared_ptr<IndexNode<point_t>> build_octree(const std::vector<point_t>& points);
		
		octree_lod_generator(){
			//create a thread pool
			pool_ptr = std::make_unique<cgv::pointcloud::utility::WorkerPool>((std::thread::hardware_concurrency() - 1));
		}

	private:
		std::unique_ptr<cgv::pointcloud::utility::WorkerPool> pool_ptr;

};

// sampler
template <typename point_t>
struct SamplerRandom : public Sampler<point_t> {
	//using point_t = LODPoint;
	template <int GRID_SIZE>
	struct SampleGrid {
		static constexpr int64_t grid_size = GRID_SIZE;
		std::vector<int64_t> grid;
		int64_t iteration = 0;
		SampleGrid() : grid(grid_size* grid_size* grid_size, -1), iteration(0) {}
	};

	// subsample a octree from bottom up, calls onNodeCompleted on every node except the root
	inline void sample(std::shared_ptr<IndexNode<point_t>> node, double baseSpacing, std::function<void(IndexNode<point_t>*)> onNodeCompleted) {
		//using IndexNode = octree_lod_generator::IndexNode;
		using vec3 = cgv::render::render_types::vec3;

		//lambda for bottom up traversal
		std::function<void(IndexNode<point_t>*, std::function<void(IndexNode<point_t>*)>)> traversePost = [&traversePost](IndexNode<point_t>* node, std::function<void(IndexNode<point_t>*)> callback) {
			for (auto child : node->children) {

				if (child != nullptr && !child->sampled) {
					traversePost(child.get(), callback);
				}
			}

			callback(node);
		};

		SampleGrid<128> cgrid;

		traversePost(node.get(), [this,baseSpacing, &onNodeCompleted, &cgrid](IndexNode<point_t>* node) {
			assert((node == nullptr) || (node->num_points == 0 && node->points.get() == nullptr) || (node->num_points == node->points->size()));

			node->sampled = true;

			int64_t gridSize = cgrid.grid_size;
			std::vector<int64_t>& grid = cgrid.grid;
			int64_t& iteration = cgrid.iteration;
			iteration++;

			auto max = node->max;
			auto min = node->min;
			auto size = max - min;

			struct CellIndex {
				int64_t index = -1;
				double distance = 0.0;
			};

			auto toCellIndex = [min, size, gridSize](vec3 point) -> CellIndex {

				double nx = (point.x() - min.x()) / size.x();
				double ny = (point.y() - min.y()) / size.y();
				double nz = (point.z() - min.z()) / size.z();

				double lx = 2.0 * fmod(double(gridSize) * nx, 1.0) - 1.0;
				double ly = 2.0 * fmod(double(gridSize) * ny, 1.0) - 1.0;
				double lz = 2.0 * fmod(double(gridSize) * nz, 1.0) - 1.0;

				double distance = sqrt(lx * lx + ly * ly + lz * lz);

				int64_t x = double(gridSize) * nx;
				int64_t y = double(gridSize) * ny;
				int64_t z = double(gridSize) * nz;

				x = std::max(int64_t(0), std::min(x, gridSize - 1));
				y = std::max(int64_t(0), std::min(y, gridSize - 1));
				z = std::max(int64_t(0), std::min(z, gridSize - 1));

				int64_t index = x + y * gridSize + z * gridSize * gridSize;

				return { index, distance };
			};

			bool isLeaf = node->is_leaf();
			if (isLeaf) {
				std::vector<int> indices(node->num_points);
				for (int i = 0; i < node->num_points; i++) {
					indices[i] = i;
				}

				unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
				//shuffle leafs
				std::shuffle(indices.begin(), indices.end(), std::default_random_engine(seed));

				auto buffer = std::make_shared<std::vector<point_t>>(node->num_points);

				for (int i = 0; i < node->num_points; i++) {
					buffer->at(indices[i]) = node->points->at(i);
				}

				node->points = buffer;

				return false;
			}

			// =================================================================
			// SAMPLING
			// =================================================================
			//
			// first, check for each point whether it's accepted or rejected
			// save result in an array with one element for each point

			std::vector<std::vector<int8_t>> acceptedChildPointFlags;
			std::vector<int64_t> numRejectedPerChild;
			int64_t numAccepted = 0;
			for (int childIndex = 0; childIndex < 8; childIndex++) {
				auto child = node->children[childIndex];

				if (child == nullptr) {
					acceptedChildPointFlags.push_back({});
					numRejectedPerChild.push_back({});

					continue;
				}

				assert((child->points == nullptr && child->num_points == 0) || (child->num_points == child->points->size()));

				std::vector<int8_t> acceptedFlags(child->num_points, 0);
				int64_t numRejected = 0;

				for (int i = 0; i < child->num_points; i++) {

					int64_t pointOffset = i;
					vec3 pos = child->points->at(pointOffset).position();

					CellIndex cellIndex = toCellIndex(pos);

					auto& gridValue = grid[cellIndex.index];

					const double all = sqrt(3.0);

					bool isAccepted;
					if (child->num_points < 100) {
						isAccepted = true;
					}
					else if (cellIndex.distance < 0.7 * all && gridValue < iteration) {
						isAccepted = true;
					}
					else {
						isAccepted = false;
					}

					if (isAccepted) {
						gridValue = iteration;
						numAccepted++;
					}
					else {
						numRejected++;
					}

					acceptedFlags[i] = isAccepted ? 1 : 0;
				}

				acceptedChildPointFlags.push_back(acceptedFlags);
				numRejectedPerChild.push_back(numRejected);
			}

			auto accepted = std::make_shared<std::vector<point_t>>();
			accepted->reserve(numAccepted);
			for (int childIndex = 0; childIndex < 8; childIndex++) {
				auto child = node->children[childIndex];

				if (child == nullptr) {
					continue;
				}

				auto numRejected = numRejectedPerChild[childIndex];
				auto& acceptedFlags = acceptedChildPointFlags[childIndex];
				auto rejected = std::make_shared<std::vector<point_t>>();
				rejected->reserve(numRejected);

				for (int i = 0; i < child->num_points; i++) {
					auto isAccepted = acceptedFlags[i];

					if (isAccepted) {
						accepted->push_back(child->points->data()[i]);
					}
					else {
						rejected->push_back(child->points->data()[i]);
					}
				}

				if (numRejected == 0) {
					//no points for the child
					node->children[childIndex] = nullptr; //this can be a problem for debugging
				} if (numRejected > 0) {
					child->points = rejected;
					child->num_points = numRejected;

					onNodeCompleted(child.get());
				}
			}

			node->points = accepted;
			node->num_points = numAccepted;

			return true;
			});
	}

};


// octree_lod_generator definitions
	template <typename point_t>
	std::string octree_lod_generator<point_t>::to_node_id(int level, int gridSize, int64_t x, int64_t y, int64_t z) {
		std::string id = "r";

		int currentGridSize = gridSize;
		int lx = x;
		int ly = y;
		int lz = z;

		for (int i = 0; i < level; i++) {

			int index = 0;

			if (lx >= currentGridSize / 2) {
				index = index + 0b100;
				lx = lx - currentGridSize / 2;
			}

			if (ly >= currentGridSize / 2) {
				index = index + 0b010;
				ly = ly - currentGridSize / 2;
			}

			if (lz >= currentGridSize / 2) {
				index = index + 0b001;
				lz = lz - currentGridSize / 2;
			}

			id = id + std::to_string(index);
			currentGridSize = currentGridSize / 2;
		}
		return id;
	}
		
	template <typename point_t>
	Chunks<point_t> octree_lod_generator<point_t>::lod_chunking(const point_t* vertices, const size_t num_points, const vec3& min, const vec3& max, const float& cube_size)
	{
		// stores chunks created by the chunking phase
		Chunks<point_t> chunks;

		max_points_per_chunk = std::min<size_t>(num_points / 20, 10'000'000ll);

		int64_t grid_size = select_grid_size(num_points);

		// COUNT
		auto grid = lod_counting(vertices, num_points, grid_size, min, max, cube_size);
#ifndef NDEBUG
		{ //Test counting
			int points = 0;
			for (const auto& e : grid) {
				points += e;
			}
			assert(points == num_points);
	}
#endif


		{ // DISTIRBUTE

			auto lut = lod_createLUT(grid, grid_size, chunks.nodes);
#ifndef NDEBUG
			{ //Test counting
				bool has_valid_entries = false;
				int first_valid = -1;
				int value = -1;
				for (const auto& e : lut.grid) {
					if (e > -1) {
						has_valid_entries = true;
						value = e;
						break;
					}
				}
				assert(has_valid_entries);
		}
#endif

			distribute_points(min, max, cube_size, grid_size, lut, vertices, num_points, chunks.nodes);
		}

		chunks.min = min;
		chunks.max = max;
		return std::move(chunks);
	}

	template <typename point_t>
	void octree_lod_generator<point_t>::lod_counting_core(std::function<void(int64_t first_point, int64_t num_points)>& processor, const int64_t num_points)
	{
		int64_t points_left = num_points;
		int64_t batch_size = 1'000'000;
		int64_t num_read = 0;

		struct Task {
			int64_t first_point;
			int64_t num_points;
		};

		struct Tasks {
			std::vector<Task> task_pool;
			std::atomic_int next_task = 0;

			std::function<void(int64_t first_point, int64_t num_points)> func;

			void operator()() {
				while (true) {
					Task* task;
					//get task
					{
						int task_id = next_task.fetch_add(1);
						if (task_id >= task_pool.size())
							return;
						task = &task_pool[task_id];
					}
					func(task->first_point, task->num_points);
				}
			}
		} tasks;
		tasks.func = processor;

		//simple thread pool
		while (points_left > 0) {

			int64_t num_to_read;
			if (points_left < batch_size) {
				num_to_read = points_left;
				points_left = 0;
			}
			else {
				num_to_read = batch_size;
				points_left = points_left - batch_size;
			}

			{
				Task t;
				t.first_point = num_read;
				t.num_points = num_to_read;
				tasks.task_pool.push_back(t);
			}
			num_read += batch_size;
		}

		pool_ptr->run([&tasks](int thread_id) {tasks(); });
	}


	template <typename point_t>
	std::vector<std::atomic_int32_t> octree_lod_generator<point_t>::lod_counting(const point_t* vertices, const int64_t num_points, int64_t grid_size, const vec3& min, const vec3& max, const float& cube_size)
	{
		std::vector<std::atomic_int32_t> grid(grid_size * grid_size * grid_size);

		// count points for each cell
		std::function<void(int64_t first_point, int64_t num_points)> processor = [this, &grid, &cube_size, grid_size, vertices, &min, &max](int64_t first_point, int64_t num_points) {
			for (int i = 0; i < num_points; i++) {
				int64_t index = grid_index(vertices[first_point + i].position(), min, cube_size, grid_size);
				grid[index].fetch_add(1, std::memory_order::memory_order_relaxed);
			}
		};

		lod_counting_core(processor, num_points);

		return std::move(grid);
	}

	template <typename point_t>
	std::vector<std::atomic_int32_t> octree_lod_generator<point_t>::lod_counting(const vec3* positions, const int64_t num_points, int64_t grid_size, const vec3& min, const vec3& max, const float& cube_size)
	{
		std::vector<std::atomic_int32_t> grid(grid_size * grid_size * grid_size);

		// count points for each cell
		std::function<void(int64_t first_point, int64_t num_points)> processor = [this, &grid, &cube_size, grid_size, positions, &min, &max](int64_t first_point, int64_t num_points) {
			for (int i = 0; i < num_points; i++) {
				int64_t index = grid_index(positions[first_point + i], min, cube_size, grid_size);
				grid[index].fetch_add(1, std::memory_order::memory_order_relaxed);
			}
		};

		lod_counting_core(processor, num_points);

		return std::move(grid);
	}


	template <typename point_t>
	void octree_lod_generator<point_t>::distribute_points(vec3 min, vec3 max, float cube_size, int64_t grid_size, NodeLUT& lut, const point_t* vertices, const int64_t num_points, const std::vector<ChunkNode<point_t>>& nodes)
	{
		//auto start = std::chrono::steady_clock::now();
		auto& grid = lut.grid;

		constexpr int max_chunk_size = 512 * (64 / sizeof(point_t));

		struct Task {
			int64_t batch_size;
			int64_t first_point;
		};

		cgv::pointcloud::utility::TaskPool<Task> tasks;

		{
			int max_num_tasks = (num_points / max_chunk_size) + 1;
			tasks.pool.resize(max_num_tasks);
			auto it = tasks.pool.begin();

			int64_t i = 0;
			while (i < num_points) {
				Task& t = *it;
				t.first_point = i;
				int64_t points_left = num_points - i;
				t.batch_size = (points_left < max_chunk_size) ? points_left : max_chunk_size;
				i += t.batch_size;
				++it;
			}
		}

		tasks.func = [this, &tasks, &vertices, &min, &cube_size, &grid_size, &grid, &nodes](Task* task) {
			int batch_size = task->batch_size;
			int64_t first_point = task->first_point;

			const point_t* start = vertices + first_point;
			const point_t* end = start + batch_size;

			//create a bucket for each chunk
			int num_buckets = nodes.size();
			std::vector<std::vector<point_t>> buckets(num_buckets);

			for (const point_t* i = start; i < end; ++i) {
				vec3 p = i->position();
				int idx = grid_index(p, min, cube_size, grid_size);
				buckets[grid[idx]].push_back(*i);
			}

			for (int i = 0; i < num_buckets; ++i) {
				if (buckets[i].size() > 0)
					nodes[i].pc_data->write_points(buckets[i].data(), buckets[i].size());
			}

		};

		pool_ptr->run([&tasks](int id) {tasks(); });

		/* //single thread variant
		for (int i = 0; i < num_points; ++i) {
			vec3 p = vertices[i].position;
			int idx = grid_index(p, min, cube_size, grid_size);

			auto& node = nodes[grid[idx]];
			point_t v = vertices[i];
			node.pc_data->write_points(&v, 1);
		}*/
		auto end = std::chrono::steady_clock::now();
		//std::printf("distributing: %d ns\n", std::chrono::duration_cast<std::chrono::nanoseconds>(end - start));
	}

	template <typename point_t>
	NodeLUT octree_lod_generator<point_t>::lod_createLUT(std::vector<std::atomic_int32_t>& grid, int64_t grid_size, std::vector<ChunkNode<point_t>>& nodes)
	{
		nodes.clear();

		auto for_xyz = [](int64_t gridSize, std::function< void(int64_t, int64_t, int64_t)> callback) {
			for (int x = 0; x < gridSize; x++) {
				for (int y = 0; y < gridSize; y++) {
					for (int z = 0; z < gridSize; z++) {
						callback(x, y, z);
					}
				}
			}
		};

		// atomic vectors are cumbersome, convert the highest level into a regular integer vector first.
		std::vector<int64_t> grid_high;
		grid_high.reserve(grid.size());
		for (auto& value : grid) {
			grid_high.push_back(value);
		}

		int64_t level_max = int64_t(log2(grid_size));

		// - evaluate counting grid in "image pyramid" fashion
		// - merge smaller cells into larger ones
		// - unmergeable cells are resulting chunks; push them to "nodes" array.
		for (int64_t level_low = level_max - 1; level_low >= 0; level_low--) {

			int64_t level_high = level_low + 1;

			int64_t gridSize_high = pow(2, level_high);
			int64_t gridSize_low = pow(2, level_low);

			std::vector<int64_t> grid_low(gridSize_low * gridSize_low * gridSize_low, 0);
			// grid_high

			// loop through all cells of the lower detail target grid, and for each cell through the 8 enclosed cells of the higher level grid
			for_xyz(gridSize_low, [this, &nodes, &grid_low, &grid_high, gridSize_low, gridSize_high, level_low, level_high, level_max](int64_t x, int64_t y, int64_t z) {

				int64_t index_low = x + y * gridSize_low + z * gridSize_low * gridSize_low;

				int64_t sum = 0;
				bool unmergeable = false;

				// loop through the 8 enclosed cells of the higher detailed grid
				for (int64_t j = 0; j < 8; j++) {
					int64_t ox = (j & 0b100) >> 2;
					int64_t oy = (j & 0b010) >> 1;
					int64_t oz = (j & 0b001) >> 0;

					int64_t nx = 2 * x + ox;
					int64_t ny = 2 * y + oy;
					int64_t nz = 2 * z + oz;

					int64_t index_high = nx + ny * gridSize_high + nz * gridSize_high * gridSize_high;

					auto value = grid_high[index_high];

					if (value == -1) {
						unmergeable = true;
					}
					else {
						sum += value;
					}
				}


				if (unmergeable || sum > max_points_per_chunk) {

					// finished chunks
					for (int64_t j = 0; j < 8; j++) {
						int64_t ox = (j & 0b100) >> 2;
						int64_t oy = (j & 0b010) >> 1;
						int64_t oz = (j & 0b001) >> 0;

						int64_t nx = 2 * x + ox;
						int64_t ny = 2 * y + oy;
						int64_t nz = 2 * z + oz;

						int64_t index_high = nx + ny * gridSize_high + nz * gridSize_high * gridSize_high;

						auto value = grid_high[index_high];


						if (value > 0) {
							std::string node_id = to_node_id(level_high, gridSize_high, nx, ny, nz);
							nodes.emplace_back(node_id, value);
							ChunkNode<point_t>& node = nodes.back();

							node.x = nx;
							node.y = ny;
							node.z = nz;
							node.size = pow(2, (level_max - level_high));
						}
					}

					// invalidate the field to show the parent that nothing can be merged with it
					grid_low[index_low] = -1;
				}
				else {
					grid_low[index_low] = sum;
				}

				});

			grid_high = grid_low;
		}

		// - create lookup table
		// - loop through nodes, add pointers to node/chunk for all enclosed cells in LUT.

		std::vector<int32_t> lut(grid_size * grid_size * grid_size, -1);

		for (int i = 0; i < nodes.size(); i++) {
			const auto& node = nodes[i];

			for_xyz(node.size, [&node, &lut, grid_size, i](int64_t ox, int64_t oy, int64_t oz) {
				int64_t x = node.size * node.x + ox;
				int64_t y = node.size * node.y + oy;
				int64_t z = node.size * node.z + oz;
				int64_t index = x + y * grid_size + z * grid_size * grid_size;

				lut[index] = i;
				});
		}

		return { grid_size, lut };
	}

	template <typename point_t>
	cgv::render::render_types::ivec3 octree_lod_generator<point_t>::grid_index_vec(const vec3& position, const vec3& min, const float& cube_size, const int& grid_size) const {
		dvec3 pos = position;
		double dgrid_size = grid_size;
		//normalized grid position
		double ux = (pos[0] - (double)min.x()) / cube_size;
		double uy = (pos[1] - (double)min.y()) / cube_size;
		double uz = (pos[2] - (double)min.z()) / cube_size;

		//debug only
		/*
		bool in_box = ux >= 0.0 && uy >= 0.0 && uz >= 0.0 &&
				ux <= 1.0+std::numeric_limits<double>::epsilon() &&
				uy <= 1.0+std::numeric_limits<double>::epsilon() &&
				uz <= 1.0+std::numeric_limits<double>::epsilon();
		assert(in_box);
		*/

		int ix = int(std::min(dgrid_size * ux, dgrid_size - 1.0));
		int iy = int(std::min(dgrid_size * uy, dgrid_size - 1.0));
		int iz = int(std::min(dgrid_size * uz, dgrid_size - 1.0));

		return ivec3(ix, iy, iz);
	}

	template <typename point_t>
	int64_t octree_lod_generator<point_t>::grid_index(const vec3& position, const vec3& min, const float& cube_size, const int& grid_size) const
	{
		auto v = grid_index_vec(position, min, cube_size, grid_size);
		int64_t index = v.x() + v.y() * grid_size + v.z() * grid_size * grid_size;
		return index;
	}

	template <typename point_t>
	void octree_lod_generator<point_t>::indexing(Chunks<point_t>& chunks, Indexer& indexer, Sampler<point_t>& sampler)
	{
		struct Task {
			ChunkNode<point_t>* chunk = nullptr;
			int id = 0;
			//one chunk per task
			explicit Task(ChunkNode<point_t>& chunk) {
				static int id = 0;
				this->id = id++;
				this->chunk = &chunk;
			}
		};

		struct Tasks {
			std::vector<Task> task_pool;
			std::atomic_int next_task = 0;

			std::function<void(Task*)> func;

			void operator()() {
				while (true) {
					Task* task;
					//get task
					{
						int task_id = next_task.fetch_add(1);
						if (task_id >= task_pool.size())
							return;
						task = &task_pool[task_id];
					}
					func(task);
				}
			}
		} tasks;
		
		std::mutex mtx_nodes;
		std::vector<std::shared_ptr<IndexNode<point_t>>> nodes;

		indexer.root = std::make_shared<IndexNode<point_t>>("r", chunks.min, chunks.max);
		indexer.spacing = (chunks.max - chunks.min).x() / 128.0;

		//builds node hierachy
		tasks.func = [this, &indexer, &sampler, &nodes, &mtx_nodes](Task* task) {
			static constexpr float Infinity = std::numeric_limits<float>::infinity();
			ChunkNode<point_t>* chunk = task->chunk;

			vec3 min(Infinity), max(-Infinity);

			for (auto& v : chunk->pc_data->vertices) {
				min.x() = std::min(min.x(), v.position().x());
				min.y() = std::min(min.y(), v.position().y());
				min.z() = std::min(min.z(), v.position().z());

				max.x() = std::max(max.x(), v.position().x());
				max.y() = std::max(max.y(), v.position().y());
				max.z() = std::max(max.z(), v.position().z());
			}

			auto chunk_root = std::make_shared<IndexNode<point_t>>(chunk->id, min, max);

			//alias vertices
			std::shared_ptr<std::vector<point_t>> points(chunk->pc_data, &(chunk->pc_data->vertices));

			build_hierarchy(&indexer, chunk_root.get(), points, points->size());

			auto onNodeCompleted = [&indexer, &chunk_root](IndexNode<point_t>* node) {
				//write nodes and unload all except the chunk roots
				assert(node != chunk_root.get());
				indexer.finish_node(*node);
			};

			sampler.sample(chunk_root, indexer.spacing, onNodeCompleted);

			//in core method does not need flushing
			//indexer.flushChunkRoot(chunk_root);

			// add chunk root, provided it isn't the root.
			if (chunk_root->name.size() > 1) {
				assert(indexer.root != nullptr);
				indexer.root->add_descendant(chunk_root);
			}

			std::lock_guard<std::mutex> lock(mtx_nodes);

			nodes.push_back(chunk_root);
		};

		//fill task pool
		for (auto& chunk : chunks.nodes) {
			tasks.task_pool.emplace_back(chunk);
		}

		pool_ptr->run([&tasks](int thread_id) {tasks(); });

		if (chunks.nodes.size() == 1) {
			indexer.root = nodes[0];
		}
		else {
			auto onNodeCompleted = [&indexer](IndexNode<point_t>* node) {
				indexer.finish_node(*node);
			};
			sampler.sample(indexer.root, indexer.spacing, onNodeCompleted);
		}
		indexer.finish_node(*indexer.root.get());
	}

	struct NodeCandidate {
		std::string name = "";
		int64_t indexStart = 0;
		int64_t numPoints = 0;
		int64_t level = 0;
		int64_t x = 0;
		int64_t y = 0;
		int64_t z = 0;
	};

	std::vector<std::vector<int64_t>> createSumPyramid(std::vector<int64_t>& grid, int gridSize) {
		int maxLevel = std::log2(gridSize);
		int currentGridSize = gridSize / 2;

		std::vector<std::vector<int64_t>> sumPyramid(maxLevel + 1);
		for (int level = 0; level < maxLevel; level++) {
			auto cells = pow(8, level);
			sumPyramid[level].resize(cells, 0);
		}
		sumPyramid[maxLevel] = grid;

		for (int level = maxLevel - 1; level >= 0; level--) {

			for (int x = 0; x < currentGridSize; x++) {
				for (int y = 0; y < currentGridSize; y++) {
					for (int z = 0; z < currentGridSize; z++) {

						auto index = morton_encode_3d(z, y, x);
						auto index_p1 = morton_encode_3d(2 * z, 2 * y, 2 * x);

						int64_t sum = 0;
						for (int i = 0; i < 8; i++) {
							sum += sumPyramid[level + 1][index_p1 + i];
						}

						sumPyramid[level][index] = sum;

					}
				}
			}

			currentGridSize = currentGridSize / 2;

		}

		return sumPyramid;
	}

	std::vector<NodeCandidate> createNodes(std::vector<std::vector<int64_t>>& pyramid, int64_t maxPointsPerChunk) {

		std::vector<NodeCandidate> nodes;

		std::vector<std::vector<int64_t>> pyramidOffsets;
		for (auto& counters : pyramid) {

			if (counters.size() == 1) {
				pyramidOffsets.push_back({ 0 });
			}
			else {

				std::vector<int64_t> offsets(counters.size(), 0);
				for (int64_t i = 1; i < counters.size(); i++) {
					int64_t offset = offsets[i - 1] + counters[i - 1];

					offsets[i] = offset;
				}

				pyramidOffsets.push_back(offsets);
			}
		}

		// pyramid starts at level 0 -> gridSize = 1
		// 2 levels -> levels 0 and 1 -> maxLevel 1
		auto maxLevel = pyramid.size() - 1;

		NodeCandidate root;
		root.name = "";
		root.level = 0;
		root.x = 0;
		root.y = 0;
		root.z = 0;

		std::vector<NodeCandidate> stack = { root };

		while (!stack.empty()) {

			NodeCandidate candidate = stack.back();
			stack.pop_back();

			auto level = candidate.level;
			auto x = candidate.x;
			auto y = candidate.y;
			auto z = candidate.z;

			auto& grid = pyramid[level];
			auto index = morton_encode_3d(z, y, x);
			int64_t numPoints = grid[index];

			if (level == maxLevel) {
				// don't split further at this time. May be split further in another pass

				if (numPoints > 0) {
					nodes.push_back(candidate);
				}
			}
			else if (numPoints > maxPointsPerChunk) {
				// split (too many points in node)

				for (int i = 0; i < 8; i++) {

					auto index_p1 = morton_encode_3d(2 * z, 2 * y, 2 * x) + i;
					auto count = pyramid[level + 1][index_p1];

					if (count > 0) {
						NodeCandidate child;
						child.level = level + 1;
						child.name = candidate.name + std::to_string(i);
						child.indexStart = pyramidOffsets[level + 1][index_p1];
						child.numPoints = count;
						child.x = 2 * x + ((i & 0b100) >> 2);
						child.y = 2 * y + ((i & 0b010) >> 1);
						child.z = 2 * z + ((i & 0b001) >> 0);

						stack.push_back(child);
					}
				}

			}
			else if (numPoints > 0) {
				// accept (small enough)
				nodes.push_back(candidate);
			}

		}

		return nodes;
	}

	template <typename point_t>
	void octree_lod_generator<point_t>::build_hierarchy(Indexer* indexer, IndexNode<point_t>* node, std::shared_ptr<std::vector<point_t>> points, int64_t numPoints, int64_t depth)
	{
		if (numPoints < max_points_per_index_node) {
			IndexNode<point_t>* realization = node;
			realization->index_start = 0;
			realization->num_points = numPoints;
			realization->points = points;
			return;
		}

		int64_t levels = 5; // = gridSize 32
		int64_t counterGridSize = pow(2, levels);
		std::vector<int64_t> counters(counterGridSize * counterGridSize * counterGridSize, 0);

		auto min = node->min;
		auto max = node->max;
		auto size = max - min;

		auto gridIndexOf = [&points, min, size, counterGridSize](int64_t pointIndex) {
			//float* xyz = reinterpret_cast<float*>(points.get() + pointIndex);
			vec3 xyz = points->at(pointIndex).position();
			double x = xyz[0];
			double y = xyz[1];
			double z = xyz[2];

			int64_t ix = std::floor(double(counterGridSize) * (x - min.x()) / size.x());
			int64_t iy = std::floor(double(counterGridSize) * (y - min.y()) / size.y());
			int64_t iz = std::floor(double(counterGridSize) * (z - min.z()) / size.z());

			ix = std::max(int64_t(0), std::min(ix, counterGridSize - 1));
			iy = std::max(int64_t(0), std::min(iy, counterGridSize - 1));
			iz = std::max(int64_t(0), std::min(iz, counterGridSize - 1));

			return morton_encode_3d(iz, iy, ix);
		};

		// COUNTING
		for (int64_t i = 0; i < numPoints; i++) {
			auto index = gridIndexOf(i);
			counters[index]++;
		}

		{ // DISTRIBUTING - reorder points to follow morton code order
			std::vector<int64_t> offsets(counters.size(), 0);
			for (int64_t i = 1; i < counters.size(); i++) {
				offsets[i] = offsets[i - 1] + counters[i - 1];
			}

			std::vector<point_t> tmp(numPoints);

			for (int64_t i = 0; i < numPoints; i++) {
				auto index = gridIndexOf(i);
				auto targetIndex = offsets[index]++;

				tmp[targetIndex] = (*points)[i];
				memcpy(tmp.data() + targetIndex, points->data() + i, sizeof(point_t));
			}
			memcpy(points->data(), tmp.data(), numPoints * sizeof(point_t));
		}

		auto pyramid = createSumPyramid(counters, counterGridSize);

		auto nodes = createNodes(pyramid, max_points_per_index_node);

		auto expandTo = [node](NodeCandidate& candidate) {

			std::string startName = node->name;
			std::string fullName = startName + candidate.name;

			// e.g. startName: r, fullName: r031
			// start iteration with char at index 1: "0"

			IndexNode<point_t>* currentNode = node;
			for (int64_t i = startName.size(); i < fullName.size(); i++) {
				int64_t index = fullName.at(i) - '0';

				if (currentNode->children[index] == nullptr) {
					auto childBox = child_bounding_box_of(currentNode->min, currentNode->max, index);
					std::string childName = currentNode->name + std::to_string(index);

					std::shared_ptr<IndexNode<point_t>> child = std::make_shared<IndexNode<point_t>>();
					child->min = childBox.get_min_pnt();
					child->max = childBox.get_max_pnt();
					child->name = childName;

					currentNode->children[index] = child;
					currentNode = child.get();
				}
				else {
					currentNode = currentNode->children[index].get();
				}


			}

			return currentNode;
		};

		std::vector<IndexNode<point_t>*> needRefinement;

		int octreeDepth = 0;

		for (NodeCandidate& candidate : nodes) {

			IndexNode<point_t>* realization = expandTo(candidate);
			realization->index_start = candidate.indexStart;
			realization->num_points = candidate.numPoints;

			std::shared_ptr<std::vector<point_t>> buffer = std::make_shared<std::vector<point_t>>(candidate.numPoints); //potential out of memory here
			memcpy(buffer->data(),
				points->data() + candidate.indexStart,
				candidate.numPoints * sizeof(point_t)
			);

			realization->points = buffer;

			if (realization->num_points > max_points_per_index_node) {
				needRefinement.push_back(realization);
			}

			octreeDepth = std::max(octreeDepth, (int) realization->level());
		}

		{
			std::lock_guard<std::mutex> lock(indexer->mtx_depth);

			indexer->octreeDepth = std::max(indexer->octreeDepth, octreeDepth);
		}

		int64_t sanityCheck = 0;
		for (int64_t nodeIndex = 0; nodeIndex < needRefinement.size(); nodeIndex++) {
			auto subject = needRefinement[nodeIndex];
			auto buffer = subject->points;

			if (sanityCheck > needRefinement.size() * 2) {
				//failed to partition point cloud in indexer::build_hierarchy();
			}

			if (subject->num_points == numPoints) {
				// the subsplit has the same number of points than the input -> ERROR

				std::unordered_map<std::string, int> counters;

				for (int64_t i = 0; i < numPoints; i++) {

					int64_t sourceOffset = i;
					vec3 pos = buffer->data()[sourceOffset].position();

					int X = std::floor(pos.x()), Y = std::floor(pos.y()), Z = std::floor(pos.z());
					std::stringstream ss;
					ss << X << ", " << Y << ", " << Z;

					std::string key = ss.str();
					counters[key]++;
				}

				int64_t numPointsInBox = subject->num_points;
				int64_t numUniquePoints = counters.size();
				int64_t numDuplicates = numPointsInBox - numUniquePoints;

				if (numDuplicates < max_points_per_index_node / 2) {
					// few uniques, just unfavouribly distributed points
					// print warning but continue
					/*
					std::stringstream ss;
					ss << "Encountered unfavourable point distribution. Conversion continues anyway because not many duplicates were encountered. ";
					ss << "However, issues may arise. If you find an error, please report it at github. \n";
					ss << "#points in box: " << numPointsInBox << ", #unique points in box: " << numUniquePoints << ", ";
					ss << "min: " << subject->min.toString() << ", max: " << subject->max.toString();
					*/
					//logger::WARN(ss.str());
				}
				else {

					// remove the duplicates, then try again

					std::vector<int64_t> distinct;
					std::unordered_map<std::string, int> handled;

					auto contains = [](auto map, auto key) {
						return map.find(key) != map.end();
					};

					for (int64_t i = 0; i < numPoints; i++) {

						int64_t sourceOffset = i;

						vec3 pos = buffer->data()[sourceOffset].position();

						int32_t X = std::floor(pos.x()), Y = std::floor(pos.y()), Z = std::floor(pos.z());

						std::stringstream ss;
						ss << X << ", " << Y << ", " << Z;

						std::string key = ss.str();

						if (contains(counters, key)) {
							if (!contains(handled, key)) {
								distinct.push_back(i);
								handled[key] = true;
							}
						}
						else {
							distinct.push_back(i);
						}

					}

					//cout << "#distinct: " << distinct.size() << endl;
					/*
					std::stringstream msg;
					msg << "Too many duplicate points were encountered. #points: " << subject->numPoints;
					msg << ", #unique points: " << distinct.size() << std::endl;
					msg << "Duplicates inside node will be dropped! ";
					msg << "min: " << subject->min.toString() << ", max: " << subject->max.toString();

					logger::WARN(msg.str());
					*/

					std::shared_ptr<std::vector<point_t>> distinctBuffer = std::make_shared<std::vector<point_t>>(distinct.size());

					for (int64_t i = 0; i < distinct.size(); i++) {
						distinctBuffer->data()[i] = buffer->data()[i];
					}

					subject->points = distinctBuffer;
					subject->num_points = distinct.size();

					// try again
					nodeIndex--;
				}

			}

			int64_t nextNumPoins = subject->num_points;

			subject->points = nullptr;
			subject->num_points = 0;

			build_hierarchy(indexer, subject, buffer, nextNumPoins, depth + 1);
		}
	}
	template <typename point_t>
	std::vector<point_t> octree_lod_generator<point_t>::generate_lods(const std::vector<point_t>& points)
	{
		std::vector<point_t> out;
		out.reserve(points.size());

		point_t* source_data = (point_t*)points.data();
		size_t source_data_size = points.size();

		//find min, max
		static constexpr float Infinity = std::numeric_limits<float>::infinity();
		vec3 min = { Infinity , Infinity , Infinity };
		vec3 max = { -Infinity , -Infinity , -Infinity };

		for (int i = 0; i < source_data_size; ++i) {
			vec3& p = source_data[i].position();
			min.x() = std::min(min.x(), p.x());
			min.y() = std::min(min.y(), p.y());
			min.z() = std::min(min.z(), p.z());

			max.x() = std::max(max.x(), p.x());
			max.y() = std::max(max.y(), p.y());
			max.z() = std::max(max.z(), p.z());
		}

		vec3 ext = max - min;
		float cube_size = *std::max_element(ext.begin(), ext.end());

		max = min + vec3(cube_size, cube_size, cube_size);

		Chunks<point_t> nodes = lod_chunking(source_data, source_data_size, min, max, cube_size);

		SamplerRandom<point_t> sampler;
		FlatIndexer indexer(&out);
		indexing(nodes, indexer, sampler);
		assert(out.size() == points.size());
		return out;
	}
	


	template <typename point_t>
	std::shared_ptr<IndexNode<point_t>> octree_lod_generator<point_t>::build_octree(const std::vector<point_t>& points) {
		const point_t* source_data = points.data();
		size_t source_data_size = points.size();

		//find min, max
		static constexpr float Infinity = std::numeric_limits<float>::infinity();
		vec3 min = { Infinity , Infinity , Infinity };
		vec3 max = { -Infinity , -Infinity , -Infinity };

		for (int i = 0; i < source_data_size; ++i) {
			const vec3& p = source_data[i].position();
			min.x() = std::min(min.x(), p.x());
			min.y() = std::min(min.y(), p.y());
			min.z() = std::min(min.z(), p.z());

			max.x() = std::max(max.x(), p.x());
			max.y() = std::max(max.y(), p.y());
			max.z() = std::max(max.z(), p.z());
		}

		vec3 ext = max - min;
		float cube_size = *std::max_element(ext.begin(), ext.end());

		max = min + vec3(cube_size, cube_size, cube_size);

		Chunks<point_t> nodes = lod_chunking(source_data, source_data_size, min, max, cube_size);

		SamplerRandom<point_t> sampler;
		Indexer indexer;
		indexing(nodes, indexer, sampler);
		return std::move(indexer.get_root());
	}



	template <typename point_t>
	IndexNode<point_t>::IndexNode(const std::string& name, const vec3& min, const vec3& max)
	{
		this->name = name;
		this->min = min;
		this->max = max;
	}

	template <typename point_t>
	void IndexNode<point_t>::traverse_pre(std::function<void(IndexNode<point_t>*)> callback)
	{
		callback(this);

		for (auto child : children) {

			if (child != nullptr) {
				child->traverse_pre(callback);
			}

		}
	}

	template <typename point_t>
	void IndexNode<point_t>::traverse_post(std::function<void(IndexNode*)> callback)
	{
		for (auto child : children) {

			if (child != nullptr) {
				child->traverse_post(callback);
			}
		}

		callback(this);
	}

	template <typename point_t>
	bool IndexNode<point_t>::is_leaf() {

		for (auto child : children) {
			if (child != nullptr) {
				return false;
			}
		}


		return true;
	}

	template <typename point_t>
	cgv::render::render_types::box3 octree_lod_generator<point_t>::child_bounding_box_of(const vec3& min, const vec3& max, const int index)
	{
		vec3 min_pnt, max_pnt;
		auto size = max - min;
		vec3 center = min + (size * 0.5f);

		if ((index & 0b100) == 0) {
			min_pnt.x() = min.x();
			max_pnt.x() = center.x();
		}
		else {
			min_pnt.x() = center.x();
			max_pnt.x() = max.x();
		}

		if ((index & 0b010) == 0) {
			min_pnt.y() = min.y();
			max_pnt.y() = center.y();
		}
		else {
			min_pnt.y() = center.y();
			max_pnt.y() = max.y();
		}

		if ((index & 0b001) == 0) {
			min_pnt.z() = min.z();
			max_pnt.z() = center.z();
		}
		else {
			min_pnt.z() = center.z();
			max_pnt.z() = max.z();
		}

		return box3(min_pnt, max_pnt);
	}

	// adds a descendant Node at the position given by its name to the Nodes tree. Missing nodes are generated on the way
	template <typename point_t>
	void IndexNode<point_t>::add_descendant(std::shared_ptr<IndexNode<point_t>> descendant) {
		static std::mutex mtx;
		std::lock_guard<std::mutex> lock(mtx);

		int descendantLevel = descendant->name.size() - 1;

		IndexNode* current = this;

		for (int level = 1; level < descendantLevel; level++) {
			int index = descendant->name[level] - '0';

			if (current->children[index] != nullptr) {
				current = current->children[index].get();
			}
			else {
				std::string childName = current->name + std::to_string(index);
				auto box = octree_lod_generator<point_t>::child_bounding_box_of(current->min, current->max, index);

				auto child = std::make_shared<IndexNode>(childName, box.get_min_pnt(), box.get_max_pnt());

				current->children[index] = child;

				current = child.get();
			}
		}

		auto index = descendant->name[descendantLevel] - '0';
		current->children[index] = descendant;
	}

	} //octree namespace
	//make classes avaiable in pointcloud namespace
	using cgv::pointcloud::octree::octree_lod_generator;
	using cgv::pointcloud::octree::SimpleLODPoint;
	using cgv::pointcloud::octree::GenericLODPoint;
} //pointcloud namespace
} //cgv namespace

#include <cgv/config/lib_end.h>