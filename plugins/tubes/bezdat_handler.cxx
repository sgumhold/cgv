
// C++ STL
#include <cmath>
#include <vector>
#include <unordered_map>
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
#include "bezdat_handler.h"


/////
// Some constant defines

/// multiple of the user-supplied distance unit for use as a tolerance to decide whether two node position values are similar or not
#define BEZDAT_SIM_TOLERANCE_POS (real(32)*std::numeric_limits<real>::epsilon())

/// multiple of the user-supplied distance unit for use as a tolerance to decide whether two node tangents are similar or not
#define BEZDAT_SIM_TOLERANCE_DPOS real(0.125)

/// multiple of the user-supplied distance unit for use as a tolerance to decide whether two node radius values are similar or not
#define BEZDAT_SIM_TOLERANCE_RAD (real(128)*std::numeric_limits<real>::epsilon())

/// multiple of the user-supplied distance unit for use as a tolerance to decide whether two node radius derivatives are similar or not
#define BEZDAT_SIM_TOLERANCE_DRAD real(0.125)

/// multiple of the user-supplied distance unit for use as a tolerance to decide whether two node color values are similar or not
#define BEZDAT_SIM_TOLERANCE_COL (real(1024)*std::numeric_limits<real>::epsilon())

/// multiple of the user-supplied color distance unit for use as a tolerance to decide whether two node color derivatives are similar or not
#define BEZDAT_SIM_TOLERANCE_DCOL real(0.125)

/// identifyier to use for tangent data
#define BEZDAT_TANGENT_ATTRIB_NAME "tangent"

/// identifyier to use for radius data
#define BEZDAT_RADIUS_ATTRIB_NAME "radius"

/// identifyier to use for color data
#define BEZDAT_COLOR_ATTRIB_NAME "color"

/// identifyier to use for color data
#define BEZDAT_DCOLOR_ATTRIB_NAME "dcolor"


////
// Local types and variables

// anonymous namespace begin
namespace {

/// generate a static, always applicable default attribute mapping suggestion for .bezdat files
const struct attrib_suggestion_struct
{
	const visual_attribute_mapping map;

	attrib_suggestion_struct()
		: map{{VisualAttrib::TANGENT, BEZDAT_TANGENT_ATTRIB_NAME}, {VisualAttrib::RADIUS, BEZDAT_RADIUS_ATTRIB_NAME},
		      {VisualAttrib::COLOR, BEZDAT_COLOR_ATTRIB_NAME}}
	{}
} attrib_suggestion;

/// one point in a bezdat dataset
template <class flt_type>
struct bezdat_point
{
	typedef flt_type real;
	typedef typename bezdat_handler<real>::Vec3 Vec3;
	Vec3 pos, color;
	real radius;
};

/// one segment in a bezdat dataset
struct bezdat_segment
{
	unsigned idx[4];
};

/// one Hermite position node directly converted from a bezdat dataset
template <class flt_type>
struct bezdat_node
{
	// Types
	typedef flt_type real;
	typedef typename bezdat_point<real>::Vec3 Vec3;

	// Data members
	Vec3 pos, dpos, col, dcol;
	real rad, drad;

	// Methods
	bezdat_node() {};
	bezdat_node(unsigned i_pnt, unsigned i_aux, const bezdat_segment &seg,
	            const std::vector<bezdat_point<real> > &points)
	{
		pos = points[seg.idx[i_pnt]].pos;
		col = points[seg.idx[i_pnt]].color;
		rad = points[seg.idx[i_pnt]].radius;
		if (i_aux & 0x1)
		{
			dpos = (points[seg.idx[i_aux]].pos-points[seg.idx[i_pnt]].pos) * real(3);
			dcol =   (points[seg.idx[i_aux]].color-points[seg.idx[i_pnt]].color) * real(3);
			drad = (points[seg.idx[i_aux]].radius-points[seg.idx[i_pnt]].radius) * real(3);
		}
		else
		{
			dpos = (points[seg.idx[i_pnt]].pos-points[seg.idx[i_aux]].pos) * real(3);
			dcol =   (points[seg.idx[i_pnt]].color-points[seg.idx[i_aux]].color) * real(3);
			drad = (points[seg.idx[i_pnt]].radius-points[seg.idx[i_aux]].radius) * real(3);
		}
	}

