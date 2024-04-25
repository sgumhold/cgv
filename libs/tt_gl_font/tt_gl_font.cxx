#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "tt_gl_font.h"
#include <cgv/utils/advanced_scan.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>

#ifdef WIN32
static const char* default_font_path = "C:/windows/fonts";
#else
static const char* default_font_path = "/usr/share/fonts";
#endif

/// variant of stbtt function that starts with initialized font and supports a border between glyphs
static int cgv_BakeFontBitmap(const stbtt_fontinfo& f,
	float pixel_height,                     // height of font in pixels
	unsigned char* pixels, int pw, int ph,  // bitmap to be filled in
	int first_char, int num_chars,          // characters to bake
	stbtt_bakedchar* chardata, int border)
{
	float scale;
	int x, y, bottom_y, i;
	STBTT_memset(pixels, 0, pw * ph); // background of 0 around pixels
	x = y = border;
	bottom_y = border;

	scale = stbtt_ScaleForPixelHeight(&f, pixel_height);
	static int ref_idx = -1;
	for (i = 0; i < num_chars; ++i) {
		int advance, lsb, x0, y0, x1, y1, gw, gh;
		int g = stbtt_FindGlyphIndex(&f, first_char + i);
		stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
		stbtt_GetGlyphBitmapBox(&f, g, scale, scale, &x0, &y0, &x1, &y1);
		if (g == 0) {
			if (ref_idx == -1)
				ref_idx = i;
			else {
				chardata[i] = chardata[ref_idx];
				continue;
			}
		}
		gw = x1 - x0;
		gh = y1 - y0;
		if (x + gw + border >= pw)
			y = bottom_y, x = border; // advance to next row
		if (y + gh + border >= ph) // check if it fits vertically AFTER potentially moving to next row
			return -i;
		STBTT_assert(x + gw < pw);
		STBTT_assert(y + gh < ph);
		stbtt_MakeGlyphBitmap(&f, pixels + x + y * pw, gw, gh, pw, scale, scale, g);
		chardata[i].x0 = (stbtt_int16)x;
		chardata[i].y0 = (stbtt_int16)y;
		chardata[i].x1 = (stbtt_int16)(x + gw);
		chardata[i].y1 = (stbtt_int16)(y + gh);
		chardata[i].xadvance = scale * advance;
		chardata[i].xoff = (float)x0;
		chardata[i].yoff = (float)y0;
		x = x + gw + border;
		if (y + gh + border > bottom_y)
			bottom_y = y + gh + border;
	}
	return bottom_y;
}

/// bake initialized font with border into bitmap of which dimension is determined automatically
static void cgv_BakeFontBitmap(
	const stbtt_fontinfo& f, float pixel_height, int border,
	int first_char, int num_chars, stbtt_bakedchar* chardata,
	unsigned& bitmap_width, unsigned& bitmap_height, std::vector<unsigned char>& bitmap)
{
	float pixel_estimate = num_chars * 0.5f * (pixel_height + border) * (pixel_height + border);
	unsigned bitmap_extent = unsigned(pow(2.0, floor(log(sqrt(pixel_estimate)) / log(2.0))));
	bitmap_width = bitmap_extent;
	bitmap_height = bitmap_extent;

	bool increase_width = true;
	while (true) {
		bitmap.resize(bitmap_width * bitmap_height);
		int res = cgv_BakeFontBitmap(f, pixel_height, bitmap.data(), bitmap_width, bitmap_height,
			first_char, num_chars, chardata, border);
		if (res >= 0)
			break;
		if (increase_width)
			bitmap_width *= 2;
		else
			bitmap_height *= 2;
		increase_width = !increase_width;
	}
}

