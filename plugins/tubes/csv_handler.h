#pragma once

// C++ STL
#include <iostream>
#include <vector>
#include <initializer_list>

// CGV framework core
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/media/color.h>

// local includes
#include "traj_loader.h"


/// possible special semantics of a .csv column
enum class CSV_ColumnSemantics
{
	ARBITRARY=0, POS_X, POS_Y, POS_Z, TRAJ_ID
};
#if !defined(CSV_HANDLER_NO_SHORTHAND_ENUM_CSV) || CSV_HANDLER_NO_SHORTHAND_ENUM_CSV==0
	/// convenience shorthand for \ref CSV_ColumnSemantics
	typedef CSV_ColumnSemantics CSV;
#endif


/// table format descriptor to identify a supported .csv file
struct csv_table_descriptor
{
	/// descriptor of one data column
	/// ToDo: add way to describe the data format of a column, right now we always assume a scalar - it would be
	///       nice to be able to specify string encodings for other data types (like actual strings or composite
	///       types like vectors or matrices)
	struct column
	{
		/// name of the column
		std::string name;

		/// special meaning of this column, if any
		CSV_ColumnSemantics semantics;

		/// whether \ref #name should be treated as case-sensitive or not
		/// ToDo: make it settable
		bool case_sensitive = false;

		// default constructor
		column() : semantics(CSV_ColumnSemantics::ARBITRARY) {}

		// constructs unnamed column with given semantics
		column(CSV_ColumnSemantics semantics=CSV_ColumnSemantics::ARBITRARY)
			: semantics(semantics)
		{}

		// constructs from string containing the column name and a given semantics
		column(const std::string &name, CSV_ColumnSemantics semantics=CSV_ColumnSemantics::ARBITRARY)
			: name(name), semantics(semantics)
		{
			if (!case_sensitive)
				this->name = cgv::utils::to_lower(name);
		}

		// constructs from string containing the column name (move semantics)
		column(std::string &&name, CSV_ColumnSemantics semantics=CSV_ColumnSemantics::ARBITRARY)
			: name(std::move(name)), semantics(semantics)
		{
			if (!case_sensitive)
				this->name = cgv::utils::to_lower(this->name);
		}

		// constructs from c-style string containing the column name
		column(const char *name, CSV_ColumnSemantics semantics=CSV_ColumnSemantics::ARBITRARY)
			: name(name), semantics(semantics)
		{
			if (!case_sensitive)
				this->name = cgv::utils::to_lower(this->name);
		}
	};

	/// whether to insist on a header row - determines whether the column names will be checked
	bool header;

	/// list of columns expected in the .csv - if \ref #header is false, the ordering must be exactly as in the
	/// .csv file
	std::vector<column> columns;

	/// default constructor
	inline csv_table_descriptor() : header(false) {}

	/// copy constructor
	inline csv_table_descriptor(const csv_table_descriptor &other)
		: header(other.header), columns(other.columns)
	{}

	/// move constructor
	inline csv_table_descriptor(csv_table_descriptor &&other)
		: header(other.header), columns(std::move(other.columns))
	{}

	/// construct with the given properties
	inline csv_table_descriptor(const std::vector<column> &columns, bool header=false)
		: header(header), columns(columns)
	{}

	/// construct with the given properties (moving in the column descriptors)
	inline csv_table_descriptor(std::vector<column> &&columns, bool header=false)
		: header(header), columns(std::move(columns))
	{}

	/// construct with the given properties, specifying the columns via initializer list
	inline csv_table_descriptor(const std::initializer_list<column> &columns, bool header=false)
		: header(header), columns(columns)
	{}

	/// copy assignment
	inline csv_table_descriptor& operator= (const csv_table_descriptor &other)
	{
		header = other.header;
		columns = other.columns;
		return *this;
	}

	/// move assignment
	inline csv_table_descriptor& operator= (csv_table_descriptor &&other)
	{
		header = other.header;
		columns = std::move(other.columns);
		other.columns.clear(); // make sure the other is always empty after having its contents moved out
		                       // (because we don't want the standard swap semantics for reasons)
		return *this;
	}
};


/// possible special semantics of a .csv column
enum class CSV_AttribSemantics
{
	ARBITRARY=0, POS, TRAJ_ID
};
#if !defined(CSV_HANDLER_NO_SHORTHAND_ENUM_CSV) || CSV_HANDLER_NO_SHORTHAND_ENUM_CSV==0
	/// convenience shorthand for \ref CSV_AttribSemantics
	typedef CSV_AttribSemantics CSVA;
#endif


/// table format descriptor to identify a supported .csv file.
class csv_descriptor
{

public:

	/// descriptor of one attribute (potentially composed from many columns) encoded in the .csv file
	/// ToDo: Add way to describe the data format of a column, right now we always assume one scalar
	///       per column. It would be nice to be able to specify string encodings of other data types
	///       (like actual strings or composite types like vectors or matrices)
	struct attribute
	{
		/// struct describing a specific .csv data column
		struct column
		{
			/// name of the attribute as specified in the .csv header row
			std::string name;

