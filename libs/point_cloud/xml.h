#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>

#include "lib_begin.h"

namespace cgv {
namespace pointcloud {
namespace file_parser {

typedef std::pair<std::string, std::string> xml_attribute;
typedef std::map<xml_attribute::first_type, xml_attribute::second_type> xml_attribute_map;

enum XMLTagType {
	XTT_UNDEF,	 // undefined
	XTT_OPEN,	 // opening/starting tag
	XTT_CLOSE,	 // closing tag
	XTT_SINGLE,	 // singleton or self closing tag
	XTT_PREAMBLE,// preamble tag
	XTT_CDATA	 // cdata tag
};

struct xml_tag
{
	XMLTagType type = XTT_UNDEF;
	std::string name = "";
	xml_attribute_map attributes;
};


struct CGV_API xml_node
{
	std::vector<xml_node*> childs;

	//xml_tag tag;
	//std::vector<std::string> non_tag_content;

	//xml_node(const xml_tag& tag);

	virtual bool has_tag() const;

	virtual ~xml_node();

	void free_childs();

	void add_child(xml_node* node);

	//void add_content(const std::string& c);
};

struct CGV_API xml_string_node : public xml_node
{
	std::string content;

	xml_string_node(const std::string& str);
};


struct CGV_API xml_tag_node : public xml_node
{
	xml_tag tag;

	xml_tag_node(const xml_tag& tag);

	bool has_tag() const;
};

extern CGV_API std::unique_ptr<xml_tag_node> read_xml(const std::string& xml_content);




extern CGV_API xml_attribute xml_read_attribute(const std::string& attribute);

extern CGV_API xml_tag xml_read_tag(const std::string& str);


}
}
}

#include <cgv/config/lib_end.h>