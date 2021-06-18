#pragma once

#include <vector>
#include "color.h"

#include "lib_begin.h"

namespace cgv {
	namespace media {
		/// different color types used as defaults in the color storage
		enum ColorType {
			CT_RGB8,
			CT_RGBA8,
			CT_RGB,
			CT_RGBA
		};

		/// traits struct used to derive color type (need to be declared on namespace level for specialization)
		template <typename T> struct color_storage_traits {};
		template <> struct color_storage_traits<color<float, RGB> > { static const ColorType color_type = CT_RGB; };
		template <> struct color_storage_traits<color<float, RGB, OPACITY> > { static const ColorType color_type = CT_RGBA; };
		template <> struct color_storage_traits<color<unsigned char, RGB> > { static const ColorType color_type = CT_RGB8; };
		template <> struct color_storage_traits<color<unsigned char, RGB, OPACITY> > { static const ColorType color_type = CT_RGBA8; };

		/// declaration of types for color storage  
		struct color_storage_types
		{
		public:			
			/// define supported color types
			typedef color<float, RGB> rgb;
			typedef color<float, RGB, OPACITY> rgba;
			typedef color<unsigned char, RGB> rgb8;
			typedef color<unsigned char, RGB, OPACITY> rgba8;
		};

		/// interface for color storage of different internal types
		class abst_color_storage : public color_storage_types
		{
		protected:
			// type of color stored in storage
			ColorType color_type;
		public:
			/// construct from color type enumerate
			abst_color_storage(ColorType _color_type);
			/// virtual destructor ensures that std::vector defined in template class color_storage is destructed too
			virtual ~abst_color_storage();
			/// return color type of color storage 
			ColorType get_color_type() const;
			/// return size of a single color in byte
			size_t get_color_size() const;
			/// clone the color storage
			virtual abst_color_storage* clone() const = 0;
			/// return number colors stored in color storage
			virtual size_t get_nr_colors() const = 0;
			/// resize to the given number of colors
			virtual void resize(size_t nr_colors) = 0;
			/// return a void pointer to the color data
			virtual const void* get_data_ptr() const = 0;
			/// return a void pointer to the color data vector
			virtual const void* get_data_vector_ptr() const = 0;
			/// set i-th color to color of type stored in storage
			virtual void set_color(size_t i, const void* col_ptr) = 0;
			/// set i-th color from color of type rgb
			virtual void set_color(size_t i, const rgb& col) = 0;
			/// set i-th color from color of type rgba
			virtual void set_color(size_t i, const rgba& col) = 0;
			/// set i-th color from color of type rgb8
			virtual void set_color(size_t i, const rgb8& col) = 0;
			/// set i-th color from color of type rgba8
			virtual void set_color(size_t i, const rgba8& col) = 0;
			/// set color of type stored in storage to i-th color
			virtual void put_color(size_t i, void* col_ptr) const = 0;
			/// set color of type rgb to i-th color
			virtual void put_color(size_t i, rgb& col) const = 0;
			/// set color of type rgba to i-th color
			virtual void put_color(size_t i, rgba& col) const = 0;
			/// set color of type rgb8 to i-th color
			virtual void put_color(size_t i, rgb8& col) const = 0;
			/// set color of type rgba8 to i-th color
			virtual void put_color(size_t i, rgba8& col) const = 0;
		};

		/// template implementation of color storage model
		template <typename C>
		class color_storage : public abst_color_storage
		{
		public:
			std::vector<C> colors;
			/// construct empty color storage
			color_storage() : abst_color_storage(color_storage_traits<C>::color_type) { }
			/// construct from color storage of same type
			color_storage(const color_storage<C>& csm) : abst_color_storage(color_storage_traits<C>::color_type),
				colors(csm.colors) { }
			/// construct from color storage of different type
			template <typename C1>
			color_storage(const color_storage<C1>& csm) : abst_color_storage(color_storage_traits<C>::color_type) {
				for (const auto& col : csm.colors)
					colors.push_back(col);
			}
			/// construct from abstract color storage of unknown type
			color_storage(const abst_color_storage& acsm) : abst_color_storage(color_storage_traits<C>::color_type) {
				switch (acsm.get_color_type()) {
				case CT_RGB:   new (this) color_storage<C>(static_cast<const color_storage<rgb>&>(acsm)); break;
				case CT_RGBA:  new (this) color_storage<C>(static_cast<const color_storage<rgba>&>(acsm)); break;
				case CT_RGB8:  new (this) color_storage<C>(static_cast<const color_storage<rgb8>&>(acsm)); break;
				case CT_RGBA8: new (this) color_storage<C>(static_cast<const color_storage<rgba8>&>(acsm)); break;
				}
			}
			/// clone the color storage
			abst_color_storage* clone() const {
				return new color_storage(*this);
			}
			// implementation of vector access passes interface to std::vector class
			size_t get_nr_colors() const { return colors.size(); }
			void resize(size_t nr_colors) { colors.resize(nr_colors); }
			const void* get_data_ptr() const { return &colors.front(); }
			const void* get_data_vector_ptr() const { return &colors; }
			// implementation of color access uses type conversion operators implemented for color class
			void set_color(size_t i, const void* col_ptr) { colors[i] = *reinterpret_cast<const C*>(col_ptr); }
			void set_color(size_t i, const rgb& col) { colors[i] = col; }
			void set_color(size_t i, const rgba& col) { colors[i] = col; }
			void set_color(size_t i, const rgb8& col) { colors[i] = col; }
			void set_color(size_t i, const rgba8& col) { colors[i] = col; }
			void put_color(size_t i, void* col_ptr) const { *reinterpret_cast<C*>(col_ptr) = colors[i]; }
			void put_color(size_t i, rgb& col) const { col = colors[i]; }
			void put_color(size_t i, rgba& col) const { col = colors[i]; }
			void put_color(size_t i, rgb8& col) const { col = colors[i]; }
			void put_color(size_t i, rgba8& col) const { col = colors[i]; }
		};
	}
}

#include <cgv/config/lib_end.h>