int extract_font_face(std::string& font_name)
{
	std::vector<cgv::utils::token> tokens;
	cgv::utils::split_to_tokens(font_name, tokens, "", false);
	int ffa = 0;
	size_t idx = tokens.size() - 1;
	int nr = std::min((int)idx, 2);
	for (int i = 0; i < nr; ++i) {
		if (cgv::utils::to_upper(to_string(tokens[idx])) == "BOLD")
			ffa |= cgv::media::font::FFA_BOLD;
		else if (cgv::utils::to_upper(to_string(tokens[idx])) == "ITALIC")
			ffa |= cgv::media::font::FFA_ITALIC;
		else
			break;
		--idx;
	}
	font_name = std::string(tokens.front().begin, tokens[idx].end);
	return ffa;
}

bool extract_font_name(stbtt_fontinfo& f, std::string& font_name)
{
	int length;
	const char* name = stbtt_GetFontNameString(&f, &length, STBTT_PLATFORM_ID_MICROSOFT, STBTT_MS_EID_UNICODE_BMP, STBTT_MS_LANG_ENGLISH, 4);
	//insert in stb_truetype.h before line 4774 for debugging
	//stbtt_uint16 plID = ttUSHORT(fc + loc + 0);
	//stbtt_uint16 enID = ttUSHORT(fc + loc + 2);
	//stbtt_uint16 laID = ttUSHORT(fc + loc + 4);
	//stbtt_uint16 naID = ttUSHORT(fc + loc + 6);
	//std::cout << i << ": plattform = " << plID << ", encoding = " << enID << ", language = " << laID << ", name = " << naID << std::endl;
	if (!name)
		return false;
	if (length > 0 && name[0] == 0) {
		font_name.clear();
		for (int i = 0; i < length; ++i)
			if (name[i] != 0)
				font_name.push_back(name[i]);
	}
	else
		font_name = std::string(name, length);
	return true;
}

typedef std::pair<std::string, int> ff_cache_key;
typedef std::tuple<float, unsigned, unsigned> tex_cache_key;
typedef std::pair< cgv::render::texture*, std::vector<stbtt_bakedchar> > tex_cache_entry_type;
typedef std::map<tex_cache_key, tex_cache_entry_type> tex_cache_type;
typedef std::map<ff_cache_key, tex_cache_type> ff_cache_type;

ff_cache_type& ref_ff_cache()
{
	static ff_cache_type cache;
	return cache;
}

void add_texture_to_cache(cgv::render::texture* tex_ptr, const std::vector<stbtt_bakedchar>& baked_chars, const std::string& font_name, int ffa, float font_size, unsigned fst_char, unsigned nr_chars)
{
	ref_ff_cache()[ff_cache_key(font_name, ffa)][tex_cache_key(font_size, fst_char, nr_chars)] = tex_cache_entry_type(tex_ptr, baked_chars);
}

cgv::render::texture* get_texture_from_cache(std::vector<stbtt_bakedchar>& baked_chars, const std::string& font_name, int ffa, float font_size, unsigned fst_char, unsigned nr_chars)
{
	auto iter = ref_ff_cache().find(ff_cache_key(font_name, ffa));
	if (iter == ref_ff_cache().end())
		return 0;
	auto jter = iter->second.find(tex_cache_key(font_size, fst_char, nr_chars));
	if (jter == iter->second.end())
		return 0;
	baked_chars = jter->second.second;
	return jter->second.first;
}


namespace cgv {

	cgv::render::rectangle_render_style& ref_rectangle_render_style()
	{
		static cgv::render::rectangle_render_style rrs;
		static bool initialized = false;
		if (!initialized) {
			rrs.pixel_blend = 0.0f;
			rrs.texture_mode = cgv::render::RTM_REPLACE_ALPHA;
			rrs.map_color_to_material = cgv::render::CM_COLOR_AND_OPACITY;
			rrs.illumination_mode = cgv::render::IM_OFF;
		}
		return rrs;
	}

	cgv::render::attribute_array_manager& ref_attribute_manager(cgv::render::context& ctx)
	{
		static cgv::render::attribute_array_manager aam;
		static bool initialized = false;
		if (!initialized) {
			aam.init(ctx);
			initialized = true;
		}
		return aam;
	}

