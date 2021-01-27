
// C++ STL
#include <cmath>
#include <array>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <utility>
#include <limits>

// CGV framework core
#include <cgv/base/register.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/advanced_scan.h>

// local includes
#include "regulargrid.h"

// implemented header
#include "csv_handler.h"


////
// Local types and variables

// anonymous namespace begin
namespace {

// ToDo: make this run-time customizable
const std::string csv_separators(",");

// ToDo: REMOVE
const struct attrib_suggestion_struct
{
	const visual_attribute_mapping map_user;
	const visual_attribute_mapping map_device;

	attrib_suggestion_struct()
		: map_user{{VisualAttrib::COLOR, "color"}, {VisualAttrib::RADIUS, "radius"}, {VisualAttrib::TANGENT, "tangents"}},
		  map_device{{VisualAttrib::COLOR, "color"}, {VisualAttrib::RADIUS, "radius"}, {VisualAttrib::TANGENT, "tangents"}}
	{}
} attrib_suggestion;

// Anonymous namespace end
}


////
// Class implementation - csv_descriptor

struct csv_descriptor::Impl
{
	// fields
	bool needs_header=false;
	std::vector<attribute> attribs;
};

csv_descriptor::csv_descriptor() : pimpl(nullptr)
{
	pimpl = new Impl;
}

csv_descriptor::csv_descriptor(const csv_descriptor &other) : csv_descriptor()
{
	pimpl->needs_header = other.pimpl->needs_header;
	pimpl->attribs = other.pimpl->attribs;
}

csv_descriptor::csv_descriptor(csv_descriptor &&other) : pimpl(other.pimpl)
{
	other.pimpl = nullptr;
}

csv_descriptor::csv_descriptor(const std::vector<attribute> &attributes) : csv_descriptor()
{
	auto &attribs = pimpl->attribs = attributes;
	pimpl->needs_header = will_need_header(attribs);
}

csv_descriptor::csv_descriptor(std::vector<attribute> &&attributes) : csv_descriptor()
{
	auto &attribs = pimpl->attribs = std::move(attributes);
	pimpl->needs_header = will_need_header(attribs);
}

csv_descriptor::~csv_descriptor()
{
	if (pimpl)
		delete pimpl;
}

csv_descriptor& csv_descriptor::operator= (const csv_descriptor &other)
{
	pimpl->attribs = other.pimpl->attribs;
	pimpl->needs_header = other.pimpl->needs_header;
	return *this;
}

csv_descriptor& csv_descriptor::operator= (csv_descriptor &&other)
{
	this->~csv_descriptor();
	pimpl = other.pimpl;
	other.pimpl = nullptr;
	return *this;
}

const std::vector<csv_descriptor::attribute>& csv_descriptor::attributes (void) const
{
	return pimpl->attribs;
}

bool csv_descriptor::will_need_header (const std::vector<attribute> &attributes)
{
	for (const auto &attrib : attributes)
		for (const auto &col : attrib.columns)
			if (col.number < 0)
				return true;
	return false;
}


////
// Class implementation - csv_handler

template <class flt_type>
struct csv_handler<flt_type>::Impl
{
	// types
	typedef flt_type real;
	typedef typename csv_handler::Vec3 Vec3;
	typedef typename csv_handler::Vec4 Vec4;
	typedef typename csv_handler::rgb rgb;

	// fields
	csv_table_descriptor table_desc;
	csv_descriptor csv_desc;
	bool desc_valid=false, has_data=false, device=false;
	real avg_dist = 0;

