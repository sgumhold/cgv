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
		virtual std::string get_driver_name() const = 0;
		/// return whether driver is installed
		virtual bool is_installed() const = 0;
		/// scan all connected vr kits and return a vector with their ids
		virtual std::vector<void*> scan_vr_kits() = 0;
		/// put a 3d x direction into passed array
		void put_x_direction(float* x_dir) const;
		/// put a 3d up direction into passed array
		virtual void put_up_direction(float* up_dir) const = 0;
		/// return the floor level relativ to the world origin
		virtual float get_floor_level() const = 0;
		/// return height of action zone in meters
		virtual float get_action_zone_height() const = 0;
		/// return a vector of floor points defining the action zone boundary as a closed polygon
		virtual void put_action_zone_bounary(std::vector<float>& boundary) const = 0;
	};

	/// return a vector with all registered vr drivers
	extern CGV_API std::vector<vr_driver*>& get_vr_drivers();
	/// iterate all registered vr drivers to scan for vr kits and return vector of vr kit handles
	extern CGV_API std::vector<void*> scan_vr_kits();
	/// query a pointer to a vr kit by its handle, function can return null pointer in case that no vr kit exists for given handle
	extern CGV_API vr_kit* get_vr_kit(void* vr_kit_handle);
	/// unregister a previously registered vr kit by handle and pointer
	extern CGV_API bool unregister_vr_kit(void* vr_kit_handle, vr_kit* vr_kit_ptr);

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
