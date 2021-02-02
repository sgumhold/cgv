#pragma once

#include "volume.h"
#include "volume_io.h"


#include "../lib_begin.h"

namespace cgv {
	namespace media {
		namespace volume {

			/** the ooc_sliced_volume data structure allows random access to a volume that is stored as per slice file on disk.
				The data structure keeps a data format and a data view inherited from the base class volume and uses it to store
				a single slice. */
			struct CGV_API ooc_sliced_volume : public volume
			{
			public:
				unsigned nr_slices;
				unsigned offset;
				std::string file_name_pattern;
			public:
				ooc_sliced_volume();
				std::string get_slice_file_name(int i) const;
				unsigned get_nr_slices() const;
				/// overload to compose dimensions from slice format and number dimensions
				dimension_type get_dimensions() const;
				/// resize the data view to hold a slice and set the total number of slices as well
				void resize(const dimension_type& S);
				/// open for read
				bool open_read(const std::string& file_name);
				///
				bool is_open() const;
				bool read_slice(int i, const std::string& slice_file_name = "");
				/// open for write, set the data format to the slice format (width, height, components, component type) before calling this function
				bool open_write(const std::string& file_name, const std::string& _file_name_pattern, unsigned nr_slices);
				/// set the content of the data view through ref_data
				bool write_slice(int i) const;
				void close();
			};

			struct CGV_API sliced_volume_info : public volume_info
			{
				std::string file_name_pattern;
				int offset;
				sliced_volume_info();
			};

			extern CGV_API bool read_sliced_header(const std::string& file_name, sliced_volume_info& info);

			extern CGV_API bool write_sliced_header(const std::string& file_name, const ooc_sliced_volume& V);
		}
	}
}

#include <cgv/config/lib_end.h>