
// C++ STL
#include <cmath>
#include <array>
#include <vector>
#include <map>
#include <set>
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
	csv_descriptor::csv_properties props;
	std::vector<attribute> attribs;
	std::string separators;
};

csv_descriptor::csv_descriptor() : pimpl(nullptr)
{
	pimpl = new Impl;
}

csv_descriptor::csv_descriptor(const csv_descriptor &other) : csv_descriptor()
{
	pimpl->props = other.pimpl->props;
	pimpl->attribs = other.pimpl->attribs;
	pimpl->separators = other.pimpl->separators;
}

csv_descriptor::csv_descriptor(csv_descriptor &&other) : pimpl(other.pimpl)
{
	other.pimpl = nullptr;
}

csv_descriptor::csv_descriptor(const std::string &separators, const std::vector<attribute> &attributes)
	: csv_descriptor()
{
	pimpl->separators = separators;
	auto &attribs = pimpl->attribs = attributes;
	pimpl->props = infer_properties(attribs);
}

csv_descriptor::csv_descriptor(std::string &&separators, const std::vector<attribute> &attributes)
	: csv_descriptor()
{
	pimpl->separators = std::move(separators);
	auto &attribs = pimpl->attribs = attributes;
	pimpl->props = infer_properties(attribs);
}

csv_descriptor::csv_descriptor(const std::string &separators, std::vector<attribute> &&attributes) : csv_descriptor()
{
	pimpl->separators = separators;
	auto &attribs = pimpl->attribs = std::move(attributes);
	pimpl->props = infer_properties(attribs);
}

csv_descriptor::csv_descriptor(std::string &&separators, std::vector<attribute> &&attributes) : csv_descriptor()
{
	pimpl->separators = std::move(separators);
	auto &attribs = pimpl->attribs = std::move(attributes);
	pimpl->props = infer_properties(attribs);
}

csv_descriptor::~csv_descriptor()
{
	if (pimpl)
		delete pimpl;
}

csv_descriptor& csv_descriptor::operator= (const csv_descriptor &other)
{
	pimpl->props = other.pimpl->props;
	pimpl->attribs = other.pimpl->attribs;
	pimpl->separators = other.pimpl->separators;
	return *this;
}

csv_descriptor& csv_descriptor::operator= (csv_descriptor &&other)
{
	this->~csv_descriptor();
	pimpl = other.pimpl;
	other.pimpl = nullptr;
	return *this;
}

const std::string& csv_descriptor::separators (void) const
{
	return pimpl->separators;
}

const std::vector<csv_descriptor::attribute>& csv_descriptor::attributes (void) const
{
	return pimpl->attribs;
}

const csv_descriptor::csv_properties& csv_descriptor::properties (void) const
{
	return pimpl->props;
}

