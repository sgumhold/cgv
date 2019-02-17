#pragma once

#include "vr_kit.h"

#include "lib_begin.h"

namespace vr {
	/**@name vr driver management*/
	//@{
	/// interface class for vr drivers, when implementing your driver, provide a constructor with a single options argument of type std::string
	class CGV_API vr_driver
	{
	protected:
		/// driver index is set during registration
		unsigned driver_index;
		/// call this method during scanning of vr kits. In case vr kit handle had been registered before, previous copy is deleted and a warning is issued
		void register_vr_kit(void* handle, vr_kit* kit);
	public:
		void set_index(unsigned _idx);
		/// declare destructor virtual to ensure it being called also for derived classes
		virtual ~vr_driver();
		/// return name of driver
		virtual std::string get_driver_name() = 0;
		/// return whether driver is installed
		virtual bool is_installed() const = 0;
		/// scan all connected vr kits and return a vector with their ids
		virtual std::vector<void*> scan_vr_kits() = 0;
		/// put a 3d up direction into passed array
		virtual void put_up_direction(float* up_dir) = 0;
		/// return the floor level relativ to the world origin
		virtual float get_floor_level() = 0;
		/// return height of interaction zone in meters
		virtual float get_interaction_zone_height() = 0;
		/// return a vector of floor points defining the interaction zone boundary as a closed polygon
		virtual void put_interaction_zone_bounary(std::vector<float>& boundary) = 0;
	};

	/// return a vector with all registered vr drivers
	extern CGV_API std::vector<vr_driver*>& get_vr_drivers();
	/// iterate all registered vr drivers to scan for vr kits and return vector of vr kit handles
	extern CGV_API std::vector<void*> scan_vr_kits();
	/// query a pointer to a vr kit by its handle, function can return null pointer in case that no vr kit exists for given handle
	extern CGV_API vr_kit* get_vr_kit(void* vr_kit_handle);

	/// register a new driver
	extern CGV_API void register_driver(vr_driver* vrd);
	/// use this template to register your own driver
	template <typename T>
	struct driver_registry
	{
		driver_registry(const std::string& options)
		{
			register_driver(new T(options));
		}
	};
}

#include <cgv/config/lib_end.h>
