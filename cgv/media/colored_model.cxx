#include "colored_model.h"
#include <cassert>

namespace cgv {
	namespace media {

		/// construct colored model
		colored_model::colored_model()
		{
			color_storage_ptr = 0;
		}
		/// copy constructur
		colored_model::colored_model(const colored_model& cm)
		{
			color_storage_ptr = cm.color_storage_ptr ? cm.color_storage_ptr->clone() : 0;
		}
		/// move constructur
		colored_model::colored_model(colored_model&& cm)
		{
			color_storage_ptr = cm.color_storage_ptr;
			cm.color_storage_ptr = nullptr;
		}
		/// assignment operator
		colored_model& colored_model::operator = (const colored_model& cm)
		{
			color_storage_ptr = cm.color_storage_ptr ? cm.color_storage_ptr->clone() : 0;
			return *this;
		}
		/// move assignment operator
		colored_model& colored_model::operator = (colored_model&& cm)
		{
			color_storage_ptr = cm.color_storage_ptr;
			cm.color_storage_ptr = nullptr;
			return *this;
		}
		/// destruct colored model
		colored_model::~colored_model()
		{
			if (color_storage_ptr) {
				delete color_storage_ptr;
				color_storage_ptr = 0;
			}
		}

		bool colored_model::has_colors() const
		{
			return color_storage_ptr != 0;
		}

		void colored_model::set_color(size_t i, const void* col_ptr)
		{
			assert(has_colors());
			color_storage_ptr->set_color(i, col_ptr);
		}
		void colored_model::set_color(size_t i, const rgb& col)
		{
			assert(has_colors());
			color_storage_ptr->set_color(i, col);
		}
		void colored_model::set_color(size_t i, const rgba& col)
		{
			assert(has_colors());
			color_storage_ptr->set_color(i, col);
		}
		void colored_model::set_color(size_t i, const rgb8& col)
		{
			assert(has_colors());
			color_storage_ptr->set_color(i, col);
		}
		void colored_model::set_color(size_t i, const rgba8& col)
		{
			assert(has_colors());
			color_storage_ptr->set_color(i, col);
		}

		void colored_model::put_color(size_t i, rgb& col) const
		{
			assert(has_colors());
			color_storage_ptr->put_color(i, col);
		}
		void colored_model::put_color(size_t i, void* col_ptr) const
		{
			assert(has_colors());
			color_storage_ptr->put_color(i, col_ptr);
		}
		void colored_model::put_color(size_t i, rgba& col) const
		{
			assert(has_colors());
			color_storage_ptr->put_color(i, col);
		}
		void colored_model::put_color(size_t i, rgb8& col) const
		{
			assert(has_colors());
			color_storage_ptr->put_color(i, col);
		}
		void colored_model::put_color(size_t i, rgba8& col) const
		{
			assert(has_colors());
			color_storage_ptr->put_color(i, col);
		}

		size_t colored_model::get_nr_colors() const
		{
			return color_storage_ptr ? color_storage_ptr->get_nr_colors() : 0;
		}
		/// resize the color storage to given number of colors
		void colored_model::resize_colors(size_t nr_colors)
		{
			if (color_storage_ptr)
				color_storage_ptr->resize(nr_colors);
			else
				ensure_colors(CT_RGBA8, nr_colors);
		}
		/// return the size of one allocated color in byte
		size_t colored_model::get_color_size() const
		{
			return color_storage_ptr ? color_storage_ptr->get_color_size() : 0;
		}
		const void* colored_model::get_color_data_ptr() const
		{
			return color_storage_ptr->get_data_ptr();
		}
		const void* colored_model::get_color_data_vector_ptr() const
		{
			return color_storage_ptr->get_data_vector_ptr();
		}

		ColorType colored_model::get_color_storage_type() const
		{
			return color_storage_ptr ? color_storage_ptr->get_color_type() : CT_RGBA8;
		}

		void colored_model::ensure_colors(ColorType _color_type, size_t nr_colors)
		{
			if (color_storage_ptr) {
				if (color_storage_ptr->get_color_type() == _color_type)
					return;
				abst_color_storage* new_storage_ptr;
				switch (_color_type) {
				case CT_RGB:   new_storage_ptr = new color_storage<rgb>(*color_storage_ptr); break;
				case CT_RGBA:  new_storage_ptr = new color_storage<rgba>(*color_storage_ptr); break;
				case CT_RGB8:  new_storage_ptr = new color_storage<rgb8>(*color_storage_ptr); break;
				case CT_RGBA8: new_storage_ptr = new color_storage<rgba8>(*color_storage_ptr); break;
				}
				delete color_storage_ptr;
				color_storage_ptr = new_storage_ptr;
			}
			else {
				switch (_color_type) {
				case CT_RGB:   color_storage_ptr = new color_storage<rgb>(); break;
				case CT_RGBA:  color_storage_ptr = new color_storage<rgba>(); break;
				case CT_RGB8:  color_storage_ptr = new color_storage<rgb8>(); break;
				case CT_RGBA8: color_storage_ptr = new color_storage<rgba8>(); break;
				}
				if (nr_colors != -1)
					color_storage_ptr->resize(nr_colors);
			}
		}
		/// destruct color storage
		void colored_model::destruct_colors()
		{
			if (color_storage_ptr) {
				delete color_storage_ptr;
				color_storage_ptr = 0;
			}
		}

	}
}