csv_descriptor::csv_properties csv_descriptor::infer_properties (const std::vector<attribute> &attributes)
{
	csv_properties ret;
	ret.header = false;
	ret.multi_traj = false;
	unsigned num_cols = 0; signed max_col_id = 0;
	for (unsigned i=0; i<attributes.size(); i++)
	{
		const auto &attrib = attributes[i];
		num_cols += (unsigned)attrib.columns.size();

		if (attrib.semantics == CSV::POS)
			ret.pos_id = i;
		else if (attrib.semantics == CSV::TRAJ_ID)
		{
			ret.multi_traj = true;
			ret.traj_id = i;
		}

		for (const auto &col : attrib.columns)
		{
			max_col_id = std::max(col.number, max_col_id);
			if (col.number < 0)
				ret.header = true;
		}
	}
	ret.max_col_id = std::max(num_cols-1, (unsigned)max_col_id);
	return ret;
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
	struct declared_attrib
	{
		std::vector<unsigned> field_ids;
		traj_attribute<real> attrib;
		declared_attrib(size_t num_cols) : attrib((unsigned)num_cols)
		{
			field_ids.reserve(num_cols);
		}
	};
	struct undeclared_attrib
	{
		std::string name;
		unsigned field_id;
		traj_attribute<real> attrib;
		undeclared_attrib(size_t num_cols) : attrib((unsigned)num_cols) {}
	};

	// fields
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
	bool check_header (const std::vector<std::string> &header_fields)
	{
		for (const auto &attrib : csv_desc.attributes())
			for (const auto &col : attrib.columns)
			{
				bool not_found=true;
				if (col.case_sensitive)
				{
					for (const auto &field : header_fields)
						if (field.compare(col.name) == 0)
						{
							not_found = false;
							break;
						}
				}
				else for (const auto &field : header_fields)
					if (    cgv::utils::to_lower(field).compare(cgv::utils::to_lower(col.name))
					     == 0)
					{
						not_found = false;
						break;
					}
				if (not_found)
					return false;
			}
		return true;
	}
	bool special_fields_not_readable (const std::vector<std::string> &line_fields)
	{
		for (const auto &attrib : csv_desc.attributes())
			if (attrib.semantics != CSV::ARBITRARY)
				for (const auto &col : attrib.columns)
				{
					double test = 0;
					size_t pos = 0;
					try { test = std::stod(line_fields[col.number], &pos); }
					catch (const std::out_of_range&) { /* DoNothing() */; }
					catch (...) { return true; }
					if (!pos)
						return true;
				}
		return false;
	}
	static bool is_header (const std::vector<std::string> &line_fields)
	{
		// in addition to being the very first line in a .csv file that actually contains something, we require the
		// headerfields to contain absolutely no number-convertible whole strings whatsoever
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
	static bool is_separator (const cgv::utils::token &token, const std::string &separators)
	{
		// only 1-char tokens can be separators
		if (token.end - token.begin != 1)
			return false;

		// check against each given separator
		for (const char sep : separators)
			if (sep == *token.begin)
				return true;
		return false;
	}
	static unsigned read_fields (
		const std::vector<cgv::utils::token> &tokens, const std::string &separators,
		std::vector<std::string> *out=nullptr
	)
	{
		// count each token that is not a separator as a field
		unsigned count = 0;
		std::vector<std::string> fields;
		for (const auto &token : tokens)
			if (!is_separator(token, separators))
			{
				count++;
				if (out)
					fields.emplace_back(std::string(token.begin, size_t(token.end - token.begin)));
			}

		// account for empty last field (indicated by last token being a separator)
		if (is_separator(tokens.back(), separators))
		{
			count++;
			if (out)
				fields.emplace_back();
		}

		// Commit field contents if requested
		if (out)
			*out = std::move(fields);

		// done
		return count;
	}
	static unsigned read_next_nonempty_line (
		std::string *line_out, std::vector<cgv::utils::token> *tokens,
		const std::string &separators, std::istream &contents, std::vector<std::string> *fields_out=nullptr
	)
	{
		unsigned num_tokens = 0;
		do {
			std::getline(contents, *line_out);
			if (!line_out->empty())
			{
				tokens->clear();
				cgv::utils::split_to_tokens(*line_out, *tokens, separators, false);
				num_tokens = (unsigned)tokens->size();
			}
		}
		while (!contents.eof() && num_tokens < 1);
		return read_fields(*tokens, separators, fields_out);
	}
	inline static real parse_field (const std::string &field)
	{
		real val;
		try { val = (real)std::stod(field); }
		catch (const std::out_of_range&) { val = std::numeric_limits<real>::infinity(); }
		catch (...) { val = 0; }
		return val;
	}
	template <unsigned components>
	static cgv::math::fvec<real, components> parse_fields (
		const std::vector<std::string> &fields, const std::vector<unsigned> field_ids
	)
	{
		cgv::math::fvec<real, components> ret;
		// ToDo: investigate compile-time for-loop (explicit unrolling) techniques
		for (unsigned i=0; i<components; i++)
			ret[i] = parse_field(fields[field_ids[i]]);
		return std::move(ret);
	}
};

template <class flt_type>
csv_handler<flt_type>::csv_handler() : pimpl(nullptr)
{
	pimpl = new Impl;
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
	// - parse first row and check if there are enough columns
	const std::string &separators = impl.csv_desc.separators();
	const auto &props = impl.csv_desc.properties();
	std::vector<std::string> columns;
	std::vector<cgv::utils::token> tokens;
	if (  Impl::read_next_nonempty_line(&line, &tokens, separators, contents, &columns)
	    < props.max_col_id)
		return false;
	// - inspect contents more closely
	if (props.header)
	{
		if (impl.check_header(columns))
			// header matches up, so at this point we don't do any further tests and just assume it'll work
			return true;
		return false;
	}
	else
	{
		// also actually process up to two lines in the stream to see if we can actually interpret the text-encoded data
		if (impl.is_header(columns))
		{
			// even if we don't need a header - in case the file does have one, we will require it to match up
			if (!impl.check_header(columns))
				return false;
			// header checked out ok, read in first actual data row
			Impl::read_next_nonempty_line(&line, &tokens, separators, contents, &columns);
		}
		if (impl.special_fields_not_readable(columns))
			return false;
	}

	// apparently a valid .csv that we can interpret
	return true;
}

template <class flt_type>
bool csv_handler<flt_type>::read (std::istream &contents, unsigned idx_offset)
{
	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// stream processing database
	const std::string &separators = impl.csv_desc.separators();
	std::string line;
	std::vector<std::string> fields;
	std::vector<cgv::utils::token> tokens;

	// read in first row and do initial pre-processing
	Impl::read_next_nonempty_line(&line, &tokens, separators, contents, &fields);
	std::set<unsigned> undeclared_cols;
	for (unsigned i=0; i<fields.size(); i++)
		undeclared_cols.emplace_hint(undeclared_cols.end(), i);
	bool header_present;

	// route .csv file columns to declared attributes
	const auto &props = impl.csv_desc.properties();
	const auto &csv_attribs = impl.csv_desc.attributes();
	std::vector<Impl::declared_attrib> declared_attribs;
	declared_attribs.reserve(csv_attribs.size());
	if (props.header)
	{
		// we know that we have a header because our attribute declaration requires one
		header_present = true;

		// find actual .csv columns belonging to each declared attribute
		for (const auto &csv_attrib : csv_attribs)
		{
			declared_attribs.emplace_back(csv_attrib.columns.size());
			auto &attrib = declared_attribs.back();
			// for each colum declaration, search the corresponding field in the actual .csv header row
			for (const auto &col : csv_attribs[props.pos_id].columns)
				for (unsigned i=0; i<(unsigned)fields.size(); i++)
				{
					if (   (col.case_sensitive && fields[i].compare(col.name) == 0)
					    || (   !(col.case_sensitive)
					        && cgv::utils::to_lower(fields[i]).compare(cgv::utils::to_lower(col.name)) == 0))
					{
						attrib.field_ids.emplace_back(i);
						undeclared_cols.erase(i);
					}
				}
		}
	}
	else
	{
		// store whether or not the file has a header
		header_present = Impl::is_header(fields);

		// just commit the user-declared column numbers
		for (const auto &csv_attrib : csv_attribs)
		{
			declared_attribs.emplace_back(csv_attrib.columns.size());
			auto &attrib = declared_attribs.back();
			for (const auto &col : csv_attrib.columns)
			{
				attrib.field_ids.emplace_back(col.number);
				undeclared_cols.erase(col.number);
			}
		}
	}
	// route remaining .csv file columns to undeclared attributes
	std::vector<Impl::undeclared_attrib> undeclared_attribs;
	undeclared_attribs.reserve(undeclared_cols.size());
	for (unsigned i : undeclared_cols)
	{
		undeclared_attribs.emplace_back(1);
		auto &attrib = undeclared_attribs.back();
		if (header_present)
			attrib.name = fields[i];
		else
			attrib.name = "field_"+std::to_string(i);
		attrib.field_id = i;
	}
	undeclared_cols.clear(); // <-- we don't need these from here on

	// ensure we're at the first actual data row
	if (header_present)
		Impl::read_next_nonempty_line(&line, &tokens, separators, contents, &fields);

	// trajectory database
	double dist_accum = 0;
	std::vector<Vec3> &P = declared_attribs[props.pos_id].attrib.get_data<Vec3>();
	std::vector<rgb> C;
	std::map<int, std::vector<unsigned> > trajs;

	// determine if we're loading one of the IML tracking datasets for on-the-fly coordinate system correction and later color map selection
	// (ToDo: quick-and-dirty hack for demo purposes, needs proper generic transformation and transfer function infrastructure)
	const bool
		user =
			   csv_attribs.size()==3 && csv_attribs[0].name.compare("timestamp")==0 && csv_attribs[0].columns.size()==1
			&& csv_attribs[0].semantics==CSV::ARBITRARY && csv_attribs[1].name.compare("id")==0
			&& csv_attribs[1].semantics==CSV::TRAJ_ID && csv_attribs[2].semantics==CSV::POS && csv_attribs[2].columns[0].number==2,
		device =
			   csv_attribs.size()==3 && csv_attribs[0].name.compare("timestamp")==0 && csv_attribs[0].columns.size()==1
			&& csv_attribs[0].semantics==CSV::ARBITRARY && csv_attribs[1].name.compare("userid")==0
			&& csv_attribs[1].semantics==CSV::TRAJ_ID && csv_attribs[2].semantics==CSV::POS && csv_attribs[2].columns[0].number==4;
	impl.device = device;

	// parse the stream until EOF
	rgb sample_color(real(3)/real(4)); // <-- ToDo: REMOVE!!!
	while (!contents.eof())
	{
		// determine trajectory id of this row
		unsigned traj_id;
		if (props.multi_traj)
		{
			const int traj_id_field = declared_attribs[props.traj_id].field_ids.front();
			traj_id = std::atoi(fields[traj_id_field].c_str());
		}
		else
			traj_id = 0;

		// current sample index
		unsigned current_idx = (unsigned)P.size();

		// read in all declared attributes
		for (auto &attrib : declared_attribs)
		{
			switch (attrib.field_ids.size())
			{
				case 1:
					attrib.attrib.get_data<real>().emplace_back(
						Impl::parse_field(fields[attrib.field_ids.front()])
					);
					continue;

				case 2:
				{
					attrib.attrib.get_data<Vec2>().emplace_back(
						std::move(Impl::parse_fields<2>(fields, attrib.field_ids))
					);
					continue;
				}
				case 3:
				{
					auto &data = attrib.attrib.get_data<Vec3>();
					// special transformation for demo formats
					// ToDo: REMOVE
					if (user)
					{
						// ToDo: REMOVE
						data.emplace_back(
							real(std::stod(fields[attrib.field_ids.front()])*-10),
							real(std::stod(fields[attrib.field_ids[1]])*10),
							real(std::stod(fields[attrib.field_ids[2]])*-10)
						);
						sample_color = traj_id==1 ? rgb{ 178.f/255.f, 223.f/255.f, 138.f/255.f } : rgb{ 51.f/255.f, 160.f/255.f, 44.f/255.f };
					}
					else if (device)
					{
						// ToDo: REMOVE
						data.emplace_back(
							real(std::stod(fields[attrib.field_ids.front()])*10),
							real(std::stod(fields[attrib.field_ids[1]])*10),
							real(std::stod(fields[attrib.field_ids[2]])*10)
						);
						sample_color = traj_id==1 ? rgb{ 166.f/255.f, 206.f/255.f, 227.f/255.f } : rgb{ 31.f/255.f, 120.f/255.f, 180.f/255.f };
					}
					else
						data.emplace_back(std::move(Impl::parse_fields<3>(fields, attrib.field_ids)));
					continue;
				}
				case 4:
				{
					attrib.attrib.get_data<Vec4>().emplace_back(
						std::move(Impl::parse_fields<4>(fields, attrib.field_ids))
					);
					continue;
				}

				default:
					/* DoNothing() */;
			}
		}

		// read in all undeclared attributes
		for (auto &attrib : undeclared_attribs)
			attrib.attrib.get_data<real>().emplace_back(Impl::parse_field(fields[attrib.field_id]));

		// ToDo: move color generation out of the loading routine once transfer functions are supported
		C.emplace_back(sample_color);

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

		// read in next data row
		Impl::read_next_nonempty_line(&line, &tokens, separators, contents, &fields);
	}

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
		if (attrib.semantics == CSV::POS && pos_found)
			return false;
		else if (attrib.semantics == CSV::POS)
		{
			pos_found = true;
			if (attrib.columns.empty() || attrib.columns.size() > 3)
				return false;
		}
		else if (attrib.semantics == CSV::TRAJ_ID && traj_id_found)
			return false;
		else if (attrib.semantics == CSV::TRAJ_ID)
		{
			traj_id_found = true;
			if (attrib.columns.empty() || attrib.columns.size() > 1)
				return false;
		}
	}
	return pos_found;
}


