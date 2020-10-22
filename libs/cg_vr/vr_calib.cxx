#include "vr_calib.h"
#include <vr/vr_driver.h>
#include <cgv/utils/file.h>
#include <sstream>
#include <fstream>
#include <cgv/utils/advanced_scan.h>
#include <cgv/math/pose.h>

namespace cgv {
	namespace gui {
		vr_calibration& ref_vr_calibration()
		{
			static vr_calibration singleton;
			return singleton;
		}
		void vr_calibration::update_calibration_info()
		{
			for (auto dp : vr::get_vr_drivers()) {
				if (!dp->is_calibration_transformation_enabled())
					continue;
				calibration_info[dp->get_driver_name()] = dp->get_tracking_reference_states();
				float trans[12];
				dp->put_calibration_transformation(trans);
				std::cout << dp->get_driver_name() << ":";
				for (int i = 0; i < 12; ++i)
					std::cout << " " << trans[i];
				std::cout << std::endl;
			}
		}
		/// update the calibration of a driver from the given target reference states
		bool vr_calibration::update_driver_calibration(vr::vr_driver* dp, const std::map<std::string, vr::vr_trackable_state>& target_reference_states) const
		{
			
			std::vector<cgv::math::quaternion<float> > relative_rotations;
			std::vector<cgv::math::fvec<float, 3> > relative_translations;
			
			if (dp->get_tracking_reference_states().empty()) {
				std::vector<void*> handles = dp->scan_vr_kits();
				if (!handles.empty()) {
					vr::vr_kit_state state;
					vr::get_vr_kit(handles.front())->query_state(state, 2);
				}
			}
			const auto& refs = dp->get_tracking_reference_states();

			float current_transform_array[12];
			auto& current_transform = reinterpret_cast<cgv::math::fmat<float, 3, 4>&>(current_transform_array[0]);
			dp->put_calibration_transformation(current_transform_array);

			std::cout << "update " << dp->get_driver_name() << std::endl;
			std::cout << "current :";
			for (int i = 0; i < 12; ++i)
				std::cout << " " << current_transform_array[i];
			std::cout << std::endl;

			// iterate target states and generate a relative transformation for each matched serial 
			for (const auto& target : target_reference_states) {
				// ensure that serial is registered in driver
				auto iter = refs.find(target.first);
				if (iter == refs.end())
					continue;
				// ensure that base station is tracked
				if (iter->second.status != vr::VRS_TRACKED)
					continue;
				// compute calibration rotation and translation necessary to achieve target pose
				const auto& target_pose = reinterpret_cast<const cgv::math::fmat<float, 3, 4>&>(target.second.pose[0]);
				const auto& pose = reinterpret_cast<const cgv::math::fmat<float, 3, 4>&>(iter->second.pose[0]);
				cgv::math::fmat<float, 3, 4> relative_transformation = target_pose;
				pose_append(relative_transformation, pose_inverse(pose));
				relative_rotations.push_back(cgv::math::quaternion<float>(pose_orientation(relative_transformation)));
				relative_translations.push_back(pose_position(relative_transformation));
			}
			// compute average over all transformations
			cgv::math::quaternion<float> avg_rel_rot(0.0f, 0.0f, 0.0f, 0.0f);
			cgv::math::fvec<float, 3> avg_rel_tra(0.0f);
			for (const auto& rr : relative_rotations)
				avg_rel_rot += rr;
			avg_rel_rot.normalize();
			for (const auto& rt : relative_translations)
				avg_rel_tra += rt;
			avg_rel_tra *= 1.0f / float(relative_translations.size());
			// validate consistency
			bool consistent = true;
			for (unsigned i = 0; i < relative_rotations.size(); ++i) {
				if (length(reinterpret_cast<const cgv::math::fvec<float,4>&>(relative_rotations[i]) - avg_rel_rot) > 0.01f) {
					consistent = false;
					break;
				}
				if (length(relative_translations[i] - avg_rel_tra) > 0.02f) {
					consistent = false;
					break;
				}
			}
			if (consistent) {
				cgv::math::fmat<float, 3, 4> relative_transformation = pose_construct(avg_rel_rot, avg_rel_tra);
				pose_transform(relative_transformation, current_transform);
				dp->enable_calibration_transformation();
				set_driver_calibration_matrix(dp, current_transform_array);
				std::cout << "relative :" << relative_transformation << std::endl;
				std::cout << "new    :";
				for (int i = 0; i < 12; ++i)
					std::cout << " " << current_transform_array[i];
				std::cout << std::endl;
				return true;
			}
			return false;
		}
		/// update the calibration of a driver from the given target reference states
		bool vr_calibration::update_driver_calibration(vr::vr_driver* dp) const
		{
			auto iter = calibration_info.find(dp->get_driver_name());
			// ensure that new calibration has been read for this driver
			if (iter == calibration_info.end())
				return false;
			return update_driver_calibration(dp, iter->second);
		}
		vr_calibration::vr_calibration()
		{
		}
		bool vr_calibration::read_calibration(const std::string& file_path, bool update_drivers)
		{
			// read calibration file
			std::string content;
			if (!cgv::utils::file::read(file_path, content, true))
				return false;
			// extract calibration information into temporary map
			std::map<std::string, std::map<std::string, vr::vr_trackable_state> > read_calibration_info;
			std::vector<cgv::utils::line> lines;
			cgv::utils::split_to_lines(content, lines);
			std::string driver_name;
			for (auto l : lines) {
				if (l.size() < 2)
					continue;
				if (l[1] != ' ')
					continue;
				switch (l[0]) {
				case 'd' :
					driver_name = std::string(l.begin + 3, l.size() - 4);
					break;
				case 'b' :
				{
					cgv::math::quaternion<float> orientation;
					cgv::math::fvec<float, 3> position;
					std::stringstream ss(std::string(l.begin + 2, l.size() - 2));
					ss >> orientation >> position;
					char rest_of_line[500];
					ss.getline(&rest_of_line[0], 500);
					std::string serial = std::string(rest_of_line+1);
					if (!serial.empty()) {
						if (serial[0] == '"')
							serial = serial.substr(1, serial.size() - 2);
						if (!serial.empty()) {
							auto& pose = reinterpret_cast<cgv::math::fmat<float,3,4>&>(read_calibration_info[driver_name][serial].pose[0]);
							orientation.normalize();
							orientation.put_matrix(pose_orientation(pose));
							pose_position(pose) = position;
						}
					}
				}
				}
			}
			// copy read information over to calibration information 
			for (const auto& driver_info : read_calibration_info)
				calibration_info[driver_info.first] = driver_info.second;

			// finally iterate drivers and update driver calibrations
			if (update_drivers) {
				for (auto dp : vr::get_vr_drivers()) {
					auto iter = read_calibration_info.find(dp->get_driver_name());
					// ensure that new calibration has been read for this driver
					if (iter == read_calibration_info.end())
						continue;
					// 
					update_driver_calibration(dp, iter->second);
				}
			}
			return true;
		}
		bool vr_calibration::write_calibration(const std::string& file_path) const
		{
			std::ofstream os(file_path);
			if (os.fail())
				return false;
			for (const auto& di : calibration_info) {
				os << "d \"" << di.first << "\"\n";
				for (const auto& s : di.second) {
					os << "b "; 
					const auto& pose = reinterpret_cast<const cgv::math::fmat<float, 3, 4>&>(s.second.pose[0]);
					cgv::math::quaternion<float> orientation(pose_orientation(pose));
					os << orientation << " " << pose_position(pose) << " \"" << s.first << "\"\n";
				}
			}
			return true;
		}
	}
}