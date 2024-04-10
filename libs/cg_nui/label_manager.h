#pragma once

#include <memory>

#include <cgv/media/font/font.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cgv/render/texture.h>
#include <cgv/render/frame_buffer.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		/// data stored in the label_manager per label
		struct label
		{
		protected:
			/// extent in texels
			int width, height;
			friend class label_manager;
		public:
			/// label text
			std::string text;
			/// label background color
			rgba background_color;
			/// border values are also in texels
			int border_x, border_y;
			/// use this to query width
			int get_width() const { return std::abs(width); }
			/// use this to query height
			int get_height() const { return std::abs(height); }
		};

		enum LabelState
		{
			LS_CURRENT = 0,
			LS_NEW_TEXT = 1,
			LS_NEW_SIZE = 2,
			LS_NEW_COLOR = 4
		};

		/// use label manager to organize an atlas texture for drawing text labeled rectangles
		class CGV_API label_manager
		{
		public:
			typedef cgv::media::axis_aligned_box<int32_t, 2> ibox2;
		protected:
			/// labels with extent, text, border size and colors
			std::vector<label> labels;
			/// packing information for labels with texture coordinate ranges
			std::vector<ibox2> tex_ranges;
			/// extent of atlas texture
			int tex_width, tex_height;
			/// atlas texture
			std::shared_ptr<cgv::render::texture> tex;
			/// fbo used to render atlas texture
			cgv::render::frame_buffer fbo;
			/// this is a temporary rotated texture into which the rotated labels are drawn
			cgv::render::texture tmp_tex;
			/// fbo used to render temp texture
			cgv::render::frame_buffer tmp_fbo;

			std::vector<uint8_t> label_states;
			bool packing_outofdate;
			bool texture_outofdate;
			bool texture_content_outofdate;
			std::vector<uint32_t> not_rotated_labels;
			std::vector<uint32_t> rotated_labels;

			float font_size;
			cgv::media::font::font_face_ptr font_face;
			int safety_extension;
			rgba text_color;
			rgba background_color;

			cgv::render::rectangle_render_style rrs;
			cgv::render::attribute_array_manager aam;

			bool render_texture_with_color;

			bool ensure_tex_fbo_combi(cgv::render::context& ctx, cgv::render::texture& tex, cgv::render::frame_buffer& fbo, int width, int height);
			void draw_label_backgrounds(cgv::render::context& ctx, const std::vector<uint32_t>& indices, bool all, bool swap);
			void draw_label_texts(cgv::render::context& ctx, const std::vector<uint32_t>& indices, int height, bool all, bool swap);
			void compute_label_size(label& l);
		public:
			//! construct label manager
			/*! First parameter controls whether texture has a color format and labels are drawn to texture with color.
				Otherwise the texture only has a red channel. In this case rendering of the labels should use the */
			label_manager(bool _render_texture_with_color = true, cgv::media::font::font_face_ptr _font_face = 0, float _font_size = -1);
			/// set the number of texels by which labels are extended in texture space to avoid texture filtering problems at label boundaries, defaults to 4
			void set_safety_extension(int nr_texels) { safety_extension = nr_texels; packing_outofdate = true; }
			/// return number of texels by which labels are extended in texture space to avoid texture filtering problems
			int get_safety_extension() const { return safety_extension; }
			/// set default font face active at begin of each label
			void set_font_face(cgv::media::font::font_face_ptr _font_face);
			/// set default font size active at begin of each label
			void set_font_size(float _font_size);
			/// set default text color active at begin of each label, defaults to opaque black
			void set_text_color(const rgba& clr);
			/// return current default text color
			const rgba& get_text_color() const { return text_color; }
			//! add a label and return its index
			/*! If width and height are given, the label size is fixed even if the text does not
				fit into the label.
				If width and height are not given, they are computed in compute_label_sizes() and
				are marked as unfixed. If later on the label text is changed, sizes are recomputed
				and atlas packing is also executed again. You can fix label size after the call to
				compute_label_sizes() with the fix_label_size() function on a per label basis.*/
			uint32_t add_label(const std::string& text,
				const rgba& bg_clr, int _border_x = 4, int _border_y = 4,
				int _width = -1, int height = -1);
			/// fix the label size of a previously unfixed label. Call after compute_label_sizes()
			void fix_label_size(uint32_t li);
			/// return whether labels need to be packed
			bool is_packing_outofdate() const { return packing_outofdate; }
			/// pack the sized labels into a texture whose width and height in texels is automatically estimated
			void pack_labels();
			//! for given label return where it is placed in the atlas texture
			/*! the texture range is encoded as vec4(u_min, v_min, u_max, v_max) such that a reinterpret_cast
				to box2 is valid. */
			vec4 get_texcoord_range(uint32_t label_index);
			/// return number of labels
			uint32_t get_nr_labels() const { return uint32_t(labels.size()); }
			/// return specific label
			const label& get_label(uint32_t i) const { return labels[i]; }
			//! update text of given label
			/*! if label is not a fixed label set packing out of date otherwise
				only texture computation is set out of date */
			void update_label_text(uint32_t i, const std::string& new_text);
			/// update label size, what always sets packing out of date
			void update_label_size(uint32_t i, int w, int h);
			/// update label color, what always sets packing out of date
			void update_label_background_color(uint32_t i, const rgba& background_color);
			/// you can enforce texture recomputation in ensure_texture_uptodate() by calling this function (typically you do not need this function)
			void set_texture_outofdate() { texture_outofdate = true; }
			/// call init() function from within the init function of your drawable
			void init(cgv::render::context& ctx);
			//! call this function to ensure that texture is up to date
			/*! this function automatically calls the compute_label_sizes(), pack_labels(), and draw_labels()
				functions in case this is necessary due to changes done to labels since last atlas texture
				computation.*/
			void ensure_texture_uptodate(cgv::render::context& ctx);
			/// return whether texture is out of date
			bool is_texture_outofdate() const { return texture_outofdate; }
			/// give access to atlas texture
			std::shared_ptr<cgv::render::texture> get_texture() const { return tex; }
			/// draws the labels to the texture (you typically do not need this function)
			void draw_labels(cgv::render::context& ctx, bool all);
			/// call in the drawable::clear() function of your drawable to destruct the atlas texture
			void destruct(cgv::render::context& ctx);
		};
	}
}

#include <cgv/config/lib_end.h>