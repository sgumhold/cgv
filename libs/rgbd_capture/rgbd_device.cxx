#include <algorithm>
#include "rgbd_device.h"

namespace rgbd {

	mesh_data_view::mesh_data_view(char* data, const size_t size) {
		this->parse_data(data, size);
	}

	bool mesh_data_view::parse_data(char* data, const size_t size) {
		auto lambda = [&]() {
			if (size < sizeof(uint32_t)) {
				return false;
			}
			this->points_size = 0;
			this->triangles_size = 0;
			this->uv_size = 0;
			//how many points the mesh has
			memcpy(&this->points_size, data, sizeof(uint32_t));
			size_t offset = sizeof(uint32_t);
			this->points = reinterpret_cast<Point*>(data + offset);

			///the number of triangles is located behind the points array
			offset += this->points_size * sizeof(Point);
			if (size <= offset + sizeof(uint32_t)) {
				return (size == offset); //in this case, data only has points
			}
			memcpy(&this->triangles_size, data + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			this->triangles = reinterpret_cast<Triangle*>(data + offset);

			///the number of texture coordinates is located behind the triangles array
			if (size <= offset + sizeof(uint32_t)) {
				return (size == offset); //true if data only has points and triangles
			}
			offset += this->triangles_size * sizeof(Triangle);
			memcpy(&this->uv_size, data + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			this->uv = reinterpret_cast<TextureCoord*>(data + offset);
			//final size
			offset += sizeof(TextureCoord)*this->uv_size;
			return size >= offset;
		};

		if (!lambda()) {
			points = nullptr;
			return false;
		}
		return true;
	}
	rgbd_device::~rgbd_device()
	{
	}
	std::string rgbd_device::get_audio_device() const
	{
		return "";
	}
	const std::vector<color_parameter_info>& rgbd_device::get_supported_color_control_parameter_infos() const
	{
		static std::vector<color_parameter_info> I;
		return I;
	}
	std::pair<int32_t, bool> rgbd_device::get_color_control_parameter(ColorControlParameter ccp) const
	{
		return std::pair<int32_t, bool>(-1, false);
	}
	bool rgbd_device::set_color_control_parameter(ColorControlParameter ccp, int32_t value, bool automatic_mode)
	{
		return false;
	}

	/// return whether rgbd device has support for view finding actuator
	bool rgbd_device::has_view_finder() const
	{
		return false;
	}
	/// return a view finder info structure
	const view_finder_info& rgbd_device::get_view_finder_info() const
	{
		static view_finder_info info;
		info.degrees_of_freedom = 0;
		info.max_angle[0] = info.max_angle[1] = info.max_angle[2] = 0;
		info.min_angle[0] = info.min_angle[1] = info.min_angle[2] = 0;
		return info;
	}
	/// set the pitch position of the rgbd device in degrees with 0 as middle position, return whether this was successful
	bool rgbd_device::set_pitch(float y)
	{
		return false;
	}
	/// return the pitch position of the rgbd device in degrees with 0 as middle position
	float rgbd_device::get_pitch() const
	{
		return 0;
	}

	/// whether rgbd device has support for a near field depth mode
	bool rgbd_device::has_near_mode() const
	{
		return false;
	}
	/// return whether the near field depth mode is activated
	bool rgbd_device::get_near_mode() const
	{
		return false;
	}
	/// activate or deactivate near field depth mode, return whether this was successful
	bool rgbd_device::set_near_mode(bool on)
	{
		return false;
	}
	/// check whether a multi-device role is supported
	bool rgbd_device::is_supported(MultiDeviceRole mdr) const
	{
		return mdr == MDR_STANDALONE;
	}
	/// configure device for a multi-device role and return whether this was successful (do this before starting)
	bool rgbd_device::configure_role(MultiDeviceRole mdr)
	{
		if (!is_supported(mdr))
			return false;
		multi_device_role = mdr;
		return true;
	}

	bool rgbd_device::has_IMU() const
	{
		return false;
	}
	const IMU_info& rgbd_device::get_IMU_info() const
	{
		static IMU_info info;
		info.has_angular_acceleration = info.has_linear_acceleration = info.has_time_stamp_support = false;
		return info;
	}
	bool rgbd_device::put_IMU_measurement(IMU_measurement& m, unsigned time_out) const
	{
		return false;
	}
	bool rgbd_device::query_calibration(rgbd_calibration& calib)
	{
		return false;
	}
	bool rgbd_device::get_emulator_configuration(emulator_parameters& cfg) const
	{
		return false;
	}
}