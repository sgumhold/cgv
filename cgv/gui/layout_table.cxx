#include <cgv/gui/layout_table.h>
#include <memory.h>
#include <cgv/type/variant.h>

using namespace cgv::type;


/**
Outline of the algorithm:
Input parameters: 
  - width and height of the whole table
  - elements with minimum, optimum and default width and height and resize properties
  - c: Number of columns
  - r: Number of rows

Method "update":
  - Method "initialize_space_tables":
	 - Create an array with c elements for all columns (one structure per column)
	 - For every column do:
	    - Get the maximum width of elements
	    - Set hints for the column:
		   - If one of the elements is expandable then the column is expandable
		   - If one of the elements is shrinkable then the column is shrinkable
     - Do the same for all rows

- As long as the relayouting process is not finished:
   - Method "distribute_space":
       - Sum the space needed for all columns
	   - If the space is smaller than the available space:
	       - Expand elements that can be expanded or filled
	   - If the space is bigger than the available space:
	       - Shrink elements that can be shrunk
	   - Do the same for all rows
   - For all elements (children) in the table at position (i,j):
       - Get width at columns[i] and height at rows[j]
	   - Method "calculate_child_size":
	       - If the child can expand in X then set its width to columns[i]
		   - If the child cannot expand in X then set its width to its optimal width
		   - ... same for Y
	   - Try to apply the width and height for the child
	   - Get the width and height of the child
	   - If it could not set the width or height update the childs minimum width and height
	   - If the overall width or height exceeds the available table dimensions:
	       - Expand the table
		   - Queue another relayout iteration with the new extents

- For all elements (children) in the table:
   	   - Method "calculate_child_pos":
	       - If the child's X-alignment is middle then use the remaining cell space/2 as X offset
		   - If the child's X-alignment is right then use the remaining cell space as X offset
		   - ... same for Y
	   - Set size and position
	   - If the overall table size differs from the requested size:
	       - Method "repair_default_values":
		       - Set the new minimum and default width for the table
*/


namespace cgv {
	namespace gui {


		layout_table::layout_table() 
		{
			nr_cols = 1;
			nr_rows = 1;
			columns = 0;
			rows = 0;
			do_not_layout = false;
		}


		layout_table::layout_table(cgv::base::group_ptr container):
		layout(container)
		{
			nr_cols = 1;
			nr_rows = 1;
			columns = 0;
			rows = 0;
			do_not_layout = false;
		}


		layout_table::~layout_table()
		{
			delete_space_tables();
		}


		void layout_table::delete_space_tables()
		{
			if (columns) 
				delete[] columns;

			if (rows)
				delete[] rows;

			columns = 0;
			rows = 0;
		}


		// calculate minimum and optimum size
		void layout_table::initialize_space_tables()
		{
			int opt_width, opt_height, min_width, min_height, hints;

			if (!columns) {
 				// initialize the row and column information
				columns = new layout_table_cell[nr_cols];
				rows = new layout_table_cell[nr_rows];
			}

			// elements can shrink by default
			for (int c=0; c<nr_cols; c++)
				columns[c].reset();
			for (int r=0; r<nr_rows; r++)
				rows[r].reset();

			for (int r=0; r<nr_rows; r++) 
				for (int c=0; c<nr_cols; c++) {

					base_ptr cur_child = get_child(c+r*nr_cols);

					get_child_default_size(cur_child, opt_width, opt_height);
					get_child_minimum_size(cur_child, min_width, min_height);
					hints = get_child_layout_hints(cur_child);

					update_spaces_table(columns[c], opt_width, min_width, hints, LH_HHINTS);
					update_spaces_table(rows[r], opt_height, min_height, hints, LH_VHINTS);
				}

		}