	// helper methods
	void common_init (void)
	{
		desc_valid = csv_handler<flt_type>::is_csv_descriptor_valid(csv_desc);
		if (!desc_valid)
			std::cerr << "csv_handler: WARNING - invalid description of .csv data structure supplied!"
			          << std::endl << std::endl;
	}
	bool is_separator (const cgv::utils::token &token, const std::vector<char> &separators)
	{
		// only 1-char tokens can be separators
		if (token.end - token.begin != 1)
			return false;

		// check against each given separator
		for (char sep : separators)
			if (sep == *token.begin)
				return true;
		return false;
	}
	unsigned enum_fields (
		const std::vector<cgv::utils::token> &tokens, const std::string &separators, std::vector<std::string> *out=nullptr
	)
	{
		// build list of separators
		std::vector<char> seps(separators.begin(), separators.end());

		// count each token that is not a separator as a field
		unsigned count = 0;
		const bool convert = out != nullptr;
		for (auto &token : tokens)
			if (!is_separator(token, seps))
			{
				count++;
				if (convert)
					out->emplace_back(std::string(token.begin, size_t(token.end - token.begin)));
			}

		// account for empty last field (indicated by last token being a separator)
		if (is_separator(tokens.back(), seps))
		{
			count++;
			if (convert)
				out->emplace_back();
		}

		// done
		return count;
	}
	void read_fields (
		std::vector<std::string>* out, const std::string &line, const std::string &separators
	)
	{
		// build list of separators
		std::vector<char> seps(separators.begin(), separators.end());

		// split line to tokens
		std::vector<cgv::utils::token> tokens;
		cgv::utils::split_to_tokens(line, tokens, csv_separators, false);

		// count each token that is not a separator as a field
		unsigned count = 0;
		std::vector<std::string> fields; fields.reserve(table_desc.columns.size());
		for (auto &token : tokens)
			if (!is_separator(token, seps))
				fields.emplace_back(std::string(token.begin, size_t(token.end - token.begin)));

		// account for empty last field (indicated by last token being a separator)
		if (is_separator(tokens.back(), seps))
			fields.emplace_back();

		// done
		*out = std::move(fields);
	}
	bool check_header (const std::vector<std::string> &header_fields)
	{
		// check if each required field is present
		if (header_fields.size() < table_desc.columns.size())
			return false;
		for (unsigned i=0; i<table_desc.columns.size(); i++)
		{
			const auto &col = table_desc.columns[i];
			auto it = std::find_if(
				header_fields.begin(), header_fields.end(),
				[&col] (const std::string &field) -> bool {
					
					if (col.case_sensitive)
						return field.compare(col.name) == 0;
					else
						return cgv::utils::to_lower(field).compare(col.name) == 0;
				}
			);
			if (it == header_fields.end())
				return false;
		}

		return true;
	}
	bool is_header (const std::vector<std::string> &line_fields)
	{
		// in addition to being the very first line in a .csv file that actually contains something, we require the
		// headerfields to contain absolutely zero whole strings that can be converted to a number
		for (const auto &field : line_fields)
		{
			double test = 0; // overflow due to too-large values should not affect this test, therefor we use double always
			size_t pos = 0;
			try { test = std::stod(field, &pos); }
			catch (...) { /* DoNothing() */; }
			if (pos > 0)
				return false;
		}
		return true;
	}
	bool special_fields_not_readable (const std::vector<std::string> &line_fields)
	{
		for (unsigned i=0; i<table_desc.columns.size(); i++)
		{
			const auto &col = table_desc.columns[i];
			if (col.semantics != CSV::ARBITRARY)
			{
				double test = 0;
				size_t pos = 0;
				try { test = std::stod(line_fields[i], &pos); }
				catch (const std::out_of_range&) { /* DoNothing() */; }
				catch (...) { return true; }
				if (!pos)
					return true;
			}
		}
		return false;
	}
};

template <class flt_type>
csv_handler<flt_type>::csv_handler() : pimpl(nullptr)
{
	pimpl = new Impl;
}

template <class flt_type>
csv_handler<flt_type>::csv_handler(const csv_table_descriptor &table_desc) : csv_handler()
{
	pimpl->table_desc = table_desc;
}

template <class flt_type>
csv_handler<flt_type>::csv_handler(csv_table_descriptor &&table_desc) : csv_handler()
{
	pimpl->table_desc = std::move(table_desc);
}

