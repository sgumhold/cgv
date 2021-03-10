#include <algorithm>
#include <random>
#include <unordered_map>
#include <sstream>
#include "clod_point_renderer.h"

//for debug purposes
#include <cgv/utils/file.h>

namespace {
	//** concurrency classes *//
	class SenseReversingBarrier
	{
		int remaining = 0;
		int initial_count = 0;
		bool barrier_sense = false;
		std::mutex barrier_mutex;
		std::condition_variable condition;
		std::unordered_map<std::thread::id, bool> thread_sense;
	public:
		SenseReversingBarrier(const int i) {
			remaining = i;
			initial_count = i;
		}

		bool sense() const {
			return barrier_sense;
		}

		// post_once is run by one thread whith following condition, 
		//	-- run after all threads reached the phase synchronization point 
		//  -- before any thread is notified in the current phase
		template <typename F>
		void await(F post_once) {
			bool sense;
			std::thread::id id = std::this_thread::get_id();
			std::unique_lock<std::mutex> lock(barrier_mutex);
			auto sen = thread_sense.find(id);
			if (sen != thread_sense.end()) {
				sense = sen->second;
			}
			else {
				sense = !barrier_sense;
			}

			--remaining;
			if (remaining == 0) {
				remaining = initial_count; //one addition for the calling thread
				barrier_sense = !barrier_sense;
				post_once();
				condition.notify_all();
			}
			else {
				while (sense != barrier_sense)
					condition.wait(lock);
			}
			thread_sense[id] = !sense;
		}

		void await() {
			await([]() {});
		}
	};

	//class is not thread safe
	class WorkerPool {

		struct PoolTask {
			//Task* task;
			std::function<void(int)> task;
			std::atomic_int remaining; //threads not finished yet
			PoolTask(const std::function<void(int)>& t, const int num_threads) : task(t), remaining(num_threads) {}
		};

		SenseReversingBarrier barrier_pre;
		SenseReversingBarrier barrier_post;

		std::unique_ptr<PoolTask> current_task = nullptr;

		std::vector<std::thread> threads;

		bool stop_request = false;

	public:
		WorkerPool(unsigned i) : barrier_pre(i + 1), barrier_post(i + 1) {
			construct_threads(i);
		}
		~WorkerPool() {
			join_all();
		}

		//collectiv task execution. calling thread also runs the task and is getting a thread id assigned
		template<typename F>
		void run(F func) {
			//no lock required since the calling threads is the only accessing thread
			current_task = std::make_unique<PoolTask>(std::move(std::function<void(int)>(func)), (int)threads.size() + 1);
			worker_kernel(current_task.get(), threads.size(), false);
		}

	protected:
		//join i threads (blocking)
		void join_all();

		// kernel runs in parallel
		void worker_kernel(PoolTask* ptask, int thread_id, bool is_pool_member = true);

		//add i threads to the pool (blocking)
		void construct_threads(unsigned i) {
			//create threads
			while (i > 0) {
				bool sense = !barrier_pre.sense();
				threads.emplace_back(&WorkerPool::worker_kernel, this, nullptr, --i, true);
			}
		}
	};

	void WorkerPool::join_all()
	{
		{
			{
				stop_request = true;
				current_task = nullptr;
			}
			worker_kernel(nullptr, -1, false);
		}
		for (auto& thread : threads) {
			thread.join();
		}
		threads.clear();
		stop_request = false;
	}

	void WorkerPool::worker_kernel(PoolTask* ptask, int thread_id, bool is_pool_member)
	{
		do {
			//std::printf("%d arrived\n", thread_id);
			barrier_pre.await(); //usually threads wait on this barrier
			//std::printf("%d passed barrier\n", thread_id);
			ptask = current_task.get();
			//terminate on request
			if (stop_request) {
				return;
			}

			//run any existing task
			if (ptask) {
				ptask->task(thread_id);
				//threads resynchronize on this barrier prior marking the Task as done
				barrier_post.await([this]() {
					current_task = nullptr;
					});

			}
		} while (is_pool_member);
	}

	static WorkerPool pool(std::thread::hardware_concurrency() - 1);
}
//**


namespace cgv {
	namespace render {
		namespace {
			struct SamplerRandom : public octree_lod_generator::Sampler {

