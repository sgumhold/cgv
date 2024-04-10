#pragma once

#include <cgv/data/optional.h>

#include "renderer.h"

// Define macros to easily define recurring methods in derived classes

/// @brief Perform a transfer of the stored data to the attribute_array right now.
///
/// Only executed if state_out_of_date is true. Normally the transfer operation is
/// performed when enable is called. However, sometimes it is necessary for the data
/// to be stored in the attribute_array and hence in the GPU buffer before rendering,
/// in order to be able to manipulate the data through, e.g., compute shaders.
/// 
/// @param ctx The GL context.
/// @param r The used renderer class instance.
#define RDB_EARLY_TRANSFER_FUNC_DEF(RENDERER) \
void early_transfer(context& ctx, RENDERER& r) { \
	r.enable_attribute_array_manager(ctx, this->attribute_array); \
	if(this->state_out_of_date) transfer(ctx, r); \
	r.disable_attribute_array_manager(ctx, this->attribute_array); \
}

/// @brief Initialize the render data and increase the reference count of the
/// specific renderer. See render_data_base::init.
/// @param ctx The GL context.
#define RDB_INIT_FUNC_DEF(RENDERER) \
bool init(context& ctx) { \
	ref_##RENDERER(ctx, 1); \
	return super::init(ctx); \
}

/// @brief Destruct the render data and decrease the reference count of the
/// specific renderer. See render_data_base::destruct.
/// @param ctx The GL context.
#define RDB_DESTRUCT_FUNC_DEF(RENDERER) \
void destruct(context& ctx) { \
	ref_##RENDERER(ctx, -1); \
	super::destruct(ctx); \
}

/// @brief Enable the render data for rendering.
///
/// Prepares the renderer to render using the attribute_array and given style.
/// If state_out_of_date is true, the attribute_array is updated to reflect
/// the stored data. Overrides the per-default used renderer and style.
/// 
/// @param ctx The GL context.
/// @param r The used renderer class instance.
/// @param style The used render style.
#define RDB_ENABLE_FUNC_DEF(RENDERER, STYLE) \
bool enable(context& ctx, RENDERER& r, const STYLE& s) { \
	if(this->size() > 0) { \
		r.set_render_style(s); \
		r.enable_attribute_array_manager(ctx, this->attribute_array); \
		if(this->state_out_of_date) transfer(ctx, r); \
		this->set_const_attributes(ctx, r); \
		return r.validate_and_enable(ctx); \
	} else if(this->state_out_of_date) { \
		early_transfer(ctx, r); \
	} \
	return false; \
}

