#pragma once 

#include <string>
#include <vector>
#include <functional>
#include <cgv/utils/advanced_scan.h>

#include <cgv/utils/lib_begin.h>

namespace cgv {
	namespace utils {

/// different flags used by csv_reader classes 
enum CSV_Flags
{
	CSV_HEADING = 1, // whether file starts with heading line
	CSV_COMMA   = 2, // whether comma is a separator
	CSV_TAB     = 4, // whether tab is a separator
	CSV_SPACE   = 8, // whether space is a separating white space (always merged)
	CSV_DEFAULT  = CSV_HEADING | CSV_COMMA| CSV_TAB
};

/// base class of csv parser responsible for splitting file into lines and tokens
class CGV_API csv_reader_base
{
protected:
	mutable std::string last_error;
	mutable bool failed = false;
	mutable size_t li = 0;
	mutable size_t nr_cols = 0;

	CSV_Flags flags;
	std::string content;
	std::vector<std::string> col_names;
	std::vector<cgv::utils::line> lines;
	const char* get_ws() const;
	const char* get_sep() const;
	bool has_heading() const;
	bool find_column_index(const std::string& col_name, size_t& col_index) const;
	csv_reader_base(const std::string& file_name, CSV_Flags _flags = CSV_DEFAULT);
	bool parse_next_line(std::vector<cgv::utils::token>& tokens) const;
public:
	bool fail() const;
};

/// csv reader for parsing matrices of a homogenous number type
class csv_matrix_reader : public csv_reader_base
{
public:
	/// construct from csv file with flags defaulting to use of space as separator
	csv_matrix_reader(const std::string& file_name, CSV_Flags _flags = CSV_SPACE) : csv_reader_base(file_name, _flags) {}
	/// parse matrix into nested vector struct and initialize entries to given init value (only rectangular matrices supported)
	template <typename T>
	bool parse_matrix(std::vector<std::vector<T>>& matrix, T init = T(0)) {
		std::vector<cgv::utils::token> tokens;
		while (parse_next_line(tokens)) {
			std::vector<T> vec(tokens.size(), init);
			for (size_t i = 0; i < tokens.size(); ++i)
				cgv::utils::from_string(vec[i], cgv::utils::to_string(tokens[i]));
			matrix.emplace_back(vec);
		}
		return true;
	}
};


//! csv reader for parsing csv files to a specific type T that can store one object with different entries per csv column.
/*! For usage simply construct reader from file name and flags, and first check for failure
    
	struct object_type {
		int a;
		std::string s;
		double custom_time;
	};
    csv_reader<object_type> cr(file_name, my_flags);
	if (cr.fail())
		return false;

	next add entries for each member to be extracted from a csv column

	if (!(
	cr.add_entry(&object_type::a, "a") &&
	cr.add_entry(&object_type::s, "s") &&
	cr.add_entry("time", [](const std::string& token, object_type& obj)->bool {
	   // parse token into obj.custom_time
	   // return whether parsing was successful
	   return true;
	   }) ) )
		return false;

	finally parse all lines in the csv file

	std::vector<object_type> objects;
	if (!cr.parse(objects))
		return false;
	*/
template <typename T>
class csv_reader : public csv_reader_base
{
protected:
	typedef std::pair<size_t, std::function<bool (const std::string&, T&)>> setter;
	std::vector<setter> setters;
public:
	/// construct from csv file with default flags (comma or tab as separator plus heading line) and extract column names from heading line
	csv_reader(const std::string& file_name, CSV_Flags _flags = CSV_DEFAULT) : csv_reader_base(file_name, _flags) {}
	/// add new entry based on column index (0, 1, ...) with custom setter function
	bool add_entry(size_t col_index, std::function<bool(const std::string&, T&)> s) {
		setters.push_back({ col_index, s });
		return true;
	}
	/// add new entry based on column index (0, 1, ...) with default setter for member type
	template <typename M>
	bool add_entry(M T::*ptr, size_t col_index) {
		add_entry(col_index, [ptr](const std::string& tok, T& obj) -> bool {
			return cgv::utils::from_string(obj.*ptr, tok);
		});
		return true;
	}
	/// add new entry based on heading of column (assumes construction with has_heading=true) with default setter for member type
	template <typename M>
	bool add_entry(M T::* ptr, const std::string& col_name) {
		size_t col_index;
		if (!find_column_index(col_name, col_index))
			return false;
		return add_entry(ptr, col_index);
	}
	/// add new entry based on heading of column with custom setter function
	bool add_entry(const std::string& col_name, std::function<bool(const std::string&, T&)> s) {
		size_t col_index;
		if (!find_column_index(col_name, col_index))
			return false;
		return add_entry(col_index, s);
	}
	/// parse csv file content into given data vector extracting added entries into members of objects of type T
	bool parse(std::vector<T>& data)
	{
		std::vector<cgv::utils::token> tokens;
		while (parse_next_line(tokens)) {
			T object;
			for (auto s : setters) {
				if (s.first >= tokens.size()) {
					last_error = "column index <" + cgv::utils::to_string(s.first) + 
						"> out of range [0," +cgv::utils::to_string(tokens.size()) + "[";
					return false;
				}
				if (!s.second(cgv::utils::to_string(tokens[s.first]), object)) {
					last_error = "could not parse <" + cgv::utils::to_string(tokens[s.first]) + ">";
					return false;
				}
			}
			data.emplace_back(object);
		}
		return true;
	}
};

	}
}

#include <cgv/config/lib_end.h>