template <class flt_type>
csv_handler<flt_type>::csv_handler(const csv_descriptor &csv_desc) : csv_handler()
{
	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// commit descriptor
	impl.csv_desc = csv_desc;
	impl.common_init();
}

template <class flt_type>
csv_handler<flt_type>::csv_handler(csv_descriptor &&csv_desc) : csv_handler()
{
	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// commit descriptor
	impl.csv_desc = std::move(csv_desc);
	impl.common_init();
}

template <class flt_type>
csv_handler<flt_type>::~csv_handler()
{
	if (pimpl)
		delete pimpl;
}

template <class flt_type>
void csv_handler<flt_type>::cleanup (void)
{
	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// reset our private fields
	impl.has_data = false;
	impl.device = false;
	impl.avg_dist = 0;
}

template <class flt_type>
bool csv_handler<flt_type>::can_handle (std::istream &contents) const
{
	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// init
	std::string line;
	stream_pos_guard g(contents);

	// check for tell-tale stream contents
	// - parse first row
	do { std::getline(contents, line); } while (!contents.eof() && line.empty());

	std::vector<cgv::utils::token> tokens;
	cgv::utils::split_to_tokens(line, tokens, csv_separators, false);
	std::vector<std::string> columns;
	unsigned num_cols = impl.enum_fields(tokens, csv_separators, &columns);
	if (impl.table_desc.header)
	{
		if (impl.check_header(columns))
			// header matches up, so at this point we don't do any further tests and just assume it'll work
			return true;
		else
			return false;
	}
	else
	{
		if (num_cols != impl.table_desc.columns.size())
			return false;
		// also actually process up to two lines in the stream to see if we can actually interpret the text-encoded data
		if (impl.is_header(columns))
			do { std::getline(contents, line); } while (!contents.eof() && line.empty());
		impl.read_fields(&columns, line, csv_separators);
		if (impl.special_fields_not_readable(columns))
			return false;
	}

	// - apparently a valid .csv that we can interpret
	return true;
}

