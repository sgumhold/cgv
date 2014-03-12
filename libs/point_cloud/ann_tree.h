#pragma once

#include <vector>
#include "point_cloud.h"

#include "lib_begin.h"

struct CGV_API ann_tree : public point_cloud_types
{
	void* ann_impl;
	const point_cloud* pc;
	Cnt k;
public:
	ann_tree();
	~ann_tree();
	void clear();
	bool is_empty() const;
	void build(const point_cloud& pc);
	void extract_neighbors(Idx i, Idx k, std::vector<Idx>& N) const;
	Idx find_closest(const Pnt& p) const;
};

#include <cgv/config/lib_end.h>