		void layout_table::update()
		{
			int opt_cwidth, opt_cheight, min_cwidth, min_cheight, chints;
			int opt_width, opt_height;
			int min_width, min_height;
			int border_width, border_height;
			int new_cwidth, new_cheight;
			int realized_cwidth, realized_cheight;
			bool is_wrong_size, need_relayout;
			int pos_x, pos_y, pos_x_offset, pos_y_offset;


			if (do_not_layout || !container || !container->get_nr_children() || !nr_cols || !nr_rows)
				return;

			true_w = w;
			true_h = h;

			is_wrong_size = false;

			// calculate the space needed for the border and inner spacings
			border_width = spacings.horizontal.element*(nr_cols-1) + spacings.horizontal.border*2;
			border_height = spacings.vertical.element*(nr_rows-1) + spacings.vertical.border*2;

			// initialize the column and row tables and calculate
			// minimum and optimum size
			initialize_space_tables();

			int num_relayouts = 100;

			// rearrange the children as long as relayouting is neccessary
			do {
				num_relayouts--;
				need_relayout = false;

				// get optimum and minimum sum size
				get_sizes(columns, nr_cols, &opt_width, &min_width);
				get_sizes(rows, nr_rows, &opt_height, &min_height);

				// add border spaces
				opt_width+=border_width;
				min_width+=border_width;
				opt_height+=border_height;
				min_height+=border_height;

				// is it possible to fit the table into the given size?
				if (min_width > true_w) {
					is_wrong_size = true;
					true_w = min_width;
				}
				if (min_height > true_h) {
					is_wrong_size = true;
					true_h = min_height;
				}


				// split the space into cells
				distribute_space(columns, nr_cols, true_w - border_width);
				distribute_space(rows, nr_rows, true_h - border_height);
				
				for (int r=0; r<nr_rows; r++) {

					for (int c=0; c<nr_cols; c++) {
						base_ptr cur_child = get_child(c+r*nr_cols);

						get_child_default_size(cur_child, opt_cwidth, opt_cheight);
						get_child_minimum_size(cur_child, min_cwidth, min_cheight);
						chints = get_child_layout_hints(cur_child);

						// calculate the child size according to the
						// specific layout hints for a child
						calculate_child_size(columns[c], opt_cwidth, min_cwidth, chints, LH_HHINTS, new_cwidth);
						calculate_child_size(rows[r], opt_cheight, min_cheight, chints, LH_VHINTS, new_cheight);

						// set size and position for this child
						set_child_size(cur_child, new_cwidth, new_cheight);

						// test whether the size request could be fulfilled
						get_child_size(cur_child, realized_cwidth, realized_cheight);

						// the width could not be set for this element as it returns a greater
						// value than we tried to set. Update its minimum width, expand our
						// container width (which will now not fit anymore) and relayout
						if (realized_cwidth > columns[c].real_size) {
							if (columns[c].min_size < realized_cwidth)
								columns[c].min_size = realized_cwidth;
							if (columns[c].opt_size < columns[c].min_size)
								columns[c].opt_size = columns[c].min_size;

							cur_child->set<int>("mw", realized_cwidth);
							//true_w += realized_cwidth - columns[c].real_size;
							is_wrong_size = true;
							need_relayout = true;
							break;
						}

						if (realized_cheight > rows[r].real_size) {
							if (rows[r].min_size < realized_cheight)
								rows[r].min_size = realized_cheight;
							if (rows[r].opt_size < rows[r].min_size)
								rows[r].opt_size = rows[r].min_size;

							cur_child->set<int>("mh", realized_cheight);
							is_wrong_size = true;
							need_relayout = true;
							break;
						}
					}
				}
			} while(need_relayout && num_relayouts>0);

			if (num_relayouts == 0) {
				std::cerr<<"layout_table stopped layouting the GUI because the maximum number of layouting iterations"<<std::endl;
				std::cerr<<"was reached. This is probably a bug in the layouter."<<std::endl;
			}

			// elements are resized now. set the position
			pos_y = spacings.vertical.border;

			for (int r=0; r<nr_rows; r++) {
				pos_x = spacings.horizontal.border;
				for (int c=0; c<nr_cols; c++) {
					base_ptr cur_child = get_child(c+r*nr_cols);

					get_child_size(cur_child, opt_cwidth, opt_cheight);
					chints = get_child_layout_hints(cur_child);

					calculate_child_pos(columns[c], opt_cwidth, chints, LH_HHINTS, pos_x_offset);
					calculate_child_pos(rows[r], opt_cheight, chints, LH_VHINTS, pos_y_offset);

					set_child_position(cur_child, pos_x + pos_x_offset, pos_y + pos_y_offset);

					pos_x+=columns[c].real_size + spacings.horizontal.element;
				}
				pos_y+=rows[r].real_size + spacings.vertical.element;
			}


			if (is_wrong_size) {
				do_not_layout = true;

				// if a resize was needed then the default values
				// (minimum size and default size) do not seem
				// to be correct. Repair them
				repair_default_values(min_width, min_height, 
									  opt_width, opt_height);

				// set the actual size
				container->set<int>("w", true_w);
				container->set<int>("h", true_h);

				do_not_layout = false;
				
			}
		}