			/// whether \ref #name should be treated as case-sensitive or not
			bool case_sensitive;

			/// the number of this column within the .csv table structure (used only when header-less
			/// .csv support is requested)
			int number;

			/// default constructor
			inline column() : case_sensitive(false), number(-1)
			{}

			/// copy constructor
			inline column(const column &other)
				: name(other.name), case_sensitive(other.case_sensitive), number(other.number)
			{}

			/// move constructor
			inline column(column &&other)
				: name(std::move(other.name)), case_sensitive(other.case_sensitive), number(other.number)
			{}

			/// constructs with the given case-insensitive name
			inline column(const std::string &name)
				: name(name), case_sensitive(false), number(-1)
			{}

			/// constructs with the given case-insensitive name (move semantics)
			inline column(std::string &&name)
				: name(std::move(name)), case_sensitive(false), number(-1)
			{}

			/// constructs with the given name and case-sensitivity
			inline column(const std::string &name, bool case_sensitive)
				: name(name), case_sensitive(case_sensitive), number(-1)
			{}

			/// constructs with the given name (move semantics) and case-sensitivity
			inline column(std::string &&name, bool case_sensitive)
				: name(std::move(name)), case_sensitive(case_sensitive), number(-1)
			{}

			/// constructs with the given name and column number
			inline column(const std::string &name, int number)
				: name(name), case_sensitive(false), number(number)
			{}

			/// constructs with the given name (move semantics) and column number
			inline column(std::string &&name, int number)
				: name(std::move(name)), case_sensitive(false), number(number)
			{}

			/// constructs with the given name, case-sensitivity and column number
			inline column(const std::string &name, bool case_sensitive, int number)
				: name(name), case_sensitive(case_sensitive), number(number)
			{}

			/// constructs with the given name (move semantics), case-sensitivity and column number
			inline column(std::string &&name, bool case_sensitive, int number)
				: name(std::move(name)), case_sensitive(case_sensitive), number(number)
			{}

			/// copy assignment
			inline column& operator= (const column &other)
			{
				name = other.name;
				case_sensitive = other.case_sensitive;
				number = other.number;
				return *this;
			}

			/// move assignment
			inline column& operator= (column &&other)
			{
				name = std::move(other.name);
				case_sensitive = case_sensitive;
				number = other.number;
				return *this;
			}
		};

		/// name of the attribute
		std::string name;

		/// special semantics of this attribute (if applicable)
		CSV_AttribSemantics semantics;

		/// .csv columns to read samples of this attribute from
		std::vector<column> columns;

		/// default constructor
		attribute() : semantics(CSVA::ARBITRARY) {}

		/// copy constructor
		inline attribute(const attribute &other)
			: name(other.name), semantics(other.semantics), columns(other.columns)
		{}

		/// move constructor
		inline attribute(attribute &&other)
			: name(std::move(other.name)), semantics(other.semantics), columns(std::move(other.columns))
		{}

		/// constructs the attribute with given name (copied in), single assigned .csv column (copied in) and semantics
		inline attribute(const std::string &name, const column &csv_column,
		                 CSV_AttribSemantics semantics=CSV_AttribSemantics::ARBITRARY)
			: name(name), semantics(semantics)
		{
			columns.emplace_back(csv_column);
		}

		/// constructs the attribute with given name (copied in), single assigned .csv column (moved in) and semantics
		inline attribute(const std::string &name, column &&csv_column,
		                 CSV_AttribSemantics semantics=CSV_AttribSemantics::ARBITRARY)
			: name(name), semantics(semantics)
		{
			columns.emplace_back(std::move(csv_column));
		}

		/// constructs the attribute with given name (moved in), single assigned .csv column (copied in) and semantics
		inline attribute(std::string &&name, const column &csv_column,
		                 CSV_AttribSemantics semantics=CSV_AttribSemantics::ARBITRARY)
			: name(std::move(name)), semantics(semantics)
		{
			columns.emplace_back(csv_column);
		}

		/// constructs the attribute with given name (moved in), single assigned .csv column (moved in) and semantics
		inline attribute(std::string &&name, column &&csv_column,
		                 CSV_AttribSemantics semantics=CSV_AttribSemantics::ARBITRARY)
			: name(std::move(name)), semantics(semantics)
		{
			columns.emplace_back(std::move(csv_column));
		}

		/// constructs the attribute with given name (copied in), .csv column assignment (copied in) and semantics
		inline attribute(const std::string &name, const std::vector<column> &column_assignment,
		                 CSV_AttribSemantics semantics=CSV_AttribSemantics::ARBITRARY)
			: name(name), columns(column_assignment), semantics(semantics)
		{}

		/// constructs the attribute with given name (copied in), .csv column assignment (moved in) and semantics
		inline attribute(const std::string &name, std::vector<column> &&column_assignment,
		                 CSV_AttribSemantics semantics=CSV_AttribSemantics::ARBITRARY)
			: name(name), columns(std::move(column_assignment)), semantics(semantics)
		{}