	struct ext_font_face_info : public font_face_info
	{
		std::string font_name;
		int ffa;
		ext_font_face_info(const std::string& _file_name, unsigned _nr_glyphs, int _fi, const std::string& _font_name, int _ffa) :
			font_face_info(_file_name, _nr_glyphs, _fi), font_name(_font_name), ffa(_ffa)
		{

		}
		bool operator < (const ext_font_face_info& ffi) const {
			return font_name < ffi.font_name ||
				(font_name == ffi.font_name && ffa < ffi.ffa);
		}
	};

	std::map<std::string, font_info>& ref_font_table()
	{
		static std::map<std::string, font_info> font_table;
		return font_table;
	}

	void ensure_font_table()
	{
		if (fonts_scanned())
			return;
		scan_fonts();
	}

	bool& ref_fonts_scanned()
	{
		static bool scanned = false;
		return scanned;
	}
	/// return whether fonts have been scanned
	bool fonts_scanned()
	{
		return ref_fonts_scanned();
	}

	std::map<std::string, tt_gl_font_ptr>& ref_font_cache()
	{
		static std::map<std::string, tt_gl_font_ptr> font_cache;
		return font_cache;
	}

	void scan_fonts(const std::vector<std::string>& file_names)
	{
		if (file_names.empty())
			return;
		std::vector<ext_font_face_info> ffis;
		std::vector<unsigned char> ttf_buffer;
		for (auto file_name : file_names) {
			if (!cgv::utils::file::exists(file_name))
				continue;
			size_t file_size = cgv::utils::file::size(file_name);
			ttf_buffer.resize(file_size);
			if (!cgv::utils::file::read(file_name, (char*)ttf_buffer.data(), file_size))
				continue;
			int nr_fonts = stbtt_GetNumberOfFonts(ttf_buffer.data());
			if (nr_fonts > 1) {
				std::cout << "found font collection with " << nr_fonts << " fonts." << std::endl;
			}
			for (int fi = 0; fi < nr_fonts; ++fi) {
				stbtt_fontinfo f;
				if (0 == stbtt_InitFont(&f, ttf_buffer.data(), stbtt_GetFontOffsetForIndex(ttf_buffer.data(), fi)))
					continue;
				std::string font_name;
				if (!extract_font_name(f, font_name))
					continue;
				int ffa = extract_font_face(font_name);
				// std::cout << font_name << " has " << font.numGlyphs << " glyphs" << std::endl;
				ffis.push_back(ext_font_face_info(file_name, unsigned(f.numGlyphs), fi, font_name, ffa));
			}
		}
		std::sort(ffis.begin(), ffis.end());
		ref_font_table().clear();
		ref_font_cache().clear();
		size_t i = 0;
		font_info* fi_ptr = 0;
		std::string last_name;
		while (i < ffis.size()) {
			if (ffis[i].font_name != last_name) {
				fi_ptr = &ref_font_table()[ffis[i].font_name];
				fi_ptr->font_faces[ffis[i].ffa] = ffis[i];
				last_name = ffis[i].font_name;
			}
			else {
				fi_ptr->font_faces[ffis[i].ffa] = ffis[i];
			}
			++i;
		}
		ref_fonts_scanned() = true;
	}

	void scan_fonts(std::string font_directory)
	{
		if (font_directory.empty())
			font_directory = default_font_path;
		std::vector<std::string> file_names;
		cgv::utils::dir::glob(font_directory, file_names, "*.ttf", true);
		scan_fonts(file_names);
	}