				// subsample a local octree from bottom up
				void sample(std::shared_ptr<octree_lod_generator::IndexNode> node, double baseSpacing, std::function<void(octree_lod_generator::IndexNode*)> onNodeCompleted) {
					using IndexNode = octree_lod_generator::IndexNode;
					using vec3 = cgv::render::render_types::vec3;
					using Vertex = clod_point_renderer::Point;
					
					//lambda for bottom up traversal
					std::function<void(IndexNode*, std::function<void(IndexNode*)>)> traversePost = [&traversePost](IndexNode* node, std::function<void(IndexNode*)> callback) {
						for (auto child : node->children) {

							if (child != nullptr && !child->sampled) {
								traversePost(child.get(), callback);
							}
						}

						callback(node);
					};
					

					traversePost(node.get(), [baseSpacing, &onNodeCompleted](IndexNode* node) {
						node->sampled = true;

						int64_t numPoints = node->num_points;

						int64_t gridSize = 128;
						thread_local std::vector<int64_t> grid(gridSize * gridSize * gridSize, -1);
						thread_local int64_t iteration = 0;
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

							auto buffer = std::make_shared<std::vector<Vertex>>(node->num_points);

							for (int i = 0; i < node->num_points; i++) {

								int64_t sourceOffset = i;
								int64_t targetOffset = indices[i];

								buffer->at(targetOffset) = node->points->at(sourceOffset);
								//memcpy(buffer->data() + targetOffset, node->points->data() + sourceOffset, sizeof(Vertex));
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

							std::vector<int8_t> acceptedFlags(child->num_points, 0);
							int64_t numRejected = 0;

							for (int i = 0; i < child->num_points; i++) {

								int64_t pointOffset = i;
								vec3 pos = child->points->at(pointOffset).position;

								CellIndex cellIndex = toCellIndex(pos);

								auto& gridValue = grid[cellIndex.index];

								static double all = sqrt(3.0);

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

						auto accepted = std::make_shared<std::vector<Vertex>>(numAccepted);
						for (int childIndex = 0; childIndex < 8; childIndex++) {
							auto child = node->children[childIndex];

							if (child == nullptr) {
								continue;
							}

							auto numRejected = numRejectedPerChild[childIndex];
							auto& acceptedFlags = acceptedChildPointFlags[childIndex];
							auto rejected = std::make_shared<std::vector<Vertex>>(numRejected);

							for (int i = 0; i < child->num_points; i++) {
								auto isAccepted = acceptedFlags[i];
								int64_t pointOffset = i;

								if (isAccepted) {
									accepted->push_back(child->points->at(pointOffset));
								}
								else {
									rejected->push_back(child->points->at(pointOffset));
								}
							}

							if (numRejected == 0) {
								node->children[childIndex] = nullptr;
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


		}

		clod_point_render_style::clod_point_render_style() {}

		bool clod_point_render_style::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				//rh.reflect_base(*static_cast<point_render_style*>(this)) &&
				rh.reflect_member("CLOD_factor", CLOD) &&
				rh.reflect_member("spacing", spacing) &&
				rh.reflect_member("scale", scale) &&
				rh.reflect_member("min_millimeters", min_millimeters) &&
				rh.reflect_member("point_size", pointSize);
		}

		std::string octree_lod_generator::to_node_id(int level, int gridSize, int64_t x, int64_t y, int64_t z) {
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

		octree_lod_generator::Chunks octree_lod_generator::lod_chunking(const Vertex* vertices, const size_t num_points, const vec3& min, const vec3& max, const float& cube_size)
		{
			// stores chunks created by the chunking phase
			Chunks chunks;

			max_points_per_chunk = std::min<size_t>(source_data_size / 20, 10'000'000ll);
			
			if (source_data_size < 4'000'000) {
				grid_size = 32;
			}
			else if (source_data_size < 20'000'000) {
				grid_size = 64;
			}
			else if (source_data_size < 100'000'000) {
				grid_size = 128;
			}
			else if (source_data_size < 500'000'000) {
				grid_size = 256;
			}
			else {
				grid_size = 512;
			}

			// COUNT
			auto grid = lod_counting(source_data, source_data_size, grid_size, min, max, cube_size);
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

				auto lut = lod_createLUT(grid, grid_size,chunks.nodes);
#ifndef NDEBUG
				{ //Test counting
					bool has_valid_entries = false;
					int first_valid = -1;
					int value = -1;
					for (const auto& e : lut.grid) {
						if (e > -1){
							has_valid_entries = true;
							value = e;
							break;
						}
					}
					assert(has_valid_entries);
				}
#endif

				distributePoints(min, max,cube_size, lut, vertices, num_points,chunks.nodes);
			}

			chunks.min = min;
			chunks.max = max;
			return std::move(chunks);
		}


		std::vector<std::atomic_int32_t> octree_lod_generator::lod_counting(const Vertex* vertices, const int64_t num_points, int64_t grid_size, const vec3& min, const vec3& max, const float& cube_size)
		{	
			int64_t points_left = num_points;
			int64_t batch_size = 1'000'000;
			//int64_t batch_size = num_points;
			int64_t num_read = 0;

			std::vector<std::atomic_int32_t> grid(grid_size * grid_size * grid_size);
			std::vector<std::thread> threads;

			auto processor = [this, &grid, &cube_size, grid_size, vertices, &min, &max](int64_t first_point, int64_t num_points) {
				for (int i = 0; i < num_points; i++) {
					int64_t index = grid_index(vertices[first_point + i].position, min, cube_size, grid_size);
					grid[index].fetch_add(1, std::memory_order::memory_order_relaxed);
				}
			};

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
						func(task->first_point,task->num_points);
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

			pool.run([&tasks](int thread_id) {tasks(); });
			
			return std::move(grid);
		}


		octree_lod_generator::NodeLUT octree_lod_generator::lod_createLUT(std::vector<std::atomic_int32_t>& grid, int64_t grid_size, std::vector<ChunkNode>& nodes)
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
				for_xyz(gridSize_low, [this, &nodes ,&grid_low, &grid_high, gridSize_low, gridSize_high, level_low, level_high, level_max](int64_t x, int64_t y, int64_t z) {

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

								ChunkNode node(node_id,value);
								node.x = nx;
								node.y = ny;
								node.z = nz;
								node.size = pow(2, (level_max - level_high));

								nodes.emplace_back(node);
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

				for_xyz(node.size, [node, &lut, grid_size, i](int64_t ox, int64_t oy, int64_t oz) {
					int64_t x = node.size * node.x + ox;
					int64_t y = node.size * node.y + oy;
					int64_t z = node.size * node.z + oz;
					int64_t index = x + y * grid_size + z * grid_size * grid_size;

					lut[index] = i;
					});
			}
			
			return { grid_size, lut };
		}

		
		cgv::render::render_types::ivec3 octree_lod_generator::grid_index_vec(const vec3& position, const vec3& min, const float& cube_size, const int& grid_size) const {
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

		int64_t octree_lod_generator::grid_index(const vec3& position, const vec3& min, const float& cube_size, const int& grid_size) const
		{
			auto v = grid_index_vec(position, min, cube_size,grid_size);
			int64_t index = v.x() + v.y() * grid_size + v.z() * grid_size * grid_size;
			return index;
		}

		void octree_lod_generator::distributePoints(vec3 min, vec3 max, float cube_size, NodeLUT& lut, const Vertex* vertices, const int64_t num_points, const std::vector<ChunkNode>& nodes)
		{
			//std::mutex mtx_push_point;

			struct Task {
				int64_t maxBatchSize;
				int64_t batchSize;
				int64_t firstPoint;
				NodeLUT* lut;
				vec3 min;
				vec3 max;
			};

			auto& grid = lut.grid;
			std::vector<int> broken_index;
			//TODO parallelize
			for (int i = 0; i < num_points; ++i) {
				vec3 p = vertices[i].position;
				int idx = grid_index(p, min, cube_size, grid_size);
				
				if (grid[idx] == -1) {
					broken_index.push_back(idx);
					continue;
					//cgv::utils::file::write("R:/dump.bin", (char*)grid.data(), grid.size() * sizeof(int));
				}
				
				auto& node = nodes[grid[idx]];
				Vertex v = vertices[i];
				node.pc_data->vertices.push_back(v);
			}
			assert(broken_index.size() == 0);
			int test = 0;
		}

		void octree_lod_generator::lod_indexing(Chunks& chunks, std::vector<Vertex>& vertices, Sampler& sampler)
		{
			struct Task {
				ChunkNode* chunk = nullptr;
				//one chunk per task
				explicit Task(ChunkNode& chunk) {
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


			Indexer indexer(&vertices);
			std::mutex mtx_nodes;
			std::vector<std::shared_ptr<IndexNode>> nodes;
			
			indexer.root = std::make_shared<IndexNode>("r", chunks.min, chunks.max);
			indexer.spacing = (chunks.max - chunks.min).x() / 128.0;

			//builds node hierachy
			tasks.func = [this,&indexer,&sampler,&nodes,&mtx_nodes](Task* task) {
				static constexpr float Infinity = std::numeric_limits<float>::infinity();
				ChunkNode* chunk = task->chunk;
				vec3 min(Infinity), max(-Infinity);

				for (auto& v : chunk->pc_data->vertices) {
					min.x() = std::min(min.x(), v.position.x());
					min.y() = std::min(min.y(), v.position.y());
					min.z() = std::min(min.z(), v.position.z());

					max.x() = std::max(max.x(), v.position.x());
					max.y() = std::max(max.y(), v.position.y());
					max.z() = std::max(max.z(), v.position.z());
				}

				auto chunk_root = std::make_shared<IndexNode>(chunk->id, min, max);
				
				//alias vertices
				std::shared_ptr<std::vector<Vertex>> points(chunk->pc_data, &(chunk->pc_data->vertices));

				buildHierarchy(&indexer, chunk_root.get(), points, points->size());

				auto onNodeCompleted = [&indexer](IndexNode* node) {
					indexer.write(*node);
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

				nodes.push_back(std::move(chunk_root));
			};

			//fill task pool
			for (auto& chunk : chunks.nodes) {
				tasks.task_pool.emplace_back(chunk);
			}
			
			//run octree generation on each chunk in parallel
			std::vector<std::thread> thread_pool;
			int num_threads = 1;// std::thread::hardware_concurrency();
			for (int i = 0; i < num_threads; ++i) {
				thread_pool.emplace_back([&tasks]() {tasks(); });
			}
			
			for (auto& thread : thread_pool) {
				thread.join();
			}

			//indexer.reloadChunkRoots();

			if (chunks.nodes.size() == 1) {
				indexer.root = nodes[0];
			}
			else {

				auto onNodeCompleted = [&indexer](IndexNode* node) {
					//indexer.write(*node);
				};

				sampler.sample(indexer.root, indexer.spacing, onNodeCompleted);
			}
			
			//indexer.write(*indexer.root.get());
			//write nodes
			nodes.push_back(indexer.root);
			for (auto& node : nodes) {
				indexer.write(*node.get());
			}
			//indexer.writer->closeAndWait();
		}


		// see https://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
		// method to seperate bits from a given integer 3 positions apart
		inline uint64_t splitBy3(unsigned int a) {
			uint64_t x = a & 0x1fffff; // we only look at the first 21 bits
			x = (x | x << 32) & 0x1f00000000ffff; // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
			x = (x | x << 16) & 0x1f0000ff0000ff; // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
			x = (x | x << 8) & 0x100f00f00f00f00f; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
			x = (x | x << 4) & 0x10c30c30c30c30c3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
			x = (x | x << 2) & 0x1249249249249249;
			return x;
		}

		// COPYPASTE from potree converter

		struct NodeCandidate {
			std::string name = "";
			int64_t indexStart = 0;
			int64_t numPoints = 0;
			int64_t level = 0;
			int64_t x = 0;
			int64_t y = 0;
			int64_t z = 0;
		};

		inline uint64_t mortonEncode_magicbits(unsigned int x, unsigned int y, unsigned int z) {
			uint64_t answer = 0;
			answer |= splitBy3(x) | splitBy3(y) << 1 | splitBy3(z) << 2;
			return answer;
		}

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

							auto index = mortonEncode_magicbits(z, y, x);
							auto index_p1 = mortonEncode_magicbits(2 * z, 2 * y, 2 * x);

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
				auto index = mortonEncode_magicbits(z, y, x);
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

						auto index_p1 = mortonEncode_magicbits(2 * z, 2 * y, 2 * x) + i;
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


		// END COPYPASTE from potree converter
		
		void octree_lod_generator::buildHierarchy(Indexer* indexer, IndexNode* node, std::shared_ptr<std::vector<Vertex>> points, int64_t numPoints, int64_t depth)
		{
			if (numPoints < max_points_per_index_node) {
				IndexNode* realization = node;
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
				vec3 xyz = points->at(pointIndex).position;
				double x = xyz[0];
				double y = xyz[1];
				double z = xyz[2];

				int64_t ix = std::floor(double(counterGridSize) * (x - min.x()) / size.x());
				int64_t iy = std::floor(double(counterGridSize) * (y - min.y()) / size.y());
				int64_t iz = std::floor(double(counterGridSize) * (z - min.z()) / size.z());

				ix = std::max(int64_t(0), std::min(ix, counterGridSize - 1));
				iy = std::max(int64_t(0), std::min(iy, counterGridSize - 1));
				iz = std::max(int64_t(0), std::min(iz, counterGridSize - 1));

				return mortonEncode_magicbits(iz, iy, ix); //replace with lookup table based morton encoding
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

				std::vector<Vertex> tmp(numPoints);

				for (int64_t i = 0; i < numPoints; i++) {
					auto index = gridIndexOf(i);
					auto targetIndex = offsets[index]++;

					tmp[targetIndex] = (*points)[i];
					memcpy(tmp.data() + targetIndex, points->data() + i, sizeof(Vertex));
				}
				memcpy(points->data(), tmp.data(), numPoints * sizeof(Vertex));
			}

			auto pyramid = createSumPyramid(counters, counterGridSize);

			auto nodes = createNodes(pyramid, max_points_per_index_node);

			auto expandTo = [node](NodeCandidate& candidate) {

				std::string startName = node->name;
				std::string fullName = startName + candidate.name;

				// e.g. startName: r, fullName: r031
				// start iteration with char at index 1: "0"

				IndexNode* currentNode = node;
				for (int64_t i = startName.size(); i < fullName.size(); i++) {
					int64_t index = fullName.at(i) - '0';

					if (currentNode->children[index] == nullptr) {
						auto childBox = child_bounding_box_of(currentNode->min, currentNode->max, index);
						std::string childName = currentNode->name + std::to_string(index);

						std::shared_ptr<IndexNode> child = std::make_shared<IndexNode>();
						child->min = childBox.get_min_pnt();
						child->max = childBox.get_max_pnt();
						child->name = childName;
						child->children.resize(8);

						currentNode->children[index] = child;
						currentNode = child.get();
					}
					else {
						currentNode = currentNode->children[index].get();
					}


				}

				return currentNode;
			};

			std::vector<IndexNode*> needRefinement;

			int64_t octreeDepth = 0;

			for (NodeCandidate& candidate : nodes) {

				IndexNode* realization = expandTo(candidate);
				realization->index_start = candidate.indexStart;
				realization->num_points = candidate.numPoints;
				
				auto buffer = std::make_shared<std::vector<Vertex>>(candidate.numPoints); //potential out of memory here
				memcpy(buffer->data(),
					points->data() + candidate.indexStart,
					candidate.numPoints
				);

				realization->points = buffer;

				if (realization->num_points > max_points_per_index_node) {
					needRefinement.push_back(realization);
				}

				octreeDepth = std::max(octreeDepth, realization->level());
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
					//failed to partition point cloud in indexer::buildHierarchy();
				}

				if (subject->num_points == numPoints) {
					// the subsplit has the same number of points than the input -> ERROR

					std::unordered_map<std::string, int> counters;

					for (int64_t i = 0; i < numPoints; i++) {

						int64_t sourceOffset = i;
						vec3 pos = buffer->data()[sourceOffset].position;
						
						int X=std::floor(pos.x()), Y=std::floor(pos.y()), Z=std::floor(pos.z());
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

							vec3 pos = buffer->data()[sourceOffset].position;

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

						std::shared_ptr<std::vector<Vertex>> distinctBuffer = std::make_shared<std::vector<Vertex>>(distinct.size());

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

				buildHierarchy(indexer, subject, buffer, nextNumPoins, depth + 1);
			}
		}

		std::vector <octree_lod_generator::Vertex> octree_lod_generator::generate_lods(const std::vector<Vertex>& vertices)
		{
			std::cerr << "Warning: octree method is unstable!\n";
			std::vector<Vertex> out;
			this->source_data = (Vertex*)vertices.data();
			this->source_data_size = vertices.size();

			//find min, max
			static constexpr float Infinity = std::numeric_limits<float>::infinity();
			vec3 min = { Infinity , Infinity , Infinity };
			vec3 max = { -Infinity , -Infinity , -Infinity };

			for (int i = 0; i < source_data_size; ++i) {
				vec3& p = source_data[i].position;
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

			auto nodes = lod_chunking(vertices.data(), vertices.size(), min, max, cube_size);

			SamplerRandom sampler;
			lod_indexing(nodes, out, sampler);
			return out;
			//TODO continue
		}



		void clod_point_renderer::generate_lods_poisson()
		{
			static constexpr int mean = 8;
			bool run_parralel = (input_buffer_data.size() > 10'000);

			if (run_parralel) {

				struct Task {
					Point* start;
					int num_points;
				};
				struct Tasks {
					std::atomic_int next_task = 0;
					std::vector<Task> task;
				} tasks;

				int64_t points_distributed = 0;
				int64_t points_total = input_buffer_data.size();
				constexpr int64_t batch_size = 500000;

				while (points_distributed < points_total) {
					int64_t batch = std::min(batch_size, points_total - points_distributed);
					tasks.task.push_back({ &input_buffer_data[points_distributed],(int)batch });
					points_distributed += batch;
				}

				pool.run([this, &tasks](int thread_id) {
					std::poisson_distribution<int> dist(mean);
					std::random_device rdev;

					while (true) {
						//fetch task
						int tid = tasks.next_task.fetch_add(1, std::memory_order_relaxed);
						if (tid < tasks.task.size()) {
							Task& task = tasks.task[tid];

							Point* end = task.start + task.num_points;
							for (Point* p = task.start; p < end; p++) {
								p->level = std::min(2 * mean, std::max(0, mean - abs(dist(rdev) - mean)));
							}
						}
						else {
							return;
						}
					}
					});
			}
			else {
				std::poisson_distribution<int> dist(8);
				std::random_device rdev;

				for (auto& v : input_buffer_data) {
					v.level = std::min(2 * mean, std::max(0, mean - abs(dist(rdev) - mean)));
				}
			}
		}

		void clod_point_renderer::draw_and_compute_impl(context& ctx, PrimitiveType type, size_t start, size_t count)
		{
			//renderer::draw_impl(ctx, type, start, count, use_strips, use_adjacency, strip_restart_index);
			
			//TODO add option to spread calculation over multiple frames
			//configure shader to compute everything after one frame
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchOffset"), 0);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchSize"), (int)count);

			// reset draw parameters
			DrawParameters dp = DrawParameters();
			//glBindBuffer(GL_SHADER_STORAGE_BUFFER, draw_parameter_buffer);
			//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawParameters), &dp, GL_STREAM_DRAW);
			//glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(DrawParameters), &dp);
			glNamedBufferSubData(draw_parameter_buffer, 0, sizeof(DrawParameters), &dp);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			// reduce
			reduce_prog.enable(ctx);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, drawp_pos, draw_parameter_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, render_pos, render_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, input_pos, input_buffer);
			glDispatchCompute((input_buffer_data.size()/128)+1, 1, 1); //with NVIDIA cards this will spam notifications about buffer usage 

			// synchronize
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			reduce_prog.disable(ctx);

			// draw composed buffer
			draw_prog.enable(ctx);
			glBindVertexArray(vertex_array);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_parameter_buffer);
			glDrawArraysIndirect(GL_POINTS, 0);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER,0);
			
			//DEBUG map buffer into host address space
			//DrawParameters* device_draw_parameters = static_cast<DrawParameters*>(glMapNamedBufferRange(draw_parameter_buffer, 0, sizeof(DrawParameters), GL_MAP_READ_BIT));
			//glUnmapNamedBuffer(draw_parameter_buffer);
			
			glBindVertexArray(0);
			draw_prog.disable(ctx);
		}

		const render_style* clod_point_renderer::get_style_ptr() const
		{
			if (rs)
				return rs;
			if (default_render_style)
				return default_render_style;
			default_render_style = create_render_style();
			return default_render_style;
		}

		render_style* clod_point_renderer::create_render_style() const
		{
			return new clod_point_render_style();
		}

		bool clod_point_renderer::init(context& ctx)
		{
			if (!reduce_prog.is_created()) {
				reduce_prog.create(ctx);
				add_shader(ctx, reduce_prog, "view.glsl", cgv::render::ST_COMPUTE);
				add_shader(ctx, reduce_prog, "point_clod_filter_points.glcs", cgv::render::ST_COMPUTE);
				reduce_prog.link(ctx);
#ifndef NDEBUG
				std::cerr << reduce_prog.last_error;
#endif // #ifdef NDEBUG
			}
			
			//create shader program
			if (!draw_prog.is_created()) {
				draw_prog.build_program(ctx, "point_clod.glpr", true);
#ifndef NDEBUG
				std::cerr << draw_prog.last_error;
#endif // #ifdef NDEBUG
			}
			
			glGenBuffers(1, &input_buffer); //array of {float x;float y;float z;uint colors;};
			glGenBuffers(1, &render_buffer);
			glGenBuffers(1, &draw_parameter_buffer);

			glGenVertexArrays(1, &vertex_array);
			glBindVertexArray(vertex_array);
			//position 
			glBindBuffer(GL_ARRAY_BUFFER, render_buffer);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
			glEnableVertexAttribArray(0);
			//color
			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Point), (void*)(sizeof(Point::position)));
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			return draw_prog.is_linked() && reduce_prog.is_linked();
		}

		bool clod_point_renderer::enable(context& ctx)
		{
			if (!draw_prog.is_linked()) {
				return false;
			}

			if (buffers_outofdate) {
				fill_buffers(ctx);
				buffers_outofdate = false;
			}

			//const clod_point_render_style& srs = get_style<clod_point_render_style>();
			vec2 screenSize(ctx.get_width(), ctx.get_height());
			//transform to model space since there is no view matrix
			vec4 pivot = inv(ctx.get_modelview_matrix())*dvec4(0.0,0.0,0.0,1.0);

			//mat4 modelview_matrix = ctx.get_modelview_matrix();
			//mat4 projection_matrix = ctx.get_projection_matrix();
			
			const clod_point_render_style& prs = get_style<clod_point_render_style>();

			draw_prog.set_uniform(ctx, "CLOD" , prs.CLOD);
			draw_prog.set_uniform(ctx, "scale", prs.scale);
			draw_prog.set_uniform(ctx, "spacing", prs.spacing);
			draw_prog.set_uniform(ctx, "pointSize", prs.pointSize);
			draw_prog.set_uniform(ctx, "minMilimeters", prs.min_millimeters);
			draw_prog.set_uniform(ctx, "screenSize", screenSize);
			draw_prog.set_uniform(ctx, "pivot", pivot);
			

			//view.glsl uniforms, set on draw_prog.enable(ctx) and  reduce_prog.enable(ctx)
			//draw_prog.set_uniform(ctx, "modelview_matrix", modelview_matrix, true);
			//draw_prog.set_uniform(ctx, "projection_matrix", projection_matrix, true);
			//reduce_prog.set_uniform(ctx, "modelview_matrix", modelview_matrix, true);
			//reduce_prog.set_uniform(ctx, "projection_matrix", projection_matrix, true);

			// compute shader
			reduce_prog.set_uniform(ctx, "CLOD", prs.CLOD);
			reduce_prog.set_uniform(ctx, "scale", prs.scale);
			reduce_prog.set_uniform(ctx, "spacing", prs.spacing);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "pivot"), pivot);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "screenSize"), screenSize);

			float y_view_angle = 45;
			//general point renderer uniforms
			draw_prog.set_uniform(ctx, "use_color_index", false);
			//draw_prog.set_uniform(ctx, "use_group_point_size", prs.use_group_point_size);
			float pixel_extent_per_depth = (float)(2.0 * tan(0.5 * 0.0174532925199 * y_view_angle) / ctx.get_height());
			draw_prog.set_uniform(ctx, "pixel_extent_per_depth", pixel_extent_per_depth);
			//draw_prog.set_uniform(ctx, "blend_width_in_pixel", prs.blend_width_in_pixel);
			//draw_prog.set_uniform(ctx, "percentual_halo_width", 0.01f * prs.percentual_halo_width);
			//draw_prog.set_uniform(ctx, "halo_width_in_pixel", prs.halo_width_in_pixel);
			//draw_prog.set_uniform(ctx, "halo_color", prs.halo_color);
			//draw_prog.set_uniform(ctx, "halo_color_strength", prs.halo_color_strength);
			return true;
		}

		bool clod_point_renderer::disable(context& ctx)
		{
			
			const clod_point_render_style& srs = get_style<clod_point_render_style>();			
			return true;
		}

		void clod_point_renderer::clear(const cgv::render::context& ctx)
		{
			reduce_prog.destruct(ctx);
			draw_prog.destruct(ctx);
			clear_buffers(ctx);
		}

		void clod_point_renderer::draw(context& ctx, size_t start, size_t count)
		{
			draw_and_compute_impl(ctx, cgv::render::PT_POINTS, start, count);
		}

		bool clod_point_renderer::render(context& ctx, size_t start, size_t count)
		{
			if (enable(ctx)) {
				draw(ctx, start, count);
				return true;
			}
			return false;
		}

		void clod_point_renderer::generate_lods(const LoDMode mode)
		{
			switch (mode) {
			case LoDMode::POTREE: {
				octree_lod_generator lod;
				input_buffer_data = lod.generate_lods(input_buffer_data);
				break; }
			case LoDMode::RANDOM_POISSON: {
				generate_lods_poisson();
				break;
			}
			}

			
		}

		void clod_point_renderer::set_positions(context& ctx, const std::vector<vec3>& positions)
		{
			input_buffer_data.resize(positions.size());
			for (int i = 0; i < positions.size(); ++i) {
				input_buffer_data[i].position = positions[i];
			}
			buffers_outofdate = true;
		}

		void clod_point_renderer::set_render_style(const render_style& rs)
		{
			this->rs = &rs;
		}

		uint8_t& clod_point_renderer::point_lod(const int i)
		{
			return input_buffer_data[i].level;
		}

		cgv::render::render_types::rgb8& clod_point_renderer::point_color(const int i)
		{
			return input_buffer_data[i].colors;
		}

		cgv::render::render_types::vec3& clod_point_renderer::point_position(const int i)
		{
			return input_buffer_data[i].position;
		}

		void clod_point_renderer::add_shader(context& ctx, shader_program& prog, const std::string& sf,const cgv::render::ShaderType st)
		{
#ifndef NDEBUG
			std::cout << "add shader " << sf << '\n';
#endif // #ifdef NDEBUG
			prog.attach_file(ctx, sf, st);
#ifndef NDEBUG
			if (prog.last_error.size() > 0) {
				std::cerr << prog.last_error << '\n';
				prog.last_error = "";
			}	
#endif // #ifdef NDEBUG

		}

		void clod_point_renderer::fill_buffers(context& ctx)
		{ //  fill buffers for the compute shader
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, input_buffer_data.size() * sizeof(Point), input_buffer_data.data(), GL_STATIC_READ);
			
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, render_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, input_buffer_data.size() * sizeof(Point), nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, draw_parameter_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawParameters), nullptr, GL_STREAM_DRAW);
		}

