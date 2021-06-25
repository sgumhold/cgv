#pragma once

#include <cgv/type/info/type_id.h>
#include <cgv/type/info/type_access.h>
#include "packing_info.h"
#include <string>
#include <vector>

#include "lib_begin.h"

using namespace cgv::type::info;

namespace cgv {
	namespace data {

/** define standard formats, which should be used to avoid wrong
    assignment of component names */
enum ComponentFormat {
	CF_UNDEF, /// undefinded format with no component
	CF_R,		 /// red channel of color format
	CF_G,		 /// green channel of color format
	CF_B,		 /// blue channel of color format
	CF_A,		 /// alpha format
	CF_L,     /// color format with luminance component L
	CF_I,     /// color format with intensity component I
	CF_LA,    /// color format with luminance and alpha components: L and A
	CF_IA,    /// color format with intensity and alpha components: I and A
	CF_RG,    /// color format with two components R and G
	CF_RGB,   /// color format with components R, G and B
	CF_RGBA,  /// color format with components R, G, B and A
	CF_BGR,   /// color format with components B, G and R
	CF_BGRA,  /// color format with components B, G, R and A
	CF_D,     /// depth component
	CF_S,     /// stencil component
	CF_P2D,   /// point format with components px and py 
	CF_P3D,   /// point format with components px, py and pz
	CF_P4D,   /// point format with components px, py, pz and pw
	CF_N2D,   /// normal format with components nx and ny
	CF_N3D,   /// normal format with components nx, ny and nz
	CF_T2D,   /// texture coordinats format with components ts and tt
	CF_T3D,   /// texture coordinats format with components ts, tt and tr
	CF_T4D,   /// texture coordinats format with components ts, tt, tr and tq
	CF_LAST   /// this is always the last entry in the component formats and used for iteration
};

/** define different interpretations of integer components */
enum ComponentIntegerInterpretation
{
	CII_DEFAULT, // default of used api such as opengl
	CII_SNORM,   // map signed integers to [-1,1]
	CII_INTEGER  // keep integer values even if api would map them to [0,1]
};

/** the component format inherits the information of a packing_info 
    and adds information on the component type, which components are 
	 present in the data and in which order they appear */
class CGV_API component_format : public packing_info
{ 
protected:
	/// store the type id of the component type
	TypeId component_type;
	/// interpretation of integer typed components
	ComponentIntegerInterpretation component_interpretation;
	/// store all component names in one string separated with 0-chars
	std::string component_string;
	/// store the position of each component in the component string
	std::vector<unsigned short> component_positions;
	/// extract components from component string
	void extract_components();
	/// store the last error that appeared during parsing of a description
	static std::string last_error;
public:
	/// construct from description string, see set_component_format for docu
	explicit component_format(const std::string& description);
	/** 
set component format from description string, which has the following syntax.
If a parse error arises, return false and set the static last_error member, which
can be queried with get_last_error():

\verbatim
component_format <- [type] [attributes] '[' component [',' component]* ']'

component <- component_name [attributes]

attributes <- [':' bit_depth]['|' alignment]

type <- "undef" | "bool" | 
		"int8"  | "int16"  | "int32"  | "int64"  | "uint8"  | "uint16"  | "uint32"  | "uint64" |
		"sint8" | "sint16" | "sint32" | "sint64" |                                                // same as int* types but in snorm interpretation
		"_int8" | "_int16" | "_int32" | "_int64" | "_uint8" | "_uint16" | "_uint32" | "_uint64" | // same as [u]int* but in integer interpretation
		"flt16" | "flt32"  | "flt64"  | "string"

component_name : string ... name of component, i.e. "R", "Cb", "px", ...

bit_depth : unsigned int ... number of bits used to represent a component

alignment : unsigned int ... number of bits to which a component is aligned
\endverbatim

Some examples of valid component format description strings:
- \c "uint8:3|4[R,G,B,A]" ...  four components represented as unsigned integers
                               with no more than 8 bits. Actually, each component is
										 stored with 3 bits and aligned to a bit index which 
										 is a multiple of 4
- \c "uint8[R:5,G:6,B:5]" ... three components packed into 16 bits with 5 bits for R,
							   6 for G and 5 for B.
- \c "sint8[R,G]"         ... two component format of type int8 in snorm interpretation,
- \c "_uint16[R]"         ... one component format of type uint16 in integer interpretation,
- \c "flt32[px,py]" ... two components of 32 bit floats

- \c "[D]" ... one depth component without specified type, which defaults
					to "undef" and implies that the default depth format should
					be used.
 */
	bool set_component_format(const std::string& description);
	/// comma separated list of component descriptors, for example "R,G,B"
	void set_components(const std::string& _components);
	/// returns an error string after parsing of description string has failed
	static const std::string& get_last_error();
	/** construct component format from component type, comma or colon 
		 separated list of component names, component alignment and bit 
		 depths for packed formats*/
	component_format(TypeId _component_type = TI_UNDEF, 
		const std::string& _component_name_list = "", 
		unsigned int align = 1, 
		unsigned int d0 = 0, unsigned int d1 = 0, 
		unsigned int d2 = 0, unsigned int d3 = 0);
	/// construct component format from component type, standard component format, component alignment and bit depths for packed formats
	component_format(TypeId _component_type, 
		ComponentFormat cf, 
		unsigned int align = 1, 
		unsigned int d0 = 0, unsigned int d1 = 0, 
		unsigned int d2 = 0, unsigned int d3 = 0);
	/// set the integer interpretation
	void set_integer_interpretation(ComponentIntegerInterpretation cii);
	/// return current integer interpretation
	ComponentIntegerInterpretation get_integer_interpretation() const;
	/// define stream out operator
	friend FRIEND_MEMBER_API std::ostream& operator << (std::ostream& os, const component_format& cf);
	/// constant access to the i-th component stored at the given location
	template <typename T>
	T get(int ci, const void* ptr) const {
		if (is_packing()) {
			if (get_component_type() <= TI_INT64) {
				int i = packing_info::get_signed(ci, ptr);
				return type_access<T>::get(&i, get_component_type());
			}
			else {
				unsigned int i = packing_info::get_unsigned(ci, ptr);
				return type_access<T>::get(&i, get_component_type());
			}
		}
		else
			return type_access<T>::get(static_cast<const unsigned char*>(ptr)+ci*align(get_type_size(get_component_type()),get_component_alignment()), get_component_type());
	}
	/// write access to the i-th component, return whether write was successful
	template <typename T>
	bool set(int ci, void* ptr, const T& v) const { 
		if (is_packing()) {
			if (get_component_type() <= TI_INT64) {
				int i = 0;
				return type_access<T>::set(&i, get_component_type(), v) && packing_info::set_signed(ci, ptr, i);
			}
			else {
				unsigned int i = 0;
				return type_access<T>::set(&i, get_component_type(), v) && packing_info::set_unsigned(ci, ptr, i);
			}
		}
		else
			return type_access<T>::set(static_cast<unsigned char*>(ptr)+ci*align(get_type_size(get_component_type()),get_component_alignment()), get_component_type(), v);
	}
	/// return whether the component format is defined
	bool empty() const;
	/// clear the component format
	void clear();
	/// return the packing info by simple conversion of the this pointer
	const packing_info& get_packing_info() const;
	/// set packing info by simply assigning to a converted this pointer
	void set_packing_info(const packing_info& pi);
	/// return the number of components
	unsigned int get_nr_components() const;
	/// return the index of a component given by name or -1 if not found
	unsigned int get_component_index(const std::string& name) const;
	/// return the name of the component with index i
	std::string get_component_name(unsigned int i) const;
	/// return whether the component format is one of the standard formats
	ComponentFormat get_standard_component_format() const;
	/// set component names from a comma or colon separated list of names
	void set_component_names(const std::string& _component_name_list);
	/// set the component names from a standard component format
	void set_component_format(ComponentFormat _cf);
	/// return the component type
	TypeId get_component_type() const;
	/// set the component type
	void set_component_type(TypeId _type_id);
	/// return the size of one entry of components in bytes
	unsigned int get_entry_size() const;
	/// comparison between component formats
	bool operator == (const component_format& cf) const;
	/// comparison between component formats
	bool operator != (const component_format& cf) const;
};

/** stream out operator writes the component format in the syntax of description strings
    as defined in the docu of set_component_format(). */
extern CGV_API std::ostream& operator << (std::ostream& os, const component_format& cf);

/// default function to check whether fmt1 is a better match to fmt than fmt2
extern CGV_API bool fmt1_compares_better(const component_format& fmt,
					                          const component_format& fmt1,
					                          const component_format& fmt2);

/** find the best matching format in a list of formats described by strings 
    and return index of best match */
extern CGV_API unsigned int find_best_match(
				const component_format& fmt,
				const char** format_descriptions,
				const component_format* fmt0 = 0,
				bool (*fmt1_better_match)(
				   const component_format& fmt,
					const component_format& fmt1,
					const component_format& fmt2) = fmt1_compares_better);

	}
}

#include <cgv/config/lib_end.h>