		/// constructs the attribute with given name (moved in), .csv column assignment (copied in) and semantics
		inline attribute(std::string &&name, const std::vector<column> &column_assignment,
		                 CSV_AttribSemantics semantics=CSV_AttribSemantics::ARBITRARY)
			: name(std::move(name)), columns(column_assignment), semantics(semantics)
		{}

		/// constructs the attribute with given name (moved in), .csv column assignment (moved in) and semantics
		inline attribute(std::string &&name, std::vector<column> &&column_assignment,
		                 CSV_AttribSemantics semantics=CSV_AttribSemantics::ARBITRARY)
			: name(std::move(name)), columns(std::move(column_assignment)), semantics(semantics)
		{}

		/// copy assignment
		inline attribute& operator= (const attribute &other)
		{
			name = other.name;
			semantics = other.semantics;
			columns = other.columns;
			return *this;
		}

		/// move assignment
		inline attribute& operator= (attribute &&other)
		{
			name = std::move(other.name);
			semantics = other.semantics;
			columns = std::move(other.columns);
			return *this;
		}
	};


private:

	/// implementation forward
	struct Impl;

	/// implementation handle
	Impl *pimpl;


public:

	/// default constructor
	csv_descriptor();

	/// copy constructor
	csv_descriptor(const csv_descriptor &other);

	/// move constructor
	csv_descriptor(csv_descriptor &&other);

	/// construct with the given .csv attribute specification
	csv_descriptor(const std::vector<attribute> &attributes);

	/// construct with the given .csv attribute specification (move semantics)
	csv_descriptor(std::vector<attribute> &&attributes);

	/// the destructor
	~csv_descriptor();

	/// copy assignment
	csv_descriptor& operator= (const csv_descriptor &other);

	/// move assignment
	csv_descriptor& operator= (csv_descriptor &&other);

	/// reference the file attribute definitions encapsulated by this descriptor
	const std::vector<attribute>& attributes (void) const;

	/// check if a given list of file attribute definitions will require .csv files to have header row
	static bool will_need_header (const std::vector<attribute> &attributes);
};


/// provides read and write capabilites for trajectories in .csv format
template <class flt_type>
class csv_handler : public traj_format_handler<flt_type>
{
	/// make sure the std::containers can access the default constructor
	//friend class std::vector;


public:

	/// real number type
	typedef traj_format_handler::real real;

	/// 2D vector type
	typedef traj_format_handler::Vec2 Vec2;

	/// 3D vector type
	typedef traj_format_handler::Vec3 Vec3;

	/// 4D vector type
	typedef traj_format_handler::Vec4 Vec4;

	/// rgb color type
	typedef traj_format_handler::rgb rgb;


private:

	/// implementation forward
	struct Impl;

	/// implementation handle
	Impl *pimpl;


protected:

	/// explicitely reset implementation-specific state
	virtual void cleanup (void);

	/// default constructor
	csv_handler();


public:

	/// constructs the handler for the given .csv table format description
	csv_handler(const csv_table_descriptor &table_desc);

	/// constructs the  handler for the given .csv table format description (move semantics)
	csv_handler(csv_table_descriptor &&table_desc);

	/// Constructs the handler for the given .csv description. Note that the columns mentioned by the
	/// descriptors are required for a file to be readable by this handler, but that does not exclude
	/// additional columns from being read in also - they will simply be read into additional generic
	/// attributes. Also, specifying an expected column number for every attribute in the descriptor
	/// will enable loading even in the absence of a header row.
	csv_handler(const csv_descriptor &csv_desc);

	/// Constructs the handler for the given .csv description (move semantics). Note that the columns
	/// mentioned by the descriptors are required for a file to be readable by this handler, but that
	/// does not exclude additional columns from being read in also - they will simply be read into
	/// additional generic attributes. Also, specifying an expected column number for every attribute
	/// in the descriptor will enable loading even in the absence of a header row.
	csv_handler(csv_descriptor &&csv_desc);

	/// the destructor
	virtual ~csv_handler();

	/// test if the given data stream appears to be a .csv file we can interpret
	virtual bool can_handle (std::istream &contents) const;

	/// parse the given stream containing the .csv file contents and report whether any data was loaded
	virtual bool read (std::istream &contents, unsigned idx_offset=0);

	/// check if the handler currently stores valid loaded data
	virtual bool has_data (void) const;

	/// report the average spatial distance between samples in the dataset
	virtual real avg_segment_length(void) const;

	/// report a visual attribute mapping that makes sense for the .csv contents
	virtual const visual_attribute_mapping& suggest_mapping (void) const;

	/// Check if a given csv description is valid. Note that this only performs superficial checks that are computationally
	/// cheap - does not catch inconsistencies possible in a \ref csv_descriptor !
	static bool is_csv_descriptor_valid (const csv_descriptor &csv_desc);

	/// check if a given csv description can result in multiple trajectories from a single file
	static bool is_csv_descriptor_multi_traj (const csv_descriptor &csv_desc);
};