/// @brief Render the stored geometry.
///
/// If offset and count are not specified the full data range is rendered.
/// Validates whether offset and count produce a valid range according to
/// the value returned by render_data_base::render_count and clamps the count
/// values if necessary.
/// 
/// @param ctx The GL context.
/// @param offset The vertex offset.
/// @param count The vertex count.
#define RDB_RENDER_FUNC3_DEF(RENDERER) \
void render(context& ctx, unsigned offset = 0, int count = -1) { \
	render(ctx, ref_##RENDERER(ctx), style, offset, count); \
}

/// @brief Render the stored geometry using the given style.
///
/// If offset and count are not specified the full data range is rendered.
/// Validates whether offset and count produce a valid range according to
/// the value returned by render_data_base::render_count and clamps the count
/// values if necessary.
/// 
/// @param ctx The GL context.
/// @param style The used render style.
/// @param offset The vertex offset.
/// @param count The vertex count.
#define RDB_RENDER_FUNC2_DEF(RENDERER, STYLE) \
void render(context& ctx, const STYLE& s, unsigned offset = 0, int count = -1) { \
	render(ctx, ref_##RENDERER(ctx), s, offset, count); \
}

/// @brief Render the stored geometry using the given renderer.
///
/// If offset and count are not specified the full data range is rendered.
/// Validates whether offset and count produce a valid range according to
/// the value returned by render_data_base::render_count and clamps the count
/// values if necessary.
/// 
/// @param ctx The GL context.
/// @param r The used renderer class instance.
/// @param offset The vertex offset.
/// @param count The vertex count.
#define RDB_RENDER_FUNC1_DEF(RENDERER) \
void render(context& ctx, RENDERER& r, unsigned offset = 0, int count = -1) { \
	render(ctx, r, style, offset, count); \
}

/// @brief Render the stored geometry using the given renderer and style.
///
/// If offset and count are not specified the full data range is rendered.
/// Validates whether offset and count produce a valid range according to
/// the value returned by render_data_base::render_count and clamps the count
/// values if necessary.
/// 
/// @param ctx The GL context.
/// @param r The used renderer class instance.
/// @param style The used render style.
/// @param offset The vertex offset.
/// @param count The vertex count.
#define RDB_RENDER_FUNC0_DEF(RENDERER, STYLE) \
void render(context& ctx, RENDERER& r, const STYLE& s, unsigned offset = 0, int count = -1) { \
	if(enable(ctx, r, s)) { \
		this->draw(ctx, r, offset, count); \
		this->disable(ctx, r); \
	} \
}

/// Define the members and template-like methods specific to each render style and renderer
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

/// @brief A base class for storing render data and style usable with the default
///		   renderers provided in the cgv::render namespace.
/// 
/// This class and its derivatives provide storage for typical vertex attributes
/// used by the different renderers to enable simple handling of geometry data and
/// rendering. Additionally, a customizable default style is provided. Per default,
/// a reference to the singleton of the target renderer is used. The reference count
/// is automatically increased and decreased by the init and destruct methods. Both
/// the default style and renderer can be exchanged for user-provided ones.
/// 
/// Each derived class may add storage for vertex attributes according to the
/// capabilities of its target renderer.
/// 
/// Data handling:
/// Host side storage is provided through vector members and GPU side storage is
/// managed through an attribte_array_manager. The data will only be uploaded to
/// the attribute_array_manager when either enable or render is called and the
/// state_out_of_date flag is set to true.
/// 
/// Vertex attributes are public and can be freely manipulated. It is up to the
/// user to ensure the validity of the data. Additionally, the following methods
/// are provided:
/// 
/// add_<attribute_name>(const <attribute_type>& value)
/// --> Add a single value of the specified type to the attribute with the given
/// name. Example: add_color(rgb(0.5f));
/// 
/// add(const <attribute_type_1>& value_1, const <attribute_type_2>& value_2)
/// --> Add a single value of each given attribute of the specified type to the
/// attribute with the given name. These methods provide syntactic sugar for quickly
/// adding common attribute configurations. Example: add(position, color);
/// 
/// fill_<attribute_name>(const <attribute_type>& value)
/// --> Fill the attribute of the given name with the given value until it has the
/// same amount of entries as the positions array. Example: fill_colors(rgb(0.5f));
/// 
/// Derived classes may add more methods to reflect the capabilities of their target
/// renderer and supported geometry properties:
/// 
/// add_segment_<attribute_name>(const <attribute_type>& value)
/// --> Add two copies of the given value to the attribute of the specified name.
/// This is used by, e.g., line-type geometries, to quickly add a single attribute value
/// for start and end points. Example: add_segment_color(color_for_whole_segment);
/// 
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class render_data_base {
public:
	/// stores an array of indices
	std::vector<uint32_t> indices;
	/// stores an array of positions
	std::vector<vec3> positions;
	/// stores an array of colors
	std::vector<ColorType> colors;
	/// stores an optional constant color used for all elements
	cgv::data::optional<ColorType> const_color;

protected:
	/// whether the state of this render datas members are out of date with the attribute_array
	bool state_out_of_date = true;
	/// the attribute array manager storing the data in GL buffers
	attribute_array_manager attribute_array;

	/// @brief Transfers the data stored in members to the attribute array.
	/// @param ctx The GL context.
	/// @param r The used renderer instance.
	/// @return True is successful, false otherwise.
	virtual bool transfer(context& ctx, renderer& r) {
		state_out_of_date = false;

		// The positions array determines the amount of stored vertices.
		// All other arrays except indices must match its size.
		if(!positions.empty()) {
			r.set_position_array(ctx, positions);
			if(colors.size() == size())
				r.set_color_array(ctx, colors);
			if(!indices.empty())
				r.set_indices(ctx, indices);
			else
				r.remove_indices(ctx);
			return true;
		} else {
			// If no positions are set the render data is assumed to be empty. In this case the
			// attribute array manager is destructed and reinitialized to clear the GPU buffers.
			if(attribute_array.is_created()) {
				attribute_array.destruct(ctx);
				attribute_array.init(ctx);
			}
			// Return false since the attribute array manager holds no data anymore.
			return false;
		}
	}

	/// @brief Set constant vertex attributes if present.
	/// 
	/// Sets the supported vertex attributes to constant values if they are specified and if
	/// the respective arrays are empty. This enables to set, e. g., a single color for all
	/// elements without the need to fill the whole color array with the same value.
	/// 
	/// @param ctx The GL context.
	/// @param r The used renderer instance.
	virtual void set_const_attributes(context& ctx, renderer& r) {
		if(colors.empty() && const_color)
			r.set_color(ctx, const_color.value());
	}

	/// @brief Template for filling a member array to the size of the render data.
	/// 
	/// Fills the member array with copies of value up to the size of the positions array of
	/// this render data.
	/// 
	/// @tparam T The data type.
	/// @param vector The member array.
	/// @param value The constant value used for padding.
	template<typename T>
	void fill(std::vector<T>& vector, const T& value) {
		if(vector.size() < size())
			vector.resize(size(), value);
	}

public:
	/// @brief Initialize the attribute array manager.
	/// @param ctx The GL context.
	virtual bool init(context& ctx) {
		return attribute_array.init(ctx);
	}

	/// @brief Destruct the attribute array manager.
	/// @param ctx The GL context.
	virtual void destruct(context& ctx) {
		attribute_array.destruct(ctx);
	}

	/// Clear the stored data and set state out of date.
	void clear() {
		indices.clear();
		positions.clear();
		colors.clear();
		
		state_out_of_date = true;
	}

	/// @brief Notify the render data about state changes.
	///
	/// Calling this method will result in the state_out_of_date flag being set to true
	/// and a call to transfer upon the next call to render or enable. This method shall
	/// never be called implicitly as a side effect of another member method, except clear,
	/// and must always be called explicitly after changing the stored data.
	void set_out_of_date() {
		state_out_of_date = true;
	}

	/// @brief Return the number of stored positions.
	/// @return The number of stored positions.
	size_t size() {
		return positions.size();
	}

	/// @brief Return whether this render data is empty.
	/// 
	/// A render data is considered to be empty when the positions array is empty.
	/// 
	/// @return True if empty, false otherwise.
	bool empty() const {
		return positions.empty();
	}

	/// @brief Return the number of vertices that will be rendered.
	/// 
	/// Returns the number of indices if indices is not empty and
	/// the number of positions otherwise.
	/// 
	/// @return The number of specified vertices.
	size_t render_count() {
		if(indices.empty())
			return positions.size();
		else
			return indices.size();
	}

	/// @brief Constant access to the private attribute_array_manager.
	/// 
	/// Can be used to retrieve internals such as buffer handles.
	/// 
	/// @return A const reference to the attribute_array_manager.
	const attribute_array_manager& ref_attribute_array_manager() const {
		return attribute_array;
	}

	void add_index(const uint32_t index) {
		indices.push_back(index);
	}

	void add_position(const vec3& position) {
		positions.push_back(position);
	}

	void add_color(const ColorType& color) {
		colors.push_back(color);
	}

	void add(const vec3& position, const ColorType& color) {
		add_position(position);
		add_color(color);
	}

	void fill_colors(const ColorType& color) {
		fill(colors, color);
	}

	/// @brief Disable the renderer and attribute_array.
	/// @param ctx The GL context.
	/// @param r The used renderer class instance.
	/// @return True if disabling was successful.
	bool disable(context& ctx, renderer& r) {
		bool res = r.disable(ctx);
		r.disable_attribute_array_manager(ctx, attribute_array);
		return res;
	}

	/// @brief Draw the stored geometry using the given renderer.
	/// 
	/// This method must only be called after a successful call to enable.
	/// Afterwards, disable must be called.
	/// If offset and count are not specified the full data range is rendered.
	/// Validates whether offset and count produce a valid range according to
	/// the value returned by render_data_base::render_count and clamps the count
	/// values if necessary.
	/// 
	/// @param ctx The GL context.
	/// @param r The used renderer class instance.
	/// @param offset The vertex offset.
	/// @param count The vertex count.
	void draw(context& ctx, renderer& r, unsigned offset = 0, int count = -1) {		
		size_t draw_count = render_count();
		offset = std::min(offset, static_cast<unsigned>(draw_count));
		draw_count = std::min(offset + (count < 0 ? draw_count : count), draw_count) - offset;

		r.draw(ctx, offset, draw_count);
	}
};

}
}