	void tt_gl_font_face::bake_characters()
	{
		baked_chars.resize(nr_chars);
		int border = 1;
		if (build_mipmap)
			border = int(font_size / 6);
		cgv_BakeFontBitmap(f, font_size, border, fst_char, nr_chars,
			baked_chars.data(), bitmap_width, bitmap_height, bitmap);
	}
	void tt_gl_font_face::ensure_bitmap()
	{
		if (!bitmap_out_of_date)
			return;
		bake_characters();
		bitmap_out_of_date = false;
	}
	void tt_gl_font_face::ensure_texture(cgv::render::context& ctx)
	{
		if (!tex_out_of_date)
			return;
		ensure_bitmap();
		if (tex_ptr == 0) {
			tex_ptr = new cgv::render::texture("[R]");
			tex_ptr->set_min_filter(cgv::render::TF_ANISOTROP, 8.0f);
		}
		else if (tex_ptr->is_created() && tex_ptr->get_width() != bitmap_width || tex_ptr->get_height() != bitmap_height)
			tex_ptr->destruct(ctx);
		ctx_ptr = &ctx;
		cgv::data::data_format df(bitmap_width, bitmap_height, cgv::type::info::TI_UINT8, cgv::data::CF_R);
		cgv::data::data_view dv(&df, bitmap.data());
		if (!tex_ptr->is_created())
			tex_ptr->create(ctx, cgv::render::TT_2D, bitmap_width, bitmap_height);
		tex_ptr->replace(ctx, 0, 0, dv);
		
		tex_ptr->generate_mipmaps(ctx);
		add_texture_to_cache(tex_ptr, baked_chars, font_name, ffa, font_size, fst_char, nr_chars);
		tex_out_of_date = false;
	}
	bool tt_gl_font_face::read_font(const std::string& file_name, int fi)
	{
		if (!cgv::utils::file::exists(file_name))
			return false;
		size_t file_size = cgv::utils::file::size(file_name);
		internal_ttf_buffer.resize(file_size);
		ttf_buffer = internal_ttf_buffer.data();
		if (!cgv::utils::file::read(file_name, (char*)internal_ttf_buffer.data(), file_size))
			return false;
		int nr_fonts = stbtt_GetNumberOfFonts(internal_ttf_buffer.data());
		if (fi >= nr_fonts)
			return false;
		if (0 == stbtt_InitFont(&f, internal_ttf_buffer.data(), stbtt_GetFontOffsetForIndex(internal_ttf_buffer.data(), fi)))
			return false;
		std::string font_name;
		if (!extract_font_name(f, font_name))
			return false;
		ffa = extract_font_face(font_name);
		this->font_name = font_name;
		bitmap_out_of_date = true;
		tex_out_of_date = true;
		return true;
	}

