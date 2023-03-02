/*	Sparse Iterative Closest Point by Sofien Bouaziz, Andrea Tagliasacchi, Mark Pauly 2013 */

#include "point_cloud.h"
#include <fstream>
#include <future>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <cgv/math/det.h>
#include "ann_tree.h"
#include "SICP.h"
#include "Eigen/Eigen"

using namespace std;
using namespace cgv::math;

namespace cgv {

	namespace pointcloud {


		float det(const cgv::math::fmat<float, 3, 3>& m) {
			return det_33<float>(m(0, 0), m(0, 1), m(0, 2), m(1, 0), m(1, 1), m(1, 2), m(2, 0), m(2, 1), m(2, 2));
		}

		SICP::SICP() : sourceCloud(nullptr), targetCloud(nullptr) {
			parameters.max_runs = 20;
			parameters.max_inner_loop = 1;
			parameters.max_outer_loop = 50;
			parameters.mu = 10;
			parameters.p = 0.5f;
			parameters.stop = 1e-5;
			parameters.use_penalty = false;
			parameters.max_mu = 1e5;
			parameters.alpha = 1.2f;
		}

		SICP::~SICP() {

		}

		void SICP::set_source_cloud(const point_cloud& inputCloud) {
			sourceCloud = &inputCloud;
		}

		void SICP::set_target_cloud(const point_cloud& inputCloud) {
			targetCloud = &inputCloud;
			neighbor_tree.build(inputCloud);
		}

		template<int I>
		inline float shrink_recursion(float mu, float norm, float p, float beta_t) {
			return shrink_recursion<I - 1>(mu, norm, p, 1.f - (p / mu)*powf(norm, p - 2.f)*powf(beta_t, p - 1.f));
		}

		template<>
		inline float shrink_recursion<0>(float mu, float norm, float p, float beta_t) {
			return beta_t;
		}

		template<int I>
		inline void shrink(vector<cgv::math::fvec<float, 3>>& Z, float mu, float p) {
			float alphaA = powf((2.f / mu)*(1.f - p), 1.f / (2.f - p));
			// threshold
			float th = alphaA + (p / mu)*powf(alphaA, p - 1.f);

			for (int i = 0; i < Z.size(); ++i) {
				float n = Z[i].length();

				if (n > th) {
					Z[i] *= shrink_recursion<I>(mu, n, p, (alphaA / n + 1.f) / 2.f);
				}
				else {
					Z[i] = cgv::math::fvec<float, 3>(0, 0, 0);
				}
			}
		}

		template<int I>
		inline void shrink(vector<float>& y, float mu, float p) {
			float alphaA = powf((2.f / mu)*(1.f - p), 1.f / (2.f - p));
			// threshold
			float th = alphaA + (p / mu)*powf(alphaA, p - 1.f);

			for (int i = 0; i < y.size(); ++i) {
				float n = abs(y[i]);

				if (n > th) {
					y[i] *= shrink_recursion<I>(mu, n, p, (alphaA / n + 1.f) / 2.f);
				}
				else {
					y[i] = 0;
				}
			}
		}

		void SICP::point_to_point(vec3* X, const vec3* Y,size_t size,mat3& rotation, vec3& translation) {
			vec3 X_mean = accumulate(X, X + size, vec3(0, 0, 0)) / ((float)size);
			vec3 Y_mean = accumulate(Y, Y + size, vec3(0, 0, 0)) / ((float)size);
			
			for (int i = 0; i < size; ++i) {
				X[i] -= X_mean;
			}
			
			cgv::math::diag_mat<float> Sigma;
			cgv::math::mat<float> U, V;
			mat3 fA; fA.zeros();
			U.zeros();
			V.zeros();
			Sigma.zeros();

			float w = 1 / (float)(size);
			
			for (int y = 0; y < 3; ++y) {
				for (int x = 0; x < 3; ++x) {
					float sum = 0;
					for (int l = 0; l < size; ++l) {
						sum += X[l][y]*(Y[l][x]-Y_mean[x])*(1.f/((float)size));
					}
					fA(y, x) = sum;
				}
			}
			cgv::math::mat<float> A(3, 3, &fA(0, 0));
			cgv::math::svd(A, U, Sigma, V);
			mat3 fU(3, 3, &U(0, 0)), fV(3, 3, &V(0, 0));
			
			if (det(fU)*det(fV) < 0.f) {
				cgv::math::diag_mat<float> S(3);

				S(0) = 1.f;S(1) = 1.f; S(2) = -1.f;
				cgv::math::mat<float> R = V *S* transpose(U);
				rotation = mat3(3, 3, &R(0,0));
			}
			else {
				cgv::math::mat<float> R = V * transpose(U);
				rotation = mat3(3, 3, &R(0,0));
			}
			translation = Y_mean - rotation * X_mean;
			
			// apply transformation
			for (int i = 0; i < size; ++i) {
				X[i] = rotation * X[i] + translation;
			}

			for (int i = 0; i < size; ++i) {
				X[i] += X_mean;
			}
		}

