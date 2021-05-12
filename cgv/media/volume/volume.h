#pragma once

#include <cgv/math/fvec.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/data/data_view.h>
#include <cgv/type/info/type_id.h>

#include "../lib_begin.h"

namespace cgv {
	namespace media {
		namespace volume {

			class CGV_API volume
			{
			public:
				typedef float coord_type;
				typedef cgv::math::fvec<int, 3> index_type;
				typedef cgv::math::fvec<int, 3> dimension_type;
				typedef cgv::math::fvec<coord_type, 3> point_type;
				typedef cgv::math::fvec<coord_type, 3> extent_type;
				typedef cgv::media::axis_aligned_box<coord_type, 3> box_type;
			protected:
				/// format description of volume data
				cgv::data::data_format df;
				/// data storage of volume data
				cgv::data::data_view dv;
				/// extent of the volume in each coordinate direction
				extent_type extent;
			public:
				/// construct empty volume with unit cube as box and "uint8[L]" as component type
				volume();
				/// copy construct volume by allocating a copy of the volume data
				volume(const volume& V);
				/// destruct
				~volume() { clear(); }
				/// return whether volume is empty
				bool empty() const { return get_dimensions() == dimension_type(0, 0, 0); }
				/// deallocate all memory and reset data format to "uint8[L]"
				void clear() { dv = cgv::data::data_view(); df = cgv::data::data_format(); }
				/// return const reference to data format
				const cgv::data::data_format& get_format() const { return df; }
				/// return reference to data format
				cgv::data::data_format& get_format() { return df; }

				/**@name access to voxel format (components and type)*/
				//@{
				/// return the component type
				cgv::type::info::TypeId get_component_type() const { return df.get_component_type(); }
				/// return component format
				cgv::data::ComponentFormat get_component_format() const { return df.get_standard_component_format(); }
				/// set a different component format
				void set_component_format(cgv::data::ComponentFormat cf) { df.set_component_format(cgv::data::component_format(df.get_component_type(), cf)); }
				/// set the value type of the voxel components
				void set_component_type(cgv::type::info::TypeId type_id) { df.set_component_type(type_id); }
				/// set the component format from a string according to the syntax declared in <cgv/data/component_format.h>
				void set_component_format(const std::string& format) { df.set_component_format(cgv::data::component_format(format)); }
				/// return the number of components within a voxel
				unsigned get_nr_components()  const { return df.get_nr_components(); }
				/// return the size of a voxel component in bytes
				unsigned get_component_size() const { return cgv::type::info::get_type_size(get_component_type()); }
				/// return the size of a voxel in bytes
				unsigned get_voxel_size() const { return df.get_component_format().get_entry_size(); }
				/// return the size of a row within a slice in bytes
				size_t get_row_size() const { return df.get_width() * get_voxel_size(); }
				/// return the size of a slice in bytes
				size_t get_slice_size() const { return df.get_height() * get_row_size(); }
				/// return size of volume in bytes
				size_t get_size() const { return get_voxel_size() * get_nr_voxels(); }
				//@}

				/**@name access to sampling resolution*/
				//@{
				/// return the dimensions or (0,0,0) if not available
				virtual dimension_type get_dimensions() const;
				/// return the total number of voxels
				size_t get_nr_voxels() const;
				/// resize the volume
				virtual void resize(const dimension_type& S);
				//@}

				/**@name access to extent of volume*/
				//@{
				/// return the bounding box in volume coordinates, which is computed from spacing and dimensions and centered around the origin
				box_type get_box() const;
				/// return const reference to spatial extent
				const extent_type& get_extent() const { return extent; }
				/// return reference spacing
				extent_type& ref_extent() { return extent; }
				/// return the voxel spacing, computed by extent/dimensions
				extent_type get_spacing() const;
				//@}

				/**@name access to volume data*/
				//@{
				/// return a const reference to the data view
				const cgv::data::data_view& get_data_view() const { return dv; }
				/// return a reference to the data view
				cgv::data::data_view& get_data_view() { return dv; }
				/// return a const pointer to the data
				template <typename T>
				const T* get_data_ptr() const { return dv.get_ptr<T>(); }
				/// return a pointer to the data
				template <typename T>
				T* get_data_ptr() { return dv.get_ptr<T>(); }
				/// return a const pointer to the data of the k-th slice
				template <typename T>
				const T* get_slice_ptr(unsigned k) const { return dv.get_ptr<T>(k); }
				/// return a pointer to the data of the k-th slice
				template <typename T>
				T* get_slice_ptr(unsigned k) { return dv.get_ptr<T>(k); }
				/// return a const pointer to the data of the j-th row in the k-th slice
				template <typename T>
				const T* get_row_ptr(unsigned j, unsigned k) const { return dv.get_ptr<T>(k, j); }
				/// return a pointer to the data of the j-th row in the k-th slice
				template <typename T>
				T* get_row_ptr(unsigned j, unsigned k) { return dv.get_ptr<T>(k, j); }
				/// return a const pointer to the component data of voxel (i,j,k)
				template <typename T>
				const T* get_voxel_ptr(unsigned i, unsigned j, unsigned k) const { return dv.get_ptr<T>(k, j, i); }
				/// return a pointer to the component data of voxel (i,j,k)
				template <typename T>
				T* get_voxel_ptr(unsigned i, unsigned j, unsigned k) { return dv.get_ptr<T>(k, j, i); }
				/// return a voxel component converted to type T
				template <typename T>
				T get_voxel_component(unsigned i, unsigned j, unsigned k, unsigned ci = 0) { return dv.get<T>(ci, k, j, i); }
				//@}

				/**@name adding and replacing components of volume data*/
				//@{
				/// add new component on the end (if result has <=4 components)
				bool add_new_component(data::data_view& component_dv);
				/// replace the i-th component in the volume data by the given data
				bool replace_component(unsigned i, data::data_view& component_dv);
				//@}
			};
		}
	}
}

#include <cgv/config/lib_end.h>