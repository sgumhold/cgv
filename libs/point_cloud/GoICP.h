#pragma once
#include <vector>
#include "point_cloud.h"
#include <random>
#include <ctime>
#include <cgv/math/svd.h>
#include "3ddt.h"


#include "lib_begin.h"

namespace cgv {

	namespace pointcloud {

		struct rotation_node
		{
			float a, b, c, w;
			float ub, lb;
			int l;
			friend bool operator < (const struct rotation_node & n1, const struct rotation_node & n2)
			{
				if (n1.lb != n2.lb)
					return n1.lb > n2.lb;
				else
					return n1.w < n2.w;
			}

		};

		struct translation_node
		{
			float x, y, z, w;
			float ub, lb;
			friend bool operator < (const translation_node & n1, const translation_node & n2)
			{
				if (n1.lb != n2.lb)
					return n1.lb > n2.lb;
				else
					return n1.w < n2.w;
			}
		};

		class CGV_API GoICP : public point_cloud_types {
			typedef cgv::math::fvec<float, 3> vec3;
			typedef cgv::math::fmat<float, 4, 4> mat;
		public:
			GoICP();
			void build_distance_transform();

			inline void set_source_cloud(const point_cloud &inputCloud) {
				source_cloud = &inputCloud;
			}
			inline void set_target_cloud(const point_cloud &inputCloud) {
				target_cloud = &inputCloud;
			};

		protected:
			float icp(mat& R_icp, mat& t_icp);
		private:
			const point_cloud *source_cloud;
			const point_cloud *target_cloud;

			Mat rotation;
			Dir translation;

			DT3D distance_transform;

			rotation_node init_rot;
			translation_node init_trans;

			//settings
			float mse_threshhold;
		};

	}
}
#include <cgv/config/lib_end.h>