		void SICP::point_to_plane(vec3* X,vec3* Y,vec3* N,const float* u, size_t size, mat3& rotation, vec3& translation)
		{
			typedef Eigen::Matrix<float, 6, 6> Mat66;
			typedef Eigen::Matrix<float, 6, 1> Vec6;
			typedef Eigen::Block<Mat66, 3, 3> Blk33;
			typedef Eigen::Matrix<float, 3, 1> Vec3;

			vec3 X_mean = accumulate(X, X + size, vec3(0, 0, 0)) / ((float)size);

			for (int i = 0; i < size; ++i) {
				X[i] -= X_mean;
			}

			Mat66 LHS = Mat66::Zero();
			Vec6 RHS = Vec6::Zero();
			Blk33 TL = LHS.topLeftCorner<3, 3>();
			Blk33 TR = LHS.topRightCorner<3, 3>();
			Blk33 BR = LHS.bottomRightCorner<3, 3>();
			Eigen::MatrixXf C = Eigen::MatrixXf::Zero(3, size);
			for (int i = 0; i < size; i++) {
				vec3 csp = cross(X[i], N[i]);
				C.col(i) = Eigen::Matrix<float,3,1>(csp.x(),csp.y(),csp.z());
			}
			for (int i = 0; i < size; i++) TL.selfadjointView<Eigen::Upper>().rankUpdate(C.col(i), 1);

			for (int i = 0; i < size; i++) {
				Eigen::Matrix<float, 3, 1> normal = Vec3(N[i].x(),N[i].y(),N[i].z());
				TR += (C.col(i)*normal.transpose());
			}
			for (int i = 0; i < size; i++) {
				Eigen::Matrix<float, 3, 1> normal = Vec3(N[i].x(), N[i].y(), N[i].z());
				BR.selfadjointView<Eigen::Upper>().rankUpdate(normal, 1);
			}
			for (int i = 0; i < C.cols(); i++) {
				float dist_to_plane = -( dot(X[i] - (Y[i]- X_mean),N[i]) - u[i] )*1.f;
				RHS.head<3>() += C.col(i)*dist_to_plane;
				Eigen::Matrix<float, 3, 1> normal = Vec3(N[i].x(), N[i].y(), N[i].z());
				RHS.tail<3>() += normal*dist_to_plane;
			}
			LHS = LHS.selfadjointView<Eigen::Upper>();
			Eigen::Affine3f transformation;
			Eigen::LDLT<Mat66> ldlt(LHS);
			RHS = ldlt.solve(RHS);

			transformation = Eigen::AngleAxisf(RHS(0), Eigen::Vector3f::UnitX()) *
				Eigen::AngleAxisf(RHS(1), Eigen::Vector3f::UnitY()) *
				Eigen::AngleAxisf(RHS(2), Eigen::Vector3f::UnitZ());
			transformation.translation() = RHS.tail<3>();
			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
					rotation(i, j) = transformation(i, j);
			translation = vec3(transformation.translation()(0), transformation.translation()(1), transformation.translation()(2));

			for (int i = 0; i < size; ++i) {
				X[i] = rotation * X[i] + translation + X_mean;
			}
		}

		void SICP::register_point_to_point(mat3& rotation, vec3& translation)
		{
			rotation.identity();
			translation.zeros();
			vector<Pnt> source_points(&sourceCloud->pnt(0), &sourceCloud->pnt(0) + sourceCloud->get_nr_points());
			vector<Pnt> Xo1 = source_points;
			vector<Pnt> Xo2 = source_points;
			vector<Pnt> closest_points(sourceCloud->get_nr_points());
			vector<Pnt> Z(sourceCloud->get_nr_points(), Pnt(0, 0, 0));
			vector<Pnt> lagrage_multipliers(sourceCloud->get_nr_points(), Pnt(0, 0, 0));

			for (int i = 0; i < parameters.max_runs; ++i) {
				for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
					closest_points[i] = targetCloud->pnt(neighbor_tree.find_closest(source_points[i]));
				}

				float mu = parameters.mu;
				
				for (int o = 0; o < parameters.max_outer_loop; ++o) {
					float dual = 0;
					for (int i = 0; i < parameters.max_inner_loop; ++i) {
						// update Z
						for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
							// hi = Rxi + t - yi + lamda_i/mu -> source - target + lamda/mu
							Z[i] = source_points[i] - closest_points[i] + lagrage_multipliers[i] / mu;
						}
						// shrinkage operator usually converges in three iterations (I = 3)
						shrink<3>(Z, mu, parameters.p);

						vector<vec3> U(sourceCloud->get_nr_points());
						for (int i = 0; i < U.size(); ++i) {
							// ci = yi + zi - lamda_i/mu
							U[i] = closest_points[i] + Z[i] - lagrage_multipliers[i] / mu;
						}
						// ridgid motion estimator
						mat3 rot_up;
						vec3 trans_up;
						point_to_point(source_points.data(), U.data(), sourceCloud->get_nr_points(), rot_up,trans_up);
						rotation = rot_up * rotation;
						translation += trans_up;

						dual = -numeric_limits<float>::infinity();
						for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
							dual = max((source_points[i] - Xo1[i]).length(),dual);
						}
						Xo1 = source_points;
						if (dual < parameters.stop) break;
					}
					// C Update

