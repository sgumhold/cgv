#pragma once

#include "renderer.h"

#include "gl/lib_begin.h"

// define some macros to easily define recurring methods in derived classes
#define RDB_EARLY_TRANSFER_FUNC_DEF(RENDERER) \
void early_transfer(context& ctx, RENDERER& r) { \
	r.enable_attribute_array_manager(ctx, this->aam); \
	if(this->out_of_date) transfer(ctx, r); \
	r.disable_attribute_array_manager(ctx, this->aam); \
}

#define RDB_INIT_FUNC_DEF(RENDERER) \
bool init(context& ctx) { \
	ref_##RENDERER(ctx, 1); \
	return super::init(ctx); \
}

#define RDB_DESTRUCT_FUNC_DEF(RENDERER) \
void destruct(context& ctx) { \
	ref_##RENDERER(ctx, -1); \
	super::destruct(ctx); \
}

#define RDB_ENABLE_FUNC_DEF(RENDERER, STYLE) \
bool enable(context& ctx, RENDERER& r, const STYLE& s) { \
	if(this->size() > 0) { \
		r.set_render_style(s); \
		r.enable_attribute_array_manager(ctx, this->aam); \
		if(this->out_of_date) transfer(ctx, r); \
		return r.validate_and_enable(ctx); \
	} \
	return false; \
}

#define RDB_RENDER_FUNC3_DEF(RENDERER) \
void render(context& ctx, unsigned offset = 0, int count = -1) { \
	render(ctx, ref_##RENDERER(ctx), style, offset, count); \
}

#define RDB_RENDER_FUNC2_DEF(RENDERER, STYLE) \
void render(context& ctx, const STYLE& s, unsigned offset = 0, int count = -1) { \
	render(ctx, ref_##RENDERER(ctx), s, offset, count); \
}

#define RDB_RENDER_FUNC1_DEF(RENDERER) \
void render(context& ctx, RENDERER& r, unsigned offset = 0, int count = -1) { \
	render(ctx, r, style, offset, count); \
}

#define RDB_RENDER_FUNC0_DEF(RENDERER, STYLE) \
void render(context& ctx, RENDERER& r, const STYLE& s, unsigned offset = 0, int count = -1) { \
	if(enable(ctx, r, s)) { \
		this->draw(ctx, r, offset, count); \
		this->disable(ctx, r); \
	} \
}

#define RDB_BASE_FUNC_DEF(RENDERER, STYLE) \
	STYLE style; \
	RDB_EARLY_TRANSFER_FUNC_DEF(RENDERER) \
	RDB_INIT_FUNC_DEF(RENDERER) \
	RDB_DESTRUCT_FUNC_DEF(RENDERER) \
	RDB_ENABLE_FUNC_DEF(RENDERER, STYLE) \
	RDB_RENDER_FUNC3_DEF(RENDERER) \
	RDB_RENDER_FUNC2_DEF(RENDERER, STYLE) \
	RDB_RENDER_FUNC1_DEF(RENDERER) \
	RDB_RENDER_FUNC0_DEF(RENDERER, STYLE) \

namespace cgv {
namespace render {

template <typename ColorType = render_types::rgb>
class render_data_base : public render_types {
protected:
	attribute_array_manager aam;
	bool out_of_date;

	std::vector<unsigned> idx;
	std::vector<vec3> pos;
	std::vector<ColorType> col;

	virtual bool transfer(context& ctx, renderer& r) {
		if(pos.size() > 0) {
			r.set_position_array(ctx, pos);
			if(col.size() == size())
				r.set_color_array(ctx, col);
			if(idx.size() > 0)
				r.set_indices(ctx, idx);
			else
				r.remove_indices(ctx);
			out_of_date = false;
			return true;
		}
		return false;
	}

public:
	render_data_base() {
		out_of_date = true;
	}

	~render_data_base() { clear(); }

	virtual bool init(context& ctx) {
		return aam.init(ctx);
	}

	virtual void destruct(context& ctx) {
		clear();
		aam.destruct(ctx);
	}

	void clear() {
		idx.clear();
		pos.clear();
		col.clear();
		
		out_of_date = true;
	}

	void set_out_of_date() { out_of_date = true; }

	size_t size() { return pos.size(); }
	size_t render_count() {
		if(idx.empty())
			return pos.size();
		else
			return idx.size();
	}

	bool disable(context& ctx, renderer& r) {
		bool res = r.disable(ctx);
		r.disable_attribute_array_manager(ctx, aam);
		return res;
	}

	void draw(context& ctx, renderer& r, unsigned offset = 0, int count = -1) {
		r.draw(ctx, offset, count < 0 ? render_count() : count);
	}

	const attribute_array_manager& ref_aam() const { return aam; }

	std::vector<unsigned>&	ref_idx() { return idx; }
	std::vector<vec3>&		ref_pos() { return pos; }
	std::vector<ColorType>&	ref_col() { return col; }

	void add_idx(const unsigned int i) {
		idx.push_back(i);
	}
};

}
}

#include <cgv/config/lib_end.h>
