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


/// possible special semantics of attributes found in .csv column(s)
enum class CSV_AttribSemantics
{
	ARBITRARY=0, POS, TRAJ_ID
};
#if !defined(CSV_HANDLER_NO_SHORTHAND_ENUM_CSV) || CSV_HANDLER_NO_SHORTHAND_ENUM_CSV==0
	/// convenience shorthand for \ref CSV_AttribSemantics
	typedef CSV_AttribSemantics CSV;
#endif


/// table format descriptor to identify a supported .csv file.
class csv_descriptor
{

public:

	/// collection of properties of a .csv descriptor
	struct csv_properties
	{
		struct
		{
			/// indicates if readable .csv files must have a header row
			unsigned char header : 1,

			/// indicates if readable .csv files can store more than one trajectory
			multi_traj : 1;
		};

		/// indicates the largest .csv column number referenced by any attribute in the descriptor
		unsigned max_col_id;

		/// indicates the index of the position attribute in declared attribute list
		unsigned pos_id;

		/// indicates the index of the trajectory id attribute in the declared attribute list, if any
		unsigned traj_id;
	};

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
		attribute() : semantics(CSV::ARBITRARY) {}

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

	/// construct with the given .csv separator and attribute specification
	csv_descriptor(const std::string &separators, const std::vector<attribute> &attributes);

	/// construct with the given .csv separator (move semantics) and attribute specification
	csv_descriptor(std::string &&separators, const std::vector<attribute> &attributes);

	/// construct with the given .csv separator and attribute specification (move semantics)
	csv_descriptor(const std::string &separators, std::vector<attribute> &&attributes);

	/// construct with the given .csv separator and attribute specification (move semantics on both arguments)
	csv_descriptor(std::string &&separators, std::vector<attribute> &&attributes);

	/// the destructor
	~csv_descriptor();

	/// copy assignment
	csv_descriptor& operator= (const csv_descriptor &other);

	/// move assignment
	csv_descriptor& operator= (csv_descriptor &&other);

	/// reference the list of column separators expected in the .csv file
	const std::string& separators (void) const;

	/// reference the file attribute definitions encapsulated by this descriptor
	const std::vector<attribute>& attributes (void) const;

	/// reference the attribute properties struct
	const csv_properties& properties (void) const;

	/// inferes the properties that would result from the given list of attribute definitions
	static csv_properties infer_properties (const std::vector<attribute> &attributes);
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
};