					vector<float> Pnorms(sourceCloud->get_nr_points());
					for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
						vec3 p = source_points[i] - closest_points[i] - Z[i];
						Pnorms[i] = p.length();
						// Step 2.3
						if (!parameters.use_penalty) {
							lagrage_multipliers[i] += mu * p;
						}
					}
					//mu Update
					if (mu < parameters.max_mu) mu *= parameters.alpha;
					/// stopping criteria
					float primal = *max_element(Pnorms.begin(),Pnorms.end());
					if (primal < parameters.stop && dual < parameters.stop) break;
				}
				float stop = -numeric_limits<float>::infinity();
				for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
					stop = max((source_points[i] - Xo2[i]).length(), stop);
				}
				Xo2 = source_points;
				if (stop < parameters.stop) break;
			}

			vec3 mean =
				  accumulate(&sourceCloud->pnt(0), &sourceCloud->pnt(0) + sourceCloud->get_nr_points(), vec3(0, 0, 0)) /
				  ((float)sourceCloud->get_nr_points());
			translation = rotation * (-mean) + translation + mean;
		}

		//unfinished
		void SICP::register_point_to_plane(mat3& rotation,vec3& translation)
		{
			rotation.identity();
			translation.zeros();
			if (!sourceCloud->has_normals()) {
				//TODO estimate normals
			}
			vector<Pnt> source_points(&sourceCloud->pnt(0), &sourceCloud->pnt(0) + sourceCloud->get_nr_points());
			vector<Pnt> Xo1 = source_points;
			vector<Pnt> Xo2 = source_points;
			vector<Pnt> closest_points_position(sourceCloud->get_nr_points());
			vector<Dir> closest_points_normal(sourceCloud->get_nr_points());
			vector<float> Z(sourceCloud->get_nr_points(), 0);
			vector<float> lagrage_multipliers(sourceCloud->get_nr_points(), 0);

			for (int i = 0; i < parameters.max_runs; ++i) {
				for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
					int c = neighbor_tree.find_closest(source_points[i]);
					closest_points_position[i] = targetCloud->pnt(c);
					closest_points_normal[i] = targetCloud->nml(c);
				}

				float mu = parameters.mu;

				for (int o = 0; o < parameters.max_outer_loop; ++o) {
					float dual = 0;
					for (int i = 0; i < parameters.max_inner_loop; ++i) {
						// update Z
						for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
							Z[i] = dot(closest_points_normal[i],(source_points[i] - closest_points_position[i])) + lagrage_multipliers[i] / mu;
						}
						// shrinkage operator usually converges in three iterations (I = 3)
						shrink<3>(Z, mu, parameters.p);

						vector<float> U(sourceCloud->get_nr_points());
						for (int i = 0; i < U.size(); ++i) {
							U[i] = Z[i] - lagrage_multipliers[i] / mu;
						}
						// ridgid motion estimator
						mat3 rot_up;
						vec3 trans_up;
						point_to_plane(source_points.data(), closest_points_position.data(), closest_points_normal.data(), U.data(), sourceCloud->get_nr_points(),rot_up,trans_up);
						rotation = rot_up * rotation;
						translation += trans_up;

						dual = -numeric_limits<float>::infinity();
						for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
							dual = max((source_points[i] - Xo1[i]).length(), dual);
						}
						Xo1 = source_points;
						if (dual < parameters.stop) break;
					}
					// C Update

					vector<float> P(sourceCloud->get_nr_points());
					for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
						float p = dot(closest_points_normal[i],(source_points[i] - closest_points_position[i])) - Z[i];
						// step 2.3
						if (!parameters.use_penalty) {
							lagrage_multipliers[i] += mu * p;
							P[i] = p;
						}
					}
					//mu Update
					if (mu < parameters.max_mu) mu *= parameters.alpha;
					/// stopping criteria
					for (auto& p : P) { p = abs(p); }
					float primal = *max_element(P.begin(), P.end());
					if (primal < parameters.stop && dual < parameters.stop) break;
				}
				float stop = -numeric_limits<float>::infinity();
				for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
					stop = max((source_points[i] - Xo2[i]).length(), stop);
				}
				Xo2 = source_points;
				if (stop < parameters.stop) break;
			}

			vec3 mean =
				  accumulate(&sourceCloud->pnt(0), &sourceCloud->pnt(0) + sourceCloud->get_nr_points(), vec3(0, 0, 0)) /
				  ((float)sourceCloud->get_nr_points());
			translation = rotation * (-mean) + translation + mean;
		}

		void SICP::register_point_cloud(ComputationMode cm, mat3& rotation, vec3& translation)
		{
			switch (cm) {
			case CM_DEFAULT:
			case CM_POINT_TO_POINT:
				register_point_to_point(rotation, translation);
				break;
			case CM_POINT_TO_PLANE:
				register_point_to_plane(rotation, translation);
			}
		}

	}
}