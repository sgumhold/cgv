#pragma once

#include <cgv/gui/layout.h>

#include <vector>
#include "lib_begin.h"


namespace cgv {
	namespace gui {

		// struct containing information on one cell
		struct layout_table_cell {
			bool can_shrink;	// can this cell be shrinked?
			bool can_expand;	// can this cell be expanded?
			bool can_fill;		// can this cell fill all avalable space?
			int min_size;		// minimum size for this cell
			int opt_size;		// optimal size for this cell
			int real_size;

			void reset() {
				can_shrink = false;
				can_expand = false;
				can_fill = false;
				min_size = 0;
				opt_size = 0;
				real_size = 0;
			};
		};


		// layout to arrange children of a group that have a size and position
		// (ie. are derived from resizable) in a table
		class CGV_API layout_table: public layout
		{
		public:
			layout_table();
			layout_table(cgv::base::group_ptr container);

			~layout_table();

			// the core update function to align the elements in a table
			void update();

			// standard get and set methods
			std::string get_property_declarations();
			bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
			bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);

			// get width of a column
			int get_column_width(int col);
			// get height of a row
			int get_row_height(int row);

			// get size of a cell
			void get_cell_size(int row, int col, int &width, int &height);

		private:
			int nr_cols, nr_rows;
			layout_table_cell *columns;
			layout_table_cell *rows;
			bool do_not_layout;

			// calculate minimum and optimum size
			void initialize_space_tables();

			void delete_space_tables();
			void update_spaces_table(layout_table_cell &element,  int opt_size, int min_size, int hints, int hints_shift);
			bool distribute_space(layout_table_cell *elements, int nr_elements, int new_size);
			void calculate_child_size(layout_table_cell &element, int opt_size, int min_size, int hints, int hints_shift, int &new_size); 
			void calculate_child_pos(layout_table_cell &element, int child_size, int hints, int hints_shift, int &new_pos);
			
			void get_sizes(layout_table_cell *elements, int nr_elements, int *opt_size, int *min_size);

			void repair_default_values(int min_width, int min_height, int opt_width, int opt_height);
		};


/// ref counted pointer to table layout
typedef cgv::data::ref_ptr<layout_table> layout_table_ptr;


	}
}


#include <cgv/config/lib_end.h>