	bool is_similar (const bezdat_node<real> &other, real pUnit=1, real rUnit=1, real cUnit=1) const
	{
		// ToDo: actually parametrize value and angle tolerances
		const real
			diff_pos=(pos - other.pos).sqr_length(),
			thr_pos=BEZDAT_SIM_TOLERANCE_POS*BEZDAT_SIM_TOLERANCE_POS * pUnit*pUnit,
			diff_dpos=(dpos - other.dpos).sqr_length(),
			thr_dpos=BEZDAT_SIM_TOLERANCE_DPOS*BEZDAT_SIM_TOLERANCE_DPOS * pUnit*pUnit,
			diff_rad=(rad - other.rad),
			thr_rad=BEZDAT_SIM_TOLERANCE_RAD * rUnit,
			diff_drad=std::abs(drad - other.drad),
			thr_drad=BEZDAT_SIM_TOLERANCE_DRAD * rUnit,
			diff_col=(col - other.col).sqr_length(),
			thr_col=BEZDAT_SIM_TOLERANCE_COL*BEZDAT_SIM_TOLERANCE_COL * cUnit*cUnit,
			diff_dcol=(dcol - other.dcol).sqr_length(),
			thr_dcol=BEZDAT_SIM_TOLERANCE_DCOL*BEZDAT_SIM_TOLERANCE_DPOS * cUnit*cUnit;

		return   (diff_pos <= thr_pos) && (diff_rad <= thr_rad) && (diff_col <= thr_col)
		      && (diff_dpos<=thr_dpos) && (diff_drad<=thr_drad) && (diff_dcol<=thr_dcol);
	}
};

/// one Hermite segment
union hermite_segment
{
	struct {
		/// index of the first node of the segment
		unsigned n0;

		/// index of the second node of the segment
		unsigned n1;
	};

	/// indices of the first and second node of the segment
	unsigned n[2];
};

// Anonymous namespace end
}


////
// Class implementation

template <class flt_type>
struct bezdat_handler<flt_type>::Impl
{
	// types
	typedef flt_type real;
	typedef typename bezdat_handler::Vec3 Vec3;
	typedef typename bezdat_handler::Vec4 Vec4;
	typedef typename bezdat_handler::rgb rgb;

	// fields
	unsigned curve_count = 0;
	real avg_dist = 0;
};

template <class flt_type>
bezdat_handler<flt_type>::bezdat_handler() : pimpl(nullptr)
{
	pimpl = new Impl;
}

template <class flt_type>
bezdat_handler<flt_type>::~bezdat_handler()
{
	if (pimpl)
		delete pimpl;
}

template <class flt_type>
void bezdat_handler<flt_type>::cleanup (void)
{
	// shortcut for saving one indirection
	auto& impl = *pimpl;

	// reset our private fields
	impl.curve_count = 0;
	impl.avg_dist = 0;
}

template <class flt_type>
bool bezdat_handler<flt_type>::can_handle (std::istream &contents) const
{
	std::string str;
	stream_pos_guard g(contents);

	// check for tell-tale stream contents
	// - .bezdat header
	std::getline(contents, str);
	if (cgv::utils::to_lower(str).compare("bezdata 1.0") != 0)
	{
		//std::cout << "bezdat_handler: first line in stream must be \"BezDatA 1.0\", but found \"" << str << "\" instead!" << std::endl;
		return false;
	}
	// - check if rest of the file looks valid on first glance
	do { std::getline(contents, str); } while (!contents.eof() && str.empty());
	str = std::move(cgv::utils::to_upper(str.substr(0, 2)));
	return str.compare("PT")==0 || str.compare("BC")==0;
}