		void layout_table::repair_default_values(int min_width, int min_height, int opt_width, int opt_height)
		{
			int cur_dwidth, cur_dheight;

			// set the new minimum size
			container->set<int>("mw", min_width);
			container->set<int>("mh", min_height);

			// check whether the default size can be overwritten
			get_child_default_size(container, cur_dwidth, cur_dheight);
			int hints = get_child_layout_hints(container);

			// if we can expand or fill then set the new default
			// size to be the optimal size. Otherwise check whether
			// the old default size is too small and repair it
			if ((hints & LH_HEXPAND) || (hints & LH_HFILL))
				container->set<int>("dw", opt_width);
			else if (min_width > cur_dwidth)
				container->set<int>("dw", min_width);

			if ((hints & LH_VEXPAND) || (hints & LH_HEXPAND)) 
				container->set<int>("dh", opt_height);
			else if (min_height > cur_dheight)
				container->set<int>("dh", min_height);

		}



		void layout_table::calculate_child_size(layout_table_cell &element, int opt_size, int min_size, int hints, int hints_shift, int &new_size)
		{
			new_size = opt_size;

			if (element.real_size > opt_size && ((hints & (LH_HFILL<<hints_shift)) || (hints & (LH_HEXPAND<<hints_shift))))
				new_size = element.real_size;

			// decrease the size if we can shrink and the size is smaller
			if ((hints & (LH_HSHRINK<<hints_shift)) && (element.real_size<opt_size)) 
			{
				if (element.real_size >= min_size)
					new_size = element.real_size;
				else
					new_size = min_size;

			}
		}


		void layout_table::calculate_child_pos(layout_table_cell &element, int child_size, int hints, int hints_shift, int &new_pos)
		{
			new_pos = 0;

			// the next part is repositioning which does nothing if there is no space
			if (element.real_size <= child_size)
				return;

			// reposition the element
			// right/bottom aligned?
			if ((hints & (LH_RIGHT<<hints_shift)))
				new_pos = element.real_size - child_size;
			else
			// centered/middled?
			if ((hints & (LH_CENTER<<hints_shift))) {
				new_pos = (element.real_size - child_size)/2;
			}

		}




		void layout_table::update_spaces_table(layout_table_cell &element, int opt_size, int min_size, int hints, int hints_shift)
		{
			// set the optimum size for this cell
			if (opt_size > element.opt_size)
						element.opt_size = opt_size;

			// set the minimum size for this cell or if an
			// element cannot be shrunk set the optimum size as minimum
			if (!(hints & (LH_HSHRINK << hints_shift)))
				min_size = opt_size;
			if (min_size > element.min_size)
				element.min_size = min_size;
			if (element.opt_size < element.min_size)
				element.opt_size = element.min_size;

			// has the column element an element that shall be 
			// filled, expanded or shrinked?
			if (hints & (LH_HFILL << hints_shift))
				element.can_fill = true;

			if (hints & (LH_HSHRINK << hints_shift))
				element.can_shrink = true;

			if (hints & (LH_HEXPAND << hints_shift))
				element.can_expand = true;
		}



		void layout_table::get_sizes(layout_table_cell *elements, int nr_elements, int *opt_size, int *min_size)
		{
			if (opt_size) {
				*opt_size = 0;
				for (int i=0; i<nr_elements; i++)
					(*opt_size)+=elements[i].opt_size;
			}

			if (min_size) {
				*min_size=0;
				for (int i=0; i<nr_elements; i++)
					(*min_size)+=elements[i].min_size;
			}
		}