template <class flt_type>
bool csv_handler<flt_type>::read (std::istream &contents, unsigned idx_offset)
{
	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// route csv columns to internal attribute representation
	std::string line;
	std::vector<std::string> fields;
	std::map<CSV_ColumnSemantics, unsigned> special_col_map;
	std::map<unsigned, std::vector<real>*> misc_field_map;
	attribute_map<flt_type> misc_attribs;
	bool has_traj_id=false, has_pos_x=false, has_pos_y=false, has_pos_z=false;
	if (impl.table_desc.header)
	{
		std::getline(contents, line);
		impl.read_fields(&fields, line, csv_separators);
		for (unsigned i=0; i<fields.size(); i++)
		{
			const auto &field = fields[i];
			auto it = std::find_if(
				impl.table_desc.columns.begin(), impl.table_desc.columns.end(),
				[&field] (const csv_table_descriptor::column &col) -> bool {
					if (col.case_sensitive)
						return col.name.compare(field) == 0;
					else
						return col.name.compare(cgv::utils::to_lower(field)) == 0;
				}
			);
			if (it != impl.table_desc.columns.end() && it->semantics != CSV::ARBITRARY)
			{
				if (it->semantics == CSV::TRAJ_ID)
					has_traj_id = true;
				else if (it->semantics == CSV::POS_X)
					has_pos_x = true;
				else if (it->semantics == CSV::POS_Y)
					has_pos_y = true;
				else if (it->semantics == CSV::POS_Z)
					has_pos_z = true;
				special_col_map[it->semantics] = i;
			}
			else
			{
				// Create a misc attribute for this column
				auto new_entry = misc_attribs.emplace(field, std::vector<real>());
				misc_field_map[i] = &(new_entry.first->second.get_data<real>());
			}
		}
	}
	else
	{
	}

	// factor out some decisions about what position attributes are present from the main loader loop below
	enum class PosMode { XYZ, XY, XZ, YZ, X_OR_Y_OR_Z } pos_mode =
		(has_pos_x && has_pos_y && has_pos_z) ?
			PosMode::XYZ :
			((has_pos_x && has_pos_y) ?
				PosMode::XY :
				((has_pos_x && has_pos_z) ?
					PosMode::XZ :
					((has_pos_y && has_pos_z) ? PosMode::YZ : PosMode::X_OR_Y_OR_Z)
				)
			);

	// trajectory database
	double dist_accum = 0;
	std::vector<Vec3> P; std::vector<rgb> C;
	std::map<int, std::vector<unsigned> > trajs;

	// determine if we're loading one of the IML tracking datasets for on-the-fly coordinate system correction and later color map selection
	// (ToDo: quick-and-dirty hack for demo purposes, needs proper generic transfer function infrastructure)
	const bool user = impl.table_desc.columns.size() == 5 && impl.table_desc.columns[1].name.compare("id") == 0,
	           device = impl.table_desc.columns.size() == 8 && impl.table_desc.columns[1].name.compare("userid") == 0;

	// parse the stream until EOF
	do {
		// parse whole line (and skip empty ones)
		std::getline(contents, line);
		if (line.empty())
			continue;
		impl.read_fields(&fields, line, csv_separators);

		// determine trajectory id of this row
		unsigned traj_id;
		if (has_traj_id)
		{
			int traj_id_field = special_col_map[CSV::TRAJ_ID];
			traj_id = std::atoi(fields[traj_id_field].c_str());
		}
		else
			traj_id = 0;

		// current sample index
		unsigned current_idx = (unsigned)P.size();

		// store special position attribute
		rgb sample_color(real(3)/real(4));
		if (user)
		{
			// ToDo: REMOVE - special handling for IML user tracking dataset
			P.emplace_back(
				real(std::stod(fields[special_col_map[CSV::POS_X]])) *-10,
				real(std::stod(fields[special_col_map[CSV::POS_Y]])) * 10,
				real(std::stod(fields[special_col_map[CSV::POS_Z]])) *-10
			);
			sample_color = traj_id==1 ? rgb{ 178.f/255.f, 223.f/255.f, 138.f/255.f } : rgb{ 51.f/255.f, 160.f/255.f, 44.f/255.f };
		}
		else if (device)
		{
			// ToDo: REMOVE - special handling for IML device tracking dataset
			P.emplace_back(
				real(std::stod(fields[special_col_map[CSV::POS_X]])) * 10,
				real(std::stod(fields[special_col_map[CSV::POS_Y]])) * 10,
				real(std::stod(fields[special_col_map[CSV::POS_Z]])) * 10
			);
			sample_color = traj_id==1 ? rgb{ 166.f/255.f, 206.f/255.f, 227.f/255.f } : rgb{ 31.f/255.f, 120.f/255.f, 180.f/255.f };
		}
		else if (pos_mode == PosMode::XYZ)
			P.emplace_back(
				real(std::stod(fields[special_col_map[CSV::POS_X]])),
				real(std::stod(fields[special_col_map[CSV::POS_Y]])),
				real(std::stod(fields[special_col_map[CSV::POS_Z]]))
			);
		else if (pos_mode == PosMode::XY)
			P.emplace_back(
				real(std::stod(fields[special_col_map[CSV::POS_X]])),
				real(std::stod(fields[special_col_map[CSV::POS_Y]])), real(0)
			);
		else if (pos_mode == PosMode::XZ)
			P.emplace_back(
				real(std::stod(fields[special_col_map[CSV::POS_X]])), real(0),
				real(std::stod(fields[special_col_map[CSV::POS_Z]]))
			);
		else if (pos_mode == PosMode::YZ)
			P.emplace_back(
				real(0), real(std::stod(fields[special_col_map[CSV::POS_Y]])),
				real(std::stod(fields[special_col_map[CSV::POS_Z]]))
			);
		else if (has_pos_x)
			P.emplace_back(real(std::stod(fields[special_col_map[CSV::POS_X]])), real(0), real(0));
		else if (has_pos_y)
			P.emplace_back(real(0), real(std::stod(fields[special_col_map[CSV::POS_Y]])), real(0));
		else if (has_pos_z)
			P.emplace_back(real(0), real(0), real(std::stod(fields[special_col_map[CSV::POS_Z]])));

		// ToDo: move color generation out of the loading routine once transfer functions are supported
		C.emplace_back(sample_color);

		// store the other attributes
		for (const auto &f : misc_field_map)
		{
			unsigned field_id = f.first;
			std::vector<real> &attrib = *f.second;
			real val;
			try { val = (real)std::stod(fields[field_id]); }
			catch (const std::out_of_range&) { val = std::numeric_limits<real>::infinity(); }
			catch (...) { val = 0; }
			attrib.emplace_back(val);
		}

		// store index for appropriate trajectory
		auto &indices = trajs[traj_id];
		indices.emplace_back(current_idx);
		if (indices.size() > 1)
		{
			// accumulate the segment length
			dist_accum += (P.back() - P[*(indices.end()-2)]).length();
			// duplicate index to create new segment under line-list semantics
			indices.emplace_back(current_idx);
		}
	}
	while (!contents.eof());

	// did we succeed at loading anything?
	if (trajs.size() < 1)
		return false;

	// commit attributes to common curve representation
	positions() = std::move(P);
	attributes().emplace("color", std::move(C));
	auto &I = indices();
	for (auto &traj : trajs)
		I.insert(I.end(), traj.second.begin(), traj.second.end()-1);
	unsigned num_segs = (unsigned)(I.size() / 2);
	impl.avg_dist = real(dist_accum / double(num_segs));

	// Invent radii
	size_t num_samples = positions().size();
	real radius = impl.avg_dist * (user ? real(0.125) : real(0.25));
	attributes().emplace("radius", std::vector<real>(num_samples, radius));

	// Invent tangents
	const auto &pos = positions();
	auto &dP = attributes().emplace(
		"tangents", std::vector<Vec4>(num_samples, {0, 0, 0, 0})
	).first->second.get_data<Vec4>();
	for (auto &traj : trajs)
	{
		auto &ids = traj.second;
		Vec3 tangent = (pos[ids[1]] - pos[ids.front()]);
		dP[ids.front()] = vec4_from_vec3s(tangent, 0);
		for (unsigned i=1; i<ids.size()-2; i+=2)
		{
			Vec3 diff0 = pos[ids[i]] - pos[ids[i-1]], diff1 = pos[ids[i+2]] - pos[ids[i]];
			real len0 = diff0.length(), len1 = diff1.length(),
			     len = std::max(radius/real(2), std::min(len0, len1));
			tangent = cgv::math::normalize(diff0 + diff1) * len;
			dP[ids[i]] = vec4_from_vec3s(tangent, 0);
		}
		tangent = (pos[ids.back()] - pos[*(ids.end()-2)]);
		dP[ids.back()] = vec4_from_vec3s(tangent, 0);
		ids.clear();
	}

	// print some stats
	std::cout << "csv_handler: loading completed! Stats:" << std::endl
	          << "  " << num_samples<<" samples" << std::endl
	          << "  " << num_segs<<" segments" << std::endl
	          << "  " << trajs.size()<<" trajectories" << std::endl << std::endl;

	// done
	return true;
}

