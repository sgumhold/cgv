#pragma once

#include "operators.h"
#include <vector>
#include <map>

#include "lib_begin.h"

namespace cgv {
	namespace media {
		namespace text {
			namespace ppp {

class ph_processor;
struct namespace_info;

enum ValueType { UNDEF_VALUE, BOOL_VALUE, INT_VALUE, DOUBLE_VALUE, STRING_VALUE, 
                 REFERENCE_VALUE, NAME_VALUE, LIST_VALUE, MAP_VALUE, FUNC_VALUE };

struct func_type
{
	unsigned block_begin;
	unsigned block_end;
	ph_processor* ph_proc;
	namespace_info* ns;
	func_type(unsigned _i, unsigned _j, ph_processor* _ph_proc = 0, namespace_info* _ns = 0);
};

class CGV_API variant
{
public:
	typedef std::vector<variant> list_type;
	typedef std::map<std::string,variant> map_type;
protected:
	/// store type of value
	ValueType vt;
	union {
		bool bool_value;
		int int_value;
		double dbl_value;
		std::string* string_value;
		variant* reference_value;
		std::string* name_value;
		list_type* list_value;
		map_type* map_value;
		func_type* func_value;
	};
public:
	/**@name constructors*/
	//@{
	/// construct undefined value
	variant();
	/// use assign operator for copy constructor
	variant(const variant& v);
	/// construct bool value
	variant(bool v);
	/// construct integer value
	variant(int v);
	/// construct double value
	variant(double v);
	/// construct string value
	variant(const std::string& v);
	/// construct reference value
	variant(variant* v);
	/// construct name value
	variant(ValueType vt, const std::string& v);
	/// construct list value
	variant(const list_type& v);
	/// construct map value
	variant(const map_type& v);
	/// construct func value
	variant(const func_type& v);
	/// remove all elements from a list or map and set to undefined type
	void clear();
	/// assignment operator
	variant& operator = (const variant& v);
	/// destructor
	~variant();
	//@}

	/**@name operator interface*/
	//@{
	bool is_unary_applicable(OperatorType ot);
	bool unary_check_defined(OperatorType ot);
	void apply_unary(OperatorType ot);
	bool is_binary_applicable(OperatorType ot, const variant& v2);
	bool binary_check_defined(OperatorType ot, const variant& v2);
	void apply_binary(OperatorType ot, const variant& v2);
	//@}

	/**@name query type*/
	//@{
	/// return the variant type
	ValueType get_type() const;
	/// lookup names and follow references and return value type
	ValueType get_value_type() const;
	/// lookup names and follow references and return whether variant is undefined
	bool      is_undefined() const;
	/// lookup names and follow references and return whether variant is int
	bool      is_int() const;
	/// lookup names and follow references and return whether variant is double
	bool      is_double() const;
	/// lookup names and follow references and return whether variant is bool
	bool      is_bool() const;
	/// lookup names and follow references and return whether variant is string
	bool      is_str() const;
	/// lookup names and follow references and return whether variant is list
	bool      is_list() const;
	/// lookup names and follow references and return whether variant is map
	bool      is_map() const;
	/// lookup names and follow references and return whether variant is func
	bool      is_func() const;
	/// name and reference type return true
	bool      is_reference() const;
	/// only a name returns true
	bool      is_name() const;
	//@}

	/**@name access to values with implicit type conversions accept list and map types */
	//@{
	/// lookup names and follow references and convert to bool: undef ... false, int ... compares unequal zero, string ... convert to int and compare unequal zero, list/map ... check size unequal zero
	bool        get_bool() const;
	/// lookup names and follow references and convert to int: undef ... -1, bool ... 0 or 1, string ... atoi, list/map ... size
	int         get_int() const;
	/// lookup names and follow references and convert to double: undef ... -1, bool ... 0 or 1, string ... atof, list/map ... size
	double		get_double() const;
	/// lookup names and follow references and convert to string
	std::string get_str() const;
	/// constant access to list value
	const list_type& get_list() const;
	/// constant access to map value
	const map_type& get_map() const;
	/// convert to int type
	void ensure_int_type();
	/// convert to int or double such that result of binary operators can be stored in this variant without loss of data, return whether conversion was to int
	bool match_number_type(const variant& v2);
	//@}

	/**@name references to values. Before usage ensure that type is matched. */
	//@{
	bool&   ref_bool();
	int&    ref_int();
	double& ref_double();
	std::string& ref_str();
	list_type& ref_list();
	map_type& ref_map();
	func_type& ref_func();
	//@}

	/**@name setters*/
	//@{
	void set_bool(bool v);
	void set_int(int v);
	void set_double(double v);
	void set_str(const std::string& v);
	void set_name(const std::string& n);
	void set_list();
	void set_list(const std::vector<variant>& l);
	void set_map();
	void set_map(const std::map<std::string,variant>& m);
	//@}

	/**@name name and reference interface*/
	//@{
	/// lookup names and follow references and return the reached variant
	const variant&     get_value() const;
	/// lookup names and follow references and return reference to the reached variant
	variant&		   ref_value();
	/// return the pointer of a reference
	variant*           get_reference() const;
	/// return the name of a name value
	const std::string& get_name() const;
	//@}

	/**@name common list and map interface*/
	//@{
	/// return number of elements in a list or map
	unsigned int get_size() const;
	/// return total number of elements in a list or map summing over all elements recursively
	unsigned get_total_nr_elements() const;
	/// return a reference to the i-th element in a list or map
	variant& ref_element(unsigned int i);
	/// return a const reference to the i-th element in a list or map
	const variant& get_element(unsigned int i) const;
	//@}

	/**@name list interface*/
	//@{
	void append_to_list(const variant& v);
	void prepend_to_list(const variant& v);
	void pop_back_from_list();
	void pop_front_from_list();
	//@}

	/**@name map interface*/
	//@{
	/// reference an element of a map by name
	variant& ref_element(const std::string& name);
	/// const reference an element of a map by name
	const variant& get_element(const std::string& name);
	/// return the name of the i-th element in a map
	const std::string& get_element_name(unsigned int i) const;
	/// insert a new entry to the map
	void insert(const std::string& name, const variant& v);
	//@}
};

/// stream out a variant
extern CGV_API std::ostream& operator << (std::ostream& os, const variant& v);

			}
		}
	}
}
#include <cgv/config/lib_end.h>