template <class flt_type>
bool bezdat_handler<flt_type>::read (std::istream &contents, unsigned idx_offset)
{
	// bezdat database
	std::vector<bezdat_point<real> > points;
	std::vector<bezdat_segment> bsegs;
	real avgNodeDist=0, avgRadiusDiff=0, avgColorDiff=0;

	// parse the stream until EOF
	std::string str;
	contents >> str;
	while (!contents.eof())
	{
		if (cgv::utils::to_upper(str).compare("PT") == 0)
		{
			// read the control point
			points.emplace_back(); bezdat_point<real> &new_point = points.back();
			contents >> new_point.pos;
			contents >> new_point.radius;
			contents >> new_point.color;
		}
		else if (cgv::utils::to_upper(str).compare("BC") == 0)
		{
			// read the segment
			bsegs.emplace_back(); bezdat_segment &new_segment = bsegs.back();
			contents >> new_segment.idx[0] >> new_segment.idx[1] >> new_segment.idx[2] >> new_segment.idx[3];
			// update estimate of average per-attribute node distances
			real curNodeDist = (  points[new_segment.idx[0]].pos
			                    - points[new_segment.idx[3]].pos).length(),
			     curRadiusDiff = std::abs(  points[new_segment.idx[3]].radius
			                              - points[new_segment.idx[0]].radius),
			     curColorDiff = (  points[new_segment.idx[0]].color
			                     - points[new_segment.idx[3]].color).length();
			const size_t num = bsegs.size();
			avgNodeDist =
				(real(num-1)*avgNodeDist + curNodeDist) / real(num);
			avgRadiusDiff =
				(real(num-1)*avgRadiusDiff + curRadiusDiff) / real(num);
			avgColorDiff =
				(real(num-1)*avgColorDiff + curColorDiff) / real(num);
		}
		contents >> str;
	}

	// build database of Hermite node data
	std::vector<bezdat_node<real> > nodes;
	std::vector<hermite_segment> segments;
	nodes.reserve(points.size());
	segments.reserve(bsegs.size());
	grid3D<real> nodes_db(
		avgNodeDist*real(2)*BEZDAT_SIM_TOLERANCE_POS,
		[&nodes] (Vec3 *pnt, size_t id) -> bool {
			if (id < nodes.size())
			{
				*pnt = nodes[id].pos;
				return true;
			}
			else
				return false;
		}
	);

	// merge similar nodes
	unsigned curve_count = 0;
	std::vector<std::vector<bezdat_node<real> > > merge_log;
	merge_log.resize(points.size());
	for (const auto &bseg : bsegs)
	{
		// add a new Hermite segment
		segments.emplace_back();
		auto &new_seg = segments.back();

		// unique-ify this control point combination
		for (unsigned i=0; i<2; i++)
		{
			// determine which is outer and which is inner control point
			const unsigned i_pnt = i*2+i, i_aux = i*2+1-i;
			const bezdat_point<real> &pnt = points[bseg.idx[i_pnt]];

			// build list of nodes close to the one we're about to construct
			std::vector<size_t> neighbors;
			nodes_db.query(&neighbors, pnt.pos, true);

			// check if we can reuse a node
			bool not_found = true;
			bezdat_node<real> new_node(i_pnt, i_aux, bseg, points);
			for (size_t nb_id : neighbors)
			{
				if (new_node.is_similar(nodes[nb_id], avgNodeDist, avgRadiusDiff, avgColorDiff))
				{
					// node already exists, log the duplicate and reference it
					merge_log[nb_id].emplace_back(std::move(new_node));
					new_seg.n[i] = (unsigned)nb_id;
					not_found = false;
					break;
				}
			}
			if (not_found)
			{
				// this node is not yet in the database
				// - determine index of new node
				unsigned id = (unsigned)nodes.size();
				// - store the new node
				nodes.emplace_back(new_node);
				// - reference the new node
				nodes_db.insert(id);
				new_seg.n[i] = id;
				// - check if this looks like a new curve (indicated by discontinuity at
				//   first node of the segment)
				if (i==0)
					curve_count++;
			}
		}
	}

	// move merged nodes to position of cluster average
	for (unsigned i=0; i<nodes.size(); i++) if (!(merge_log[i].empty()))
	{
		auto &node = nodes[i];
		for (const auto &mn : merge_log[i])
		{
			node.pos += mn.pos;
			node.dpos += mn.dpos;
			node.rad += mn.rad;
			node.drad += mn.drad;
			node.col += mn.col;
			node.dcol += mn.dcol;
		}
		real num = real(merge_log[i].size()+1);
		node.pos /= num; node.dpos /= num;
		node.rad /= num; node.drad /= num;
		node.col /= num; node.dcol /= num;
	}

	// did we succeed at loading anything?
	if (curve_count < 1)
		return false;

	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// commit attributes to common curve representation
	const size_t num = nodes.size();
	auto &A = attributes();
	auto &P = positions(); P.reserve(num);
	auto &I = indices(); I.reserve(segments.size()*2);
	std::vector<Vec4> dP; dP.reserve(num);
	std::vector<real> R; R.reserve(num);
	std::vector<rgb> C, dC; C.reserve(num); dC.reserve(num);
	for (auto &node : nodes)
	{
		P.emplace_back(node.pos);
		dP.emplace_back(vec4_from_vec3s(node.dpos, node.drad));
		R.emplace_back(node.rad);
		C.emplace_back(vec3_to_rgb(node.col));
		dC.emplace_back(vec3_to_rgb(node.dcol));
	}
	for (auto& seg : segments)
	{
		I.push_back(seg.n0 + idx_offset);
		I.push_back(seg.n1 + idx_offset);
		impl.avg_dist += (P[seg.n1] - P[seg.n0]).length();
	}
	impl.avg_dist /= (real)segments.size();
	impl.curve_count = curve_count;

	// commit to actual attribute storage
	A.emplace(BEZDAT_TANGENT_ATTRIB_NAME, std::move(dP));
	A.emplace(BEZDAT_RADIUS_ATTRIB_NAME, std::move(R));
	A.emplace(BEZDAT_COLOR_ATTRIB_NAME, std::move(C));
	A.emplace(BEZDAT_DCOLOR_ATTRIB_NAME, std::move(dC));

	// print some stats
	std::cout << "bezdat_handler: loading completed! Stats:" << std::endl
	          << "  " << points.size()<<" .bezdat control points" << std::endl
	          << "  " << bsegs.size()<<" .bezdat curve segments" << std::endl
	          << " --- converted to: ---" << std::endl
	          << "  " << nodes.size()<<" Hermite nodes" << std::endl
	          << "  " << segments.size()<<" Hermite segments" << std::endl
	          << "  " << impl.curve_count<<" distinct smooth "<<(impl.curve_count>1?"intervals":"interval")
	          << std::endl << std::endl;

	// done
	return true;
}

template <class flt_type>
bool bezdat_handler<flt_type>::has_data (void) const
{
	return pimpl->curve_count > 0;
}

template <class flt_type>
flt_type bezdat_handler<flt_type>::avg_segment_length (void) const
{
	return pimpl->avg_dist;
}

template <class flt_type>
const visual_attribute_mapping& bezdat_handler<flt_type>::suggest_mapping (void) const
{
	return attrib_suggestion.map;
}


////
// Explicit template instantiations

// Only float and double variants are intended
template class bezdat_handler<float>;
template class bezdat_handler<double>;


////
// Object registration

// Register both float and double handlers
cgv::base::object_registration<bezdat_handler<float> > flt_bezdat_reg("bezdat trajectory handler (float)");
cgv::base::object_registration<bezdat_handler<double> > dbl_bezdat_reg("bezdat trajectory handler (double)");