	/// construct font face
	tt_gl_font_face::tt_gl_font_face(const std::string file_name, float _font_size, int fi) : cgv::media::font::font_face(0)
	{
		build_mipmap = true;
		font_size = _font_size;
		bitmap_width = bitmap_height = 0;
		fst_char = 32;
		nr_chars = 224;
		ctx_ptr = 0;
		tex_ptr = 0;
		if (!read_font(file_name, fi)) {
			internal_ttf_buffer.clear();
			ttf_buffer = 0;
		}
	}
	///
	bool tt_gl_font_face::is_valid() const {
		return ttf_buffer != nullptr;
	}
	/// construct font face
	tt_gl_font_face::tt_gl_font_face(const std::string name, const stbtt_fontinfo& _f, float _font_size, unsigned char* _ttf_buffer, int ffa) :
		cgv::media::font::font_face(ffa), ttf_buffer(_ttf_buffer), font_name(name), f(_f)
	{
		build_mipmap = true;
		bitmap_width = bitmap_height = 0;
		fst_char = 32;
		nr_chars = 224;
		font_size = _font_size;
		ctx_ptr = 0;
		tex_ptr = 0;
	}
	/// destruct font face
	tt_gl_font_face::~tt_gl_font_face()
	{
		if (tex_ptr && tex_ptr->is_created() && ctx_ptr) {
			if (ctx_ptr->make_current()) {
				tex_ptr->destruct(*ctx_ptr);
				delete tex_ptr;
			}
		}
	}
	/// returns "tt_gl_font_face"
	std::string tt_gl_font_face::get_type_name() const { return "tt_gl_font_face"; }
	/// enumerate the supported sizes
	void tt_gl_font_face::enumerate_sizes(std::vector<int>& supported_sizes) const { }
	/// return the width of a text printed in the given size, which is measured in pixels
	float tt_gl_font_face::measure_text_width(const std::string& text, float font_size) const
	{
		return compute_box(text).get_extent().x();
	}
	/// enables font face of given size and should be called once before calling draw_text functions
	void tt_gl_font_face::enable(void* context_ptr, float _font_size) {
		if (!ctx_ptr)
			ctx_ptr = reinterpret_cast<cgv::render::context*>(context_ptr);
		set_font_size(_font_size);
		if (ctx_ptr)
			ensure_texture(*ctx_ptr);
	}
	/// draw text at given location with rectangle renderer initialized with default render style
	void tt_gl_font_face::draw_text(float& x, float& y, const std::string& text) const
	{
		if (text.empty())
			return;
		if (!ctx_ptr)
			return;
		std::vector<cgv::render::textured_rectangle> Q;
		vec2 p(x, y);
		text_to_quads(p, text, Q);
		x = p[0];
		y = p[1];
		static int init_rr = 1;
		auto& rr = cgv::render::ref_rectangle_renderer(*ctx_ptr, init_rr);
		if (init_rr)
			init_rr = 0;
		rr.set_render_style(ref_rectangle_render_style());
		rr.enable_attribute_array_manager(*ctx_ptr, ref_attribute_manager(*ctx_ptr));
		rr.set_textured_rectangle_array(*ctx_ptr, Q);
		rr.set_color(*ctx_ptr, ctx_ptr->get_color());
		ref_texture(*ctx_ptr).enable(*ctx_ptr);
		GLboolean blend = glIsEnabled(GL_BLEND);
		glEnable(GL_BLEND);
		GLenum blend_src, blend_dst;
		glGetIntegerv(GL_BLEND_DST, reinterpret_cast<GLint*>(&blend_dst));
		glGetIntegerv(GL_BLEND_SRC, reinterpret_cast<GLint*>(&blend_src));
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GLboolean depth = glIsEnabled(GL_DEPTH_TEST);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		rr.render(*ctx_ptr, 0, (GLsizei)Q.size());
		rr.disable_attribute_array_manager(*ctx_ptr, ref_attribute_manager(*ctx_ptr));
		glBlendFunc(blend_src, blend_dst);
		glDepthMask(GL_TRUE);
		if (!blend)
			glDisable(GL_BLEND);
		if (depth)
			glEnable(GL_DEPTH_TEST);
		ref_texture(*ctx_ptr).disable(*ctx_ptr);
	}
	unsigned tt_gl_font_face::get_nr_glyphs() const { return f.numGlyphs; }
	std::string tt_gl_font_face::get_font_name() const { return font_name; }

