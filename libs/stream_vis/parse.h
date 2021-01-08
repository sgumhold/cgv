#pragma once

#include <cgv/render/render_types.h>
#include <map>

bool parse_bool (const std::map<std::string, std::string>& pp, const std::string& name, bool& b);
bool parse_float(const std::map<std::string, std::string>& pp, const std::string& name, float& flt);
bool parse_color(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::rgb& color);
bool parse_quat(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::quat& quat);
bool parse_vecn (const std::map<std::string, std::string>& pp, const std::string& name, float* v, uint32_t dim);
bool parse_vec2 (const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::vec2& v);
bool parse_vec3 (const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::vec3& v);
bool parse_vec4 (const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::vec4& v);
bool parse_ivec2(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::ivec2& v);
bool parse_ivec3(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::ivec3& v);
bool parse_ivec4(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::ivec4& v);



