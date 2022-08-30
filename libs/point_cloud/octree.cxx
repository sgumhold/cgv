#include "octree.h"

namespace cgv {
namespace pointcloud {
namespace octree {


std::vector<std::vector<int64_t>> createSumPyramid(std::vector<int64_t>& grid, int gridSize)
{
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


	std::vector<NodeCandidate> createNodes(std::vector<std::vector<int64_t>>& pyramid, int64_t maxPointsPerChunk)
{

	std::vector<NodeCandidate> nodes;

	std::vector<std::vector<int64_t>> pyramidOffsets;
	for (auto& counters : pyramid) {

		if (counters.size() == 1) {
			pyramidOffsets.push_back({0});
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

	std::vector<NodeCandidate> stack = {root};

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


}
} // namespace pointcloud
} // namespace cgv