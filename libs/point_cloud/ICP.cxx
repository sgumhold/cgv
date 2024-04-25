#include "point_cloud.h"
#include <algorithm>
#include <random>
#include <fstream>
#include "ICP.h"

namespace cgv {
	namespace pointcloud {
		ICP::ICP() {
			sourceCloud = nullptr;
			targetCloud = nullptr;
			tree = nullptr;
			crspd_source = nullptr;
			crspd_target = nullptr;
			this->maxIterations = 400;
			this->numRandomSamples = 400;
			this->eps = 1e-8;
		}

		ICP::~ICP() {

		}

		void ICP::build_ann_tree()
		{
			if (!targetCloud) {
				std::cerr << "ICP::build_ann_tree: target cloud missing, can't build ann tree\n";
				return;
			}

			tree = std::make_shared<ann_tree>();
			tree->build(*targetCloud);
		}

		void ICP::clear()
		{
			tree = nullptr;
		}

		void ICP::set_source_cloud(const point_cloud& inputCloud) {
			sourceCloud = &inputCloud;
		}

		void ICP::set_target_cloud(const point_cloud& inputCloud, std::shared_ptr<ann_tree> precomputed_tree) {
			targetCloud = &inputCloud;
			if (precomputed_tree)
				tree = precomputed_tree;
		}

		void ICP::set_iterations(int Iter) {
			this->maxIterations = Iter;
		}

		void ICP::set_num_random(int NR) {
			this->numRandomSamples = NR;
		}

		void ICP::set_eps(float e) {
			this->eps = e;
		}

		///output the rotation matrix and translation vector
		void ICP::reg_icp(Mat& rotation_mat, Dir& translation_vec)
		{
			if (!tree) {
				/// create the ann tree
				build_ann_tree();
				//std::cerr << "ICP::reg_icp: called reg_icp before initialization!\n";
			}
			if (!(sourceCloud && targetCloud)) {
				std::cerr << "ICP::reg_icp: source or target cloud not set!\n";
				return;
			}
			Pnt source_center;
			Pnt target_center;
			source_center.zeros();
			target_center.zeros();
			size_t num_source_points = sourceCloud->get_nr_points();

			float cost = 1.0;
			///initialize as Infinity
			float last_error = std::numeric_limits<float>::infinity();

			Mat fA(0.0f);             // this initializes fA to matrix filled with zeros

			cgv::math::mat<float> U, V;
			cgv::math::diag_mat<float> Sigma;
			/*
			U.zeros();
			V.zeros();
			Sigma.zeros();
			*/
			point_cloud S, Q;


			/// sample the source point cloud
			if (numRandomSamples > 0 && numRandomSamples < sourceCloud->get_nr_points()) {
				std::vector<int> numbers(sourceCloud->get_nr_points());
				for (int i = 0; i < sourceCloud->get_nr_points(); ++i)
					numbers[i] = i;
				std::random_device rd;
				std::default_random_engine rng(std::time(0));
				std::shuffle(numbers.begin(), numbers.end(), rng);

				S.resize(numRandomSamples);
				Q.resize(numRandomSamples);
				std::srand(std::time(0));
				for (int i = 0; i < numRandomSamples; i++)
					S.pnt(i) = sourceCloud->pnt(numbers[i]);
			}
			else {
				S.resize(sourceCloud->get_nr_points());
				Q.resize(sourceCloud->get_nr_points());
				for (int i = 0; i < sourceCloud->get_nr_points(); i++)
					S.pnt(i) = sourceCloud->pnt(i);
			}

			Mat rotation_update_mat = rotation_mat;
			Dir translation_update_vec = translation_vec;

			for (int iter = 0; iter < maxIterations; iter++)
			{
				// update S
				for (int i = 0; i < S.get_nr_points(); i++)
					S.pnt(i) = rotation_update_mat * S.pnt(i) + translation_update_vec;
				// update center
				get_center_point(S, source_center);
			
				fA.zeros();
				for (int i = 0; i < S.get_nr_points(); i++)
				{
					/// get the closest point to p from the target point cloud
					Pnt p = S.pnt(i);
					Pnt q = targetCloud->pnt(tree->find_closest(p));
					Q.pnt(i) = q; 
					fA += Mat(q - target_center, p - source_center);
				}
				get_center_point(Q, target_center);

				///cast fA to A
				cgv::math::mat<float> A(3, 3, &fA(0, 0));
				cgv::math::svd(A, U, Sigma, V, false);
				Mat fU(3, 3, &U(0, 0)), fV(3, 3, &V(0, 0)), m;
				float det = cgv::math::det(V * cgv::math::transpose(U));
				m.identity();
				m(2, 2) = det;
				///get new R and t
				//Mat rotation_update_mat = fU * cgv::math::transpose(fV);
				rotation_update_mat = fV * m * cgv::math::transpose(fU);
				rotation_update_mat.transpose();

				translation_update_vec = target_center - rotation_update_mat * source_center;
				///calculate error function E(R,t)
				cost = 0.f;
				for (int i = 0; i < S.get_nr_points(); i++) {
					///the new rotation matrix: rotation_update_mat
					cost += error(Q.pnt(i), S.pnt(i), rotation_update_mat, translation_update_vec);
				}
				cost /= S.get_nr_points();
				///judge if cost is decreasing, and is larger than eps. If so, update the R and t, otherwise stop and output R and t
				
				/// update the R and t		
				rotation_mat = rotation_update_mat * rotation_mat;
				translation_vec = rotation_update_mat * translation_vec + translation_update_vec;
				
				if (abs(last_error-cost) <= eps)
					break;
				last_error = cost;
			}
			//std::cout << "rotate_mat: " << rotation_mat << std::endl;
			//std::cout << "translation_vec: " << translation_vec << std::endl;
			//print_rotation(rotation_mat);
			//print_translation(translation_vec);
		}

