
// C++ STL
#include <cmath>
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
// Class implementation

template <class flt_type>
struct csv_handler<flt_type>::Impl
{
	// types
	typedef flt_type real;
	typedef typename csv_handler::Vec3 Vec3;
	typedef typename csv_handler::Vec4 Vec4;
	typedef typename csv_handler::rgb rgb;

	// fields
	bool has_data=false, device=false;
	real avg_dist = 0;
};

template <class flt_type>
csv_handler<flt_type>::csv_handler() : pimpl(nullptr)
{
	pimpl = new Impl;
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
	auto& impl = *pimpl;

	// reset our private fields
	impl.has_data = false;
	impl.device = false;
	impl.avg_dist = 0;
}

template <class flt_type>
bool csv_handler<flt_type>::can_handle (std::istream &contents) const
{
	std::string str;
	auto g = contents.tellg();

	// check for tell-tale stream contents
	// - csv header row
	std::getline(contents, str);
	const std::string str_lower = cgv::utils::to_lower(str);
	if (   str_lower.compare(cgv::utils::to_lower("timestamp,id,pos_1,pos_2,pos_3,condition")) != 0
	    && str_lower.compare(cgv::utils::to_lower("timestamp,userid,screenPos_1,screenPos_2,spacePos_1,spacePos_2,spacePos_3,orientation_1,orientation_2,orientation_3,condition")) != 0)
	{
		contents.seekg(g);
		return false;
	}
	// - apparent valid file structure
	contents.seekg(g);
	return true;
}

template <class flt_type>
bool csv_handler<flt_type>::read (std::istream &contents, unsigned idx_offset)
{
	// trajectory database
	double dist_accum = 0;
	std::vector<Vec3> P;
	std::vector<rgb> C;
	std::map<unsigned, std::vector<unsigned> > trajs;

	// determine if device data
	std::string str;
	std::getline(contents, str);
	std::vector<cgv::utils::token> fields;
	cgv::utils::split_to_tokens(str, fields, "", false, "", "", ",");
	unsigned num_fields = (unsigned)fields.size()-1;
	bool device = cgv::utils::to_lower(str).compare(cgv::utils::to_lower(
		"timestamp,userid,screenPos_1,screenPos_2,spacePos_1,spacePos_2,spacePos_3,orientation_1,orientation_2,orientation_3,condition"
	)) == 0;

	// parse the stream until EOF
	bool running[2] = { false, false };
	do {
		// parsing
		std::getline(contents, str);
		fields.clear();
		cgv::utils::split_to_tokens(str, fields, "", false, "", "", ",");
		if (fields.size() < num_fields)
			continue;

		// processing
		unsigned current_idx = (unsigned)P.size(),
		         userid = std::atoi(fields[1].begin)-1;
		// - attributes
		if (device)
		{
			C.emplace_back(userid==0 ? rgb{ 166.f/255.f, 206.f/255.f, 227.f/255.f } : rgb{ 31.f/255.f, 120.f/255.f, 180.f/255.f });
			P.emplace_back(real(std::atof(fields[4].begin))*10, real(std::atof(fields[5].begin))*10, real(std::atof(fields[6].begin))*10);
		}
		else
		{
			C.emplace_back(userid==0 ? rgb{ 178.f/255.f, 223.f/255.f, 138.f/255.f } : rgb{ 51.f/255.f, 160.f/255.f, 44.f/255.f });
			P.emplace_back(real(std::atof(fields[2].begin))*-10, real(std::atof(fields[3].begin))*10, real(std::atof(fields[4].begin))*-10);
		}
		// - indices
		auto &indices = trajs[userid];
		indices.emplace_back(current_idx);
		if (running[userid])
		{
			dist_accum += (P.back() - P[*(indices.end()-2)]).length();
			// duplicate index to create new segment under line-list semantics
			indices.emplace_back(current_idx);
		}
		running[userid] = true;
	}
	while (!contents.eof());

	// did we succeed at loading anything?
	if (trajs.size() < 1)
		return false;

	// shortcut for saving one indirection
	auto &impl = *pimpl;

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
	real radius = impl.avg_dist * (device ? real(0.25) : real(0.125));
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


////
// Explicit template instantiations

// Only float and double variants are intended
template class csv_handler<float>;
template class csv_handler<double>;


////
// Object registration

// The trajectory format handler registry
cgv::base::object_registration<csv_handler<float> > flt_csv_reg("csv trajectory handler (float)");
cgv::base::object_registration<csv_handler<double> > dbl_csv_reg("csv trajectory handler (double)");