		bool layout_table::distribute_space(layout_table_cell *elements, int nr_elements, int new_size)
		{
			if (new_size<0) {
				new_size = 0;
			}

			int opt_size;
			// calculate the optimum size
			get_sizes(elements, nr_elements, &opt_size, NULL);
			for (int i=0; i<nr_elements; i++)
				elements[i].real_size = elements[i].opt_size;

			// do we have enough space? if this is the case then first
			// look for elements that shall fill the space and distribute
			// the remaining space in the ratio of their old size
			if (new_size >= opt_size) {
				int remain_space = new_size - opt_size;

				// get the sum of all fillable spaces
				int fill_space = 0;
				int expand_space = 0;
				for (int i=0; i<nr_elements; i++) {
					if (elements[i].can_fill) 
						fill_space+=elements[i].opt_size;
					if (elements[i].can_expand)
						expand_space+=elements[i].opt_size;
				}

				// do we have something that shall be filled?
				// if yes then distribute the space and quit
				if (fill_space>0) {
					for (int i=0; i<nr_elements; i++)
						if (elements[i].can_fill)
							elements[i].real_size+=elements[i].opt_size*remain_space/fill_space;

					return true;
				}

				// nothing to fill but elements to be expanded?
				// also distribute the space and quit
				if (expand_space > 0) {
					for (int i=0; i<nr_elements; i++)
						if (elements[i].can_expand)
							elements[i].real_size+=elements[i].opt_size*remain_space/expand_space;

					return true;
				}

				// still here. expand again but only all elements
				for (int i=0; i<nr_elements; i++)
					elements[i].real_size+=elements[i].opt_size*remain_space/opt_size;
			}



			// the size is not sufficient, so try to shrink elements
			if (new_size < opt_size) {
				// calculate the minimum size
				int min_size;
				get_sizes(elements, nr_elements, NULL, &min_size);

				// is it possible to shrink the elements enough?
				// if not we can stop trying
				if (min_size>new_size) {
					return false;
				}

				// how much can be shrunk
				int shrink_size = opt_size - min_size;
				// how much has to be shrunk
				int overfl_size = opt_size - new_size;

				int shrunk = 0;
				// shrink all elements according to their ratio
				for (int i=0; i<nr_elements; i++)
					if (elements[i].can_shrink)  {
						elements[i].real_size-= (elements[i].opt_size - elements[i].min_size)*overfl_size/shrink_size;
						shrunk+=(elements[i].opt_size - elements[i].min_size)*overfl_size/shrink_size;
					}

				// there might be one pixel left due to a division
				// by an odd number. get it from the first element
				// that can be shrunk.
				if (shrunk != overfl_size)
					for (int i=0; i<nr_elements; i++)
						if (elements[i].can_shrink && elements[i].real_size > elements[i].min_size) {
							elements[i].real_size--;
							break;
						}
			}

			return true;
		}



		std::string layout_table::get_property_declarations()
		{
			return layout::get_property_declarations()+";cols:int32;rows:int32";
		}


		bool layout_table::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
		{
			if (layout::set_void(property, value_type, value_ptr))
				return true;

			if (property == "cols")
				nr_cols = variant<int32_type>::get(value_type, value_ptr);
			else if (property == "rows") 
				nr_rows = variant<int32_type>::get(value_type, value_ptr);
			else
				return false;

			return true;
		}


		bool layout_table::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
		{
			if (layout::get_void(property, value_type, value_ptr))
				return true;

			if (property == "cols") 
				set_variant(nr_cols, value_type, value_ptr);
			else if (property == "rows")
				set_variant(nr_rows, value_type, value_ptr);
			else
				return false;

			return true;
		}

		// get width of a column
		int layout_table::get_column_width(int col) 
		{
			if (col<nr_cols && columns)
				return columns[col].min_size;

			return -1;
		}
		
		// get height of a row
		int layout_table::get_row_height(int row)
		{
			if (row<nr_rows && rows)
				return rows[row].min_size;

			return -1;
		}

		// get size of a cell
		void layout_table::get_cell_size(int row, int col, int &width, int &height)
		{
			width = get_column_width(col);
			height = get_row_height(row);
		}



	}
}