////
// Explicit template instantiations

// Only float and double variants are intended
template class csv_handler<float>;
template class csv_handler<double>;


////
// Object registration

// Register example handler for the IML multi-user study .csv files (float and double versions)
static const csv_descriptor csv_imluser_desc(",", {
	{ "timestamp", {"timestamp", false, 0} },
	{ "id",        {"id", false, 1}, CSV::TRAJ_ID },
	{ "position",  {{"pos_1", false, 2}, {"pos_2", false, 3}, {"pos_3", false, 4}}, CSV::POS }
}),
csv_imldevice_desc(",", {
	{ "timestamp", {"timestamp", false, 0} },
	{ "userid",    {"userid", false, 1}, CSV::TRAJ_ID },
	{ "position",  {{"spacePos_1", false, 4}, {"spacePos_2", false, 5}, {"spacePos_3", false, 6}}, CSV::POS }
});
cgv::base::object_registration_1<csv_handler<float>, csv_descriptor> flt_csv_imldevice_reg(
	csv_imldevice_desc, "IML device trajectory csv handler (float)"
);
cgv::base::object_registration_1<csv_handler<double>, csv_descriptor> dbl_csv_imldevice_reg(
	csv_imldevice_desc, "IML device trajectory csv handler (double)"
);
cgv::base::object_registration_1<csv_handler<float>, csv_descriptor> flt_csv_imluser_reg(
	csv_imluser_desc, "IML user trajectory csv handler (float)"
);
cgv::base::object_registration_1<csv_handler<double>, csv_descriptor> dbl_csv_imluser_reg(
	csv_imluser_desc, "IML user trajectory csv handler (double)"
);
