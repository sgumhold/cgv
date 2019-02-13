#include "vr_driver.h"
#include <map>
#include <iostream>

namespace vr {
	/// access to vr_kit map singleton 
	std::map<void*, vr_kit*>& ref_vr_kit_map()
	{
		static std::map<void*, vr_kit*> vr_kit_map;
		return vr_kit_map;
	}

	/// declare destructor virtual to ensure it being called also for derived classes
	vr_driver::~vr_driver()
	{

	}

	/// call this method during scanning of vr kits but only in case vr kit id does not yield vr kit through global get_vr_kit function
	void vr_driver::register_vr_kit(void* handle, vr_kit* kit)
	{
		auto iter = ref_vr_kit_map().find(handle);
		if (iter != ref_vr_kit_map().end()) {
			std::cerr << "WARNING: vr kit <" << kit->get_name() << "> recreated. Deleted copy <" 
				<< iter->second->get_name() << "> might be still in use." << std::endl;
			delete iter->second;
			iter->second = kit;
		}
		else {
			ref_vr_kit_map()[handle] = kit;
		}
	}

	void vr_driver::set_index(unsigned _idx)
	{
		driver_index = _idx;
	}

	/// return registered drivers
	std::vector<vr_driver*>& ref_drivers()
	{
		static std::vector<vr_driver*> drivers;
		return drivers;
	}

	/// register a new driver
	void register_driver(vr_driver* vrd)
	{
		vrd->set_index(ref_drivers().size());
		ref_drivers().push_back(vrd);
	}
	/// return a vector with all registered vr drivers
	std::vector<vr_driver*>& get_vr_drivers()
	{
		return ref_drivers();
	}

	/// scan all registered vr drivers for vr kits and return vector of vr kit ids
	std::vector<void*> scan_vr_kits()
	{
		ref_vr_kit_map().clear();
		std::vector<void*> kit_handles;
		for (auto driver_ptr : ref_drivers()) {
			auto new_ids = driver_ptr->scan_vr_kits();
			kit_handles.insert(kit_handles.end(), new_ids.begin(), new_ids.end());
		}
		return kit_handles;
	}
	/// query a pointer to a vr kit by its id, function can return null pointer in case that no vr kit exists for given id
	vr_kit* get_vr_kit(void* handle)
	{
		auto iter = ref_vr_kit_map().find(handle);
		if (iter == ref_vr_kit_map().end())
			return NULL;
		return iter->second;
	}
}