	void tt_gl_font_face::set_character_range(int _fst_char, unsigned _nr_chars)
	{
		if (fst_char != _fst_char || nr_chars != _nr_chars) {
			fst_char = _fst_char;
			nr_chars = _nr_chars;
			tex_ptr = get_texture_from_cache(baked_chars, font_name, ffa, font_size, fst_char, nr_chars);
			if (tex_ptr == 0) {
				bitmap_out_of_date = true;
				tex_out_of_date = true;
			}
		}
	}
	void tt_gl_font_face::set_font_size(float _font_size)
	{
		if (font_size != _font_size) {
			font_size = _font_size;
			tex_ptr = get_texture_from_cache(baked_chars, font_name, ffa, font_size, fst_char, nr_chars);
			if (tex_ptr == 0) {
				bitmap_out_of_date = true;
				tex_out_of_date = true;
			}
			else {
				bitmap_width = unsigned(tex_ptr->get_width());
				bitmap_height = unsigned(tex_ptr->get_height());
			}
		}
	}
	bool tt_gl_font_face::write_atlas(const std::string& file_name)
	{
		ensure_bitmap();
		cgv::data::data_format df(bitmap_width, bitmap_height, cgv::type::info::TI_UINT8, cgv::data::CF_L);
		cgv::data::const_data_view dv(&df, bitmap.data());
		cgv::media::image::image_writer iw(file_name);
		return iw.write_image(dv);
	}
	cgv::render::texture& tt_gl_font_face::ref_texture(cgv::render::context& ctx) const
	{
		return *tex_ptr;
	}
	cgv::render::texture& tt_gl_font_face::ref_texture(cgv::render::context& ctx)
	{
		ensure_texture(ctx);
		return *tex_ptr;
	}
	unsigned tt_gl_font_face::text_to_quads(vec2& p, const std::string& text, std::vector<cgv::render::textured_rectangle>& Q, float scale, bool flip_y) const
	{
		p /= scale;
		unsigned cnt = 0;
		for (unsigned char c : text) {
			if (c < fst_char || c >= fst_char + nr_chars)
				continue;
			stbtt_aligned_quad q;
			float y0 = p[1];
			stbtt_GetBakedQuad(baked_chars.data(), bitmap_width, bitmap_height, int(c) - fst_char, &p[0], &p[1], &q, 1);
			if (!flip_y) {
				float tmp = q.y0;
				q.y0 = 2 * y0 - q.y1;
				q.y1 = 2 * y0 - tmp;
				p[1] = 2 * y0 - p[1];
				std::swap(q.t1, q.t0);
			}
			Q.resize(Q.size() + 1);
			Q.back().rectangle = box2(scale*vec2(q.x0, q.y0), scale * vec2(q.x1, q.y1));
			Q.back().texcoords = vec4(q.s0, q.t0, q.s1, q.t1);
			++cnt;
		}
		p *= scale;
		return cnt;
	}
	unsigned tt_gl_font_face::text_to_quads(vec2& p, const std::string& text, std::vector<cgv::render::textured_rectangle>& Q, float scale, bool flip_y)
	{
		ensure_bitmap();
		return const_cast<const tt_gl_font_face*>(this)->text_to_quads(p, text, Q, scale, flip_y);
	}
	box2 tt_gl_font_face::compute_box(const std::string& text, float scale, bool flip_y) const
	{
		box2 extent;
		vec2 p(0.0f);
		float y_scale = !flip_y ? -1.0f : 1.0f;
		for (unsigned char c : text) {
			if (c < fst_char || c >= fst_char + nr_chars)
				continue;
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(baked_chars.data(), bitmap_width, bitmap_height, int(c) - fst_char, &p[0], &p[1], &q, 1);
			extent.add_point(vec2(q.x0, y_scale*q.y0));
			extent.add_point(vec2(q.x1, y_scale*q.y1));
		}
		extent.scale(scale);
		return extent;
	}
	vec2 tt_gl_font_face::align_text(const vec2& p, const std::string& text, cgv::render::TextAlignment ta, float scale, bool flip_y) const
	{
		box2 B = compute_box(text, scale, flip_y);
		vec2 a = B.get_center();
		vec2 hE = 0.5f * B.get_extent();
		if (!flip_y)
			hE[1] *= -1.0f;
		if ((ta & cgv::render::TA_BOTTOM) != 0)
			a[1] += hE[1];
		if ((ta & cgv::render::TA_TOP) != 0)
			a[1] -= hE[1];
		if ((ta & cgv::render::TA_LEFT) != 0)
			a[0] -= hE[0];
		if ((ta & cgv::render::TA_RIGHT) != 0)
			a[0] += hE[0];
		return p - a;
	}