		void clod_point_renderer::clear_buffers(const context& ctx)
		{
			glDeleteBuffers(1, &input_buffer);
			glDeleteBuffers(1, &render_buffer);
			glDeleteBuffers(1, &draw_parameter_buffer);
			input_buffer = render_buffer = draw_parameter_buffer = 0;
		}
		
		octree_lod_generator::IndexNode::IndexNode(const std::string& name, const vec3& min, const vec3& max)
		{
			this->name = name;
			this->min = min;
			this->max = max;
			children.resize(8, nullptr);
		}

		void octree_lod_generator::IndexNode::traverse_pre(std::function<void(IndexNode*)> callback)
		{
			callback(this);

			for (auto child : children) {

				if (child != nullptr) {
					child->traverse_pre(callback);
				}

			}
		}

		void octree_lod_generator::IndexNode::traverse_post(std::function<void(IndexNode*)> callback)
		{
			for (auto child : children) {

				if (child != nullptr) {
					child->traverse_post(callback);
				}
			}

			callback(this);
		}

		bool octree_lod_generator::IndexNode::is_leaf() {

			for (auto child : children) {
				if (child != nullptr) {
					return false;
				}
			}


			return true;
		}

		render_types::box3 octree_lod_generator::child_bounding_box_of(const vec3& min, const vec3& max, const int index)
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