		void ICP::get_center_point(const point_cloud& input, Pnt& center_point) {
			center_point.zeros();
			for (unsigned int i = 0; i < input.get_nr_points(); i++)
				center_point += input.pnt(i);
			center_point /= (float)input.get_nr_points();
		}

		float ICP::error(Pnt& ps, Pnt& pd, Mat& r, Dir& t)
		{
			Pnt tmp = ps - r * pd - t;
			return dot(tmp, tmp);
		}

		void ICP::get_crspd(Mat& rotation_mat, Dir& translation_vec, point_cloud& pc1, point_cloud& pc2)
		{
			Pnt source_center;
			Pnt target_center;
			source_center.zeros();
			target_center.zeros();
			/// create the ann tree
			ann_tree* tree = new ann_tree();
			tree->build(*targetCloud);
			size_t num_source_points = sourceCloud->get_nr_points();
			
			get_center_point(*targetCloud, target_center);
			get_center_point(*sourceCloud, source_center);
			Dir trans_center;
			if (target_center.x() - source_center.x() > eps || target_center.y() - source_center.y() > eps ||
				target_center.z() - source_center.z() > eps)
			{
				trans_center = Dir(target_center.x() - source_center.x(), target_center.y() - source_center.y(),
								   target_center.z() - source_center.z());
			}
			//std::cout << "target_center: " << target_center << "source_center: " << source_center << std::endl;
			Pnt p;

			float cost = 1.0;
			///define min as Infinity
			float min = DBL_MAX;
			std::srand(std::time(0));

			Mat fA(0.0f);             // this initializes fA to matrix filled with zeros

			cgv::math::mat<float> U, V;
			cgv::math::diag_mat<float> Sigma;
			U.zeros();
			V.zeros();
			Sigma.zeros();
			pc1.clear();
			pc2.clear();
			for (int iter = 0; iter < maxIterations && abs(cost) > eps; iter++)
			{
				cost = 0.0;
				point_cloud Q, S;
				S.clear();
				Q.clear();
				source_center = rotation_mat * source_center + translation_vec;
				fA.zeros();
				for (int i = 0; i < sourceCloud->get_nr_points(); i++)
				{
					//int randSample = std::rand() % sourceCloud->get_nr_points();
					/// sample the source point cloud
					//S.pnt(i) = sourceCloud->pnt(randSample);
					//Q.pnt(i) = targetCloud->pnt(tree->find_closest(S.pnt(i)));
					Pnt temp_s(0.0);
					Pnt temp_q(0.0);
					if (correspondences_filter(*sourceCloud, *targetCloud, temp_s, temp_q))
					{
						S.add_point(temp_s);
						Q.add_point(temp_q);
						fA += Mat(temp_q - target_center, rotation_mat * temp_s + translation_vec - source_center);
					}
					/// get the closest point in the target point cloud
					//Q.pnt(i) = targetCloud->pnt(i);
					//fA += Mat(Q.pnt(i) - target_center, rotation_mat * S.pnt(i) + translation_vec - source_center);
				}
				///cast fA to A
				cgv::math::mat<float> A(3, 3, &fA(0, 0));
				cgv::math::svd(A, U, Sigma, V);
				Mat fU(3, 3, &U(0, 0)), fV(3, 3, &V(0, 0));
				///get new R and t
				Mat rotation_update_mat = fU * cgv::math::transpose(fV);
				Dir translation_update_vec = target_center - rotation_update_mat * source_center;
				///calculate error function E(R,t)
				for (int i = 0; i < S.get_nr_points(); i++) {
					///transform Pi to R*Pi + t
					S.pnt(i) = rotation_mat * S.pnt(i) + translation_vec;
					///the new rotation matrix: rotation_update_mat
					//float tempcost = error(Q.pnt(i), S.pnt(i), rotation_update_mat, translation_update_vec);
					cost += error(Q.pnt(i), S.pnt(i), rotation_update_mat, translation_update_vec);
				}
				cost /= sourceCloud->get_nr_points();
				std::cout << "no:" << iter  <<"cost: " << cost << std::endl;
				///judge if cost is decreasing, and is larger than eps. If so, update the R and t, otherwise stop and output R and t
				if (min >= abs(cost)) {
					///update the R and t
					rotation_mat = rotation_update_mat * rotation_mat;
					translation_vec = rotation_update_mat * translation_vec + translation_update_vec;
					//std::cout << "no:" << iter << "cost: " << cost << std::endl;
					min = abs(cost);
				}
				else {
					break;
				}
				// this is for drawing corresponding lines after transforming
				pc1.clear();
				pc2.clear();
				for (int i = 0; i < S.get_nr_points(); i++) {
					// this is for drawing corresponding lines after transforming
					S.pnt(i) = rotation_mat * S.pnt(i) + translation_vec;
					pc1.add_point(S.pnt(i));
				}
				for (int i = 0; i < Q.get_nr_points(); i++)
					pc2.add_point(Q.pnt(i));
			}
			std::cout << "rotate_mat: \n" << rotation_mat << std::endl;
			std::cout << "translation_vec: \n" << translation_vec << std::endl;
			delete tree;
		}
		///print rotation matrix
		void ICP::print_rotation(float* rotation) {
			std::cout << "rotation" << std::endl;
			for (int i = 0; i < 9; i = i + 3) {
				std::cout << rotation[i] << " " << rotation[i + 1] << " " << rotation[i + 2] << std::endl;
			}
		}
		///print translation vector
		void ICP::print_translation(float* translation) {
			std::cout << "translation" << std::endl;
			std::cout << translation[0] << " " << translation[1] << " " << translation[2] << std::endl;
		}

		bool ICP::correspondences_filter(const point_cloud& source, const point_cloud& target, Pnt& source_p, Pnt& target_p)
		{
			ann_tree* tree = new ann_tree();
			tree->build(target);
			ann_tree* tree_inv = new ann_tree();
			tree_inv->build(source);
			float dist, dist_inv = 0.0;
			int randSample = std::rand() % source.get_nr_points();
			source_p = source.pnt(randSample);
			target_p = target.pnt(tree->find_closest(source_p));
			//std::cout << "source_p: " << source_p << "target_p: " << target_p << std::endl;
			dist = dis_pts(source_p, target_p);
			Pnt source_p_inv = source.pnt(tree_inv->find_closest(target_p));
			dist_inv = dis_pts(source_p_inv, target_p);
			if (dist > 1.5 * dist_inv || dist < 0.667 * dist_inv)
			{
				return false;
			}
			return true;
		}

		float ICP::dis_pts(const Pnt& source_p, const Pnt& target_p)
		{
			float dist = 0.0;
			dist = sqrt(pow((source_p.x() - target_p.x()), 2) + pow((source_p.y() - target_p.y()), 2) + pow((source_p.z() - target_p.z()), 2));
			return dist;
		}
	}
}