	tt_gl_font::tt_gl_font(const std::string& _font_name, float _font_size, const font_info& FI)
	{
		font_name = _font_name;
		for (int ffa = 0; ffa < 4; ++ffa) {
			if (!FI.font_faces[ffa].is_valid())
				continue;
			font_faces[ffa] = new tt_gl_font_face(FI.font_faces[ffa].file_name, _font_size, FI.font_faces[ffa].fi);
		}
	}
	/// returns "font"
	std::string tt_gl_font::get_type_name() const { return "tt_gl_font"; }
	/// return the name of the font
	const char* tt_gl_font::get_name() const {
		return font_name.c_str();
	}
	/// check whether the given font includes a face that include the possibly or-ed together selection of font face attributes
	bool tt_gl_font::supports_font_face(cgv::media::font::FontFaceAttributes ffa) const
	{
		return ffa <= font_faces.size() && !font_faces[ffa].empty();
	}
	/// return a pointer to a font face
	cgv::media::font::font_face_ptr tt_gl_font::get_font_face(cgv::media::font::FontFaceAttributes ffa) const {
		bool font_face_valid = font_faces.at(ffa) != nullptr && font_faces[ffa]->is_valid();
#ifndef NDEBUG
		if (!font_face_valid) {
			std::cout << __func__ << ": Attribute not in font face! Fallback to normal font ..." << std::endl;
			assert(font_faces[cgv::media::font::FFA_REGULAR]->is_valid() && "Not even the regular font face is valid?!");
		}
#endif // !NDEBUG
		return font_face_valid ? font_faces.at(ffa) : font_faces.at(cgv::media::font::FFA_REGULAR);
	}
	/// enumerate the supported font sizes
	void tt_gl_font::enumerate_sizes(std::vector<int>& supported_sizes) const {
	}
	void tt_gl_font::set_character_range(int _fst_char, unsigned _nr_chars)
	{
		for (unsigned fi = 0; fi < 4; ++fi)
			if (font_faces[fi])
				font_faces[fi]->set_character_range(_fst_char, _nr_chars);
	}
	void tt_gl_font::set_font_size(float _font_size)
	{
		for (unsigned fi = 0; fi < 4; ++fi)
			if (font_faces[fi])
				font_faces[fi]->set_font_size(_font_size);
	}

	/// find an installed font by name
	tt_gl_font_ptr find_font(const std::string& font_name)
	{
		ensure_font_table();
		tt_gl_font_ptr& f_ptr = ref_font_cache()[font_name];
		if (f_ptr)
			return f_ptr;
		if (ref_font_table().find(font_name) == ref_font_table().end())
			return tt_gl_font_ptr();
		f_ptr = new tt_gl_font(font_name, 24, ref_font_table()[font_name]);
		return f_ptr;
	}

	size_t& ref_tt_gl_fonts_timestamp()
	{
		static size_t timestamp = 1;
		return timestamp;
	}

	size_t get_tt_gl_fonts_timestamp()
	{
		return ref_tt_gl_fonts_timestamp();
	}

	const std::vector<std::string>& get_font_names()
	{
		static size_t timestamp = 0;
		static std::vector<std::string> font_names;
		ensure_font_table();
		if (timestamp < get_tt_gl_fonts_timestamp()) {
			font_names.clear();
			for (auto iter : ref_font_table())
				font_names.push_back(iter.first.c_str());
			timestamp = get_tt_gl_fonts_timestamp();
		}
		return font_names;
	}
	const std::string& get_font_enum_declaration()
	{
		static size_t timestamp = 0;
		static std::string font_enum_declaration;
		if (timestamp < get_tt_gl_fonts_timestamp()) {
			font_enum_declaration = "enums='";
			const std::vector<std::string>& font_names = get_font_names();
			bool first = true;
			for (auto font_name : font_names) {
				if (first)
					first = false;
				else
					font_enum_declaration += ',';

				font_enum_declaration += font_name;
			}
			font_enum_declaration += "'";
		}
		return font_enum_declaration;
	}
}