		void octree_lod_generator::IndexNode::add_descendant(std::shared_ptr<IndexNode> descendant) {
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
					auto box = child_bounding_box_of(current->min, current->max, index);

					auto child = std::make_shared<IndexNode>(childName, box.get_min_pnt(), box.get_max_pnt());

					current->children[index] = child;

					current = child.get();
				}
			}
		}

}
}


#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct clod_point_render_style_gui_creator : public gui_creator {
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*) {
				if (value_type != cgv::type::info::type_name<cgv::render::clod_point_render_style>::get_name())
					return false;
				cgv::render::clod_point_render_style* rs_ptr = reinterpret_cast<cgv::render::clod_point_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
				p->add_member_control(b, "CLOD factor", rs_ptr->CLOD, "value_slider", "min=0.1;max=10;ticks=true");
				p->add_member_control(b, "scale", rs_ptr->scale, "value_slider", "min=0.1;max=10;ticks=true");
				p->add_member_control(b, "point spacing", rs_ptr->spacing, "value_slider", "min=0.1;max=10;ticks=true");
				p->add_member_control(b, "point size", rs_ptr->pointSize, "value_slider", "min=0.1;max=10;ticks=true");
				p->add_member_control(b, "min millimeters", rs_ptr->min_millimeters, "value_slider", "min=0.1;max=10;ticks=true");
				return true;
			}
		};

		cgv::gui::gui_creator_registration<clod_point_render_style_gui_creator> cprsgc("clod_point_render_style_gui_creator");
	}
}