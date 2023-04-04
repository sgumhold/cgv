#pragma once

#include <vector>
#include "color_storage.h"

#include "lib_begin.h"

namespace cgv {
	namespace media {
		/** coordinate type independent base class of simple mesh data structure that handles indices and colors. */
		class CGV_API colored_model : public color_storage_types
		{
		protected:
			/// pointer to color storage
			abst_color_storage* color_storage_ptr;
		public:
			/// construct colored model
			colored_model();
			/// copy constructur
			colored_model(const colored_model& cm);
			/// move constructur
			colored_model(colored_model&& cm);
			/// assignment operator
			colored_model& operator=(const colored_model& cm);
			/// move assignment operator
			colored_model& operator=(colored_model&& cm);
			/// destruct colored model
			virtual ~colored_model();
			/**@name access to colors*/
			//@{
			/// check whether colors have been allocated
			bool has_colors() const;
			/// set i-th color to color of type stored in storage
			void set_color(size_t i, const void* col_ptr);
			/// set i-th color from color of type rgb
			void set_color(size_t i, const rgb& col);
			/// set i-th color from color of type rgba
			void set_color(size_t i, const rgb8& col);
			/// set i-th color from color of type rgb8
			void set_color(size_t i, const rgba& col);
			/// set i-th color from color of type rgba8
			void set_color(size_t i, const rgba8& col);
			/// set color of type stored in storage to i-th color
			void put_color(size_t i, void* col_ptr) const;
			/// set color of type rgb to i-th color
			void put_color(size_t i, rgb& col) const;
			/// set color of type rgba to i-th color
			void put_color(size_t i, rgba& col) const;
			/// set color of type rgb8 to i-th color
			void put_color(size_t i, rgb8& col) const;
			/// set color of type rgba8 to i-th color
			void put_color(size_t i, rgba8& col) const;
			/// return number of allocated colors
			size_t get_nr_colors() const;
			/// resize the color storage to given number of colors
			void resize_colors(size_t nr_colors);
			/// return the size of one allocated color in byte
			size_t get_color_size() const;
			/// return storage type of colors, if no colors are allocated CT_RGBA8 is returned
			ColorType get_color_storage_type() const;

			const void* get_color_data_ptr() const;
			const void* get_color_data_vector_ptr() const;

			//! ensure that colors are allocated and of given storage type
			/*! Only in case of new allocation, the second parameter is used to define the number of colors */
			void ensure_colors(ColorType _color_type, size_t nr_colors = -1);
			/// destruct color storage
			void destruct_colors();
		};
	}
}

#include <cgv/config/lib_end.h>