template <class flt_type>
bool csv_handler<flt_type>::has_data (void) const
{
	return pimpl->has_data;
}

template <class flt_type>
flt_type csv_handler<flt_type>::avg_segment_length (void) const
{
	return pimpl->avg_dist;
}

template <class flt_type>
const visual_attribute_mapping& csv_handler<flt_type>::suggest_mapping (void) const
{
	return attrib_suggestion.map_user;
}

template <class flt_type>
bool csv_handler<flt_type>::is_csv_descriptor_valid (const csv_descriptor &csv_desc)
{
	bool pos_found=false, traj_id_found=false;
	for (const auto &attrib : csv_desc.attributes())
	{
		if (attrib.semantics == CSVA::POS && pos_found)
			return false;
		else if (attrib.semantics == CSVA::POS)
		{
			pos_found = true;
			if (attrib.columns.empty() || attrib.columns.size() > 3)
				return false;
		}
		else if (attrib.semantics == CSVA::TRAJ_ID && traj_id_found)
			return false;
		else if (attrib.semantics == CSVA::TRAJ_ID)
		{
			traj_id_found = true;
			if (attrib.columns.empty() || attrib.columns.size() > 1)
				return false;
		}
	}
	return pos_found;
}

template <class flt_type>
bool csv_handler<flt_type>::is_csv_descriptor_multi_traj (const csv_descriptor &csv_desc)
{
	for (const auto &attrib : csv_desc.attributes())
		if (attrib.semantics == CSVA::TRAJ_ID)
			return true;
	return false;
}


////
// Explicit template instantiations

// Only float and double variants are intended
template class csv_handler<float>;
template class csv_handler<double>;


////
// Object registration

// Register example handler for the IML multi-user study .csv files (float and double versions)
static const csv_descriptor csv_imluser_desc({
	{ "timestamp", {"timestamp", false, 0} },
	{ "userid",    {"userid", false, 1}, CSVA::TRAJ_ID },
	{ "",         {{"pos_1", false, 4}, {"pos_2", false, 5}, {"pos_3", false, 6}}, CSVA::POS }
});
cgv::base::object_registration_1<csv_handler<float>, csv_descriptor> flt_csv_imluser_reg(
	csv_imluser_desc, "IML user trajectory csv handler (float)"
);
cgv::base::object_registration_1<csv_handler<double>, csv_descriptor> dbl_csv_imluser_reg(
	csv_imluser_desc, "IML user trajectory csv handler (double)"
);
/*cgv::base::object_registration_1<csv_handler<float>, csv_table_descriptor> flt_csv_reg(
	csv_table_descriptor(
		{ "timestamp",
		 {"id",    CSV::TRAJ_ID},
		 {"pos_1", CSV::POS_X},
		 {"pos_2", CSV::POS_Y},
		 {"pos_3", CSV::POS_Z}},
		true
	),
	"IML multi-view study user trajectory handler (float)"
);
cgv::base::object_registration_1<csv_handler<float>, csv_table_descriptor> dbl_csv_reg(
	csv_table_descriptor(
		{ "timestamp",
		 {"userid",     CSV::TRAJ_ID},
		 {"spacePos_1", CSV::POS_X},
		 {"spacePos_2", CSV::POS_Y},
		 {"spacePos_3", CSV::POS_Z},
		  "orientation_1", "orientation_2", "orientation_3"},
		true
	),
	"IML multi-view study device trajectory handler (float)"
);*//*
cgv::base::object_registration_1<csv_handler<float>, csv_table_descriptor> flt_csv_imluser_reg(
	csv_table_descriptor(
		{ "timestamp",
		 {"id",    CSV::TRAJ_ID},
		 {"pos_1", CSV::POS_X},
		 {"pos_2", CSV::POS_Y},
		 {"pos_3", CSV::POS_Z},
		  "condition"}
	),
	"IML multi-view study user trajectory handler (float)"
);
cgv::base::object_registration_1<csv_handler<float>, csv_table_descriptor> flt_csv_imldevice_reg(
	csv_table_descriptor(
		{ "timestamp",
		 {"userid",     CSV::TRAJ_ID},
		  "screenPos_1",
		  "screenPos_2",
		 {"spacePos_1", CSV::POS_X},
		 {"spacePos_2", CSV::POS_Y},
		 {"spacePos_3", CSV::POS_Z},
		  "orientation_1", "orientation_2", "orientation_3", "condition"}
	),
	"IML multi-view study device trajectory handler (float)"
);*/
