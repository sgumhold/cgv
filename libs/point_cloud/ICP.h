#pragma once

#include <vector>
#include "point_cloud.h"
#include "ann_tree.h"
#include <random>
#include <ctime>
#include <cgv/math/svd.h> 
#include "lib_begin.h"
#include <memory>

namespace cgv {

	namespace pointcloud {

		class CGV_API ICP : public point_cloud_types {
		public:
			enum Sampling_Type {
				DEFAULT_SAMPLING = 0,
				RANDOM_SAMPLING = 1,
				NORMAL_SPACE_SAMPLING = 2  ,
			} S_type;

			const point_cloud* sourceCloud;
			const point_cloud* targetCloud;
			int maxIterations;
			int numRandomSamples;
			float eps;
			point_cloud* crspd_source;
			point_cloud* crspd_target;

			ICP();
			~ICP();

			void build_ann_tree();
			void clear();
			void set_source_cloud(const point_cloud& inputCloud);
			void set_target_cloud(const point_cloud& inputCloud, std::shared_ptr<ann_tree> precomputed_tree = nullptr);
			void set_iterations(int Iter);
			void set_num_random(int NR);
			void set_eps(float e);
			void reg_icp(Mat& rotation_m, Dir& translation_v);
			void get_center_point(const point_cloud& input, Pnt& mid_point);
			float error(Pnt& ps, Pnt& pd, Mat& r, Dir& t);
			void get_crspd(Mat& rotation_m, Dir& translation_v, point_cloud& pc1, point_cloud& pc2);
			void print_rotation(float* rotationMatrix);
			void print_translation(float* translation);

			bool correspondences_filter(const point_cloud& source, const point_cloud& target, Pnt& source_p, Pnt& target_p);
			float dis_pts(const Pnt& source_p, const Pnt& target_p);
		private:
			std::shared_ptr<ann_tree> tree;
		};
	}
}
#include <cgv/config/lib_end.h>
