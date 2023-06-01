#pragma once

#include <cgv/render/render_types.h>

#include "easing_functions.h"



template<class key_type, class value_type>
class interval_map {
private:
	typedef typename std::map<key_type, value_type> map_type;
	typedef typename map_type::size_type size_type;
	typedef typename map_type::iterator iterator_type;
	typedef typename map_type::const_iterator const_iterator_type;
	typedef typename map_type::reverse_iterator reverse_iterator_type;
	typedef typename map_type::const_reverse_iterator const_reverse_iterator_type;

	map_type data;

public:
	void clear() {
		data.clear();
	}

	bool empty() const {

		return data.empty();
	}

	size_type size() const {

		return data.size();
	}

	void insert(key_type key, const value_type& value) {

		data.insert({ key, value });
	}

	size_type erase(key_type frame) {

		return data.erase(frame);
	}

	iterator_type erase(iterator_type it) {

		return data.erase(it);
	}

	iterator_type find(key_type key) {

		return data.find(key);
	}

	bool move(key_type source_key, key_type target_key) {

		auto target_it = data.find(target_key);

		if(target_it == data.end()) {
			auto source_it = data.find(source_key);

			if(source_it != data.end()) {
				value_type value_copy = source_it->second;

				data.erase(source_it);
				data.insert({ target_key, value_copy });

				return true;
			}
		}
		return false;
	}

	iterator_type begin() { return data.begin(); }

	iterator_type end() { return data.end(); }

	const_iterator_type begin() const { return data.cbegin(); }

	const_iterator_type end() const { return data.cend(); }

	reverse_iterator_type rbegin() { return data.rbegin(); }

	reverse_iterator_type rend() { return data.rend(); }

	const_reverse_iterator_type rbegin() const { return data.crbegin(); }

	const_reverse_iterator_type rend() const { return data.crend(); }

	// Returns an iterator pointing to the first element in the container whose key is equivalent or smaller than the given key. If no such element exists, returns end().
	iterator_type lower_bound(key_type key) {

		auto it = data.lower_bound(key);

		if(it == data.end())
			return data.empty() ? data.end() : std::prev(it);
		else if(it->first == key)
			return it;
		else if(it == data.begin())
			return data.end();
		else
			return std::prev(it);
	}

	// Returns an iterator pointing to the first element in the container whose key is greater than the given key. If no such element exists, returns end().
	iterator_type upper_bound(key_type key) {

		return data.upper_bound(key);
	}

	// Returns a pair of iterators pointing to the lower and upper bounds of the given key.
	// The lower bound is defined as the greatest key <= than the given key.
	// The upper bound is defined as the smallest key > than the given key.
	std::pair<iterator_type, iterator_type> bounds(key_type key) {

		return { lower_bound(key), upper_bound(key) };
	}
};

struct view_parameters {
	cgv::render::vec3 eye_position = cgv::render::vec3(0.0f, 0.0f, 1.0f);
	cgv::render::vec3 focus_position = cgv::render::vec3(0.0f);
	cgv::render::vec3 up_direction = cgv::render::vec3(0.0f, 1.0f, 0.0f);
};

struct keyframe {
	view_parameters camera_state;
	std::function<float(float)> easing_function = nullptr;

	keyframe() {}
};

struct tween_data {
	size_t start_frame = 0;
	size_t end_frame = 0;
	keyframe start_key;
	keyframe end_key;

	view_parameters interpolate(float t) {

		auto ease = start_key.easing_function;
		t = ease ? ease(t) : t;

		view_parameters state;
		state.eye_position = cgv::math::lerp(start_key.camera_state.eye_position, end_key.camera_state.eye_position, t);
		state.focus_position = cgv::math::lerp(start_key.camera_state.focus_position, end_key.camera_state.focus_position, t);
		state.up_direction = cgv::math::lerp(start_key.camera_state.up_direction, end_key.camera_state.up_direction, t);

		return state;
	}

	view_parameters interpolate_by_frame(size_t frame) {

		if(end_frame <= start_frame)
			return start_key.camera_state;

		size_t tween_frame = cgv::math::clamp(frame, start_frame, end_frame) - start_frame;
		size_t tween_length = end_frame - start_frame;

		float t = static_cast<float>(tween_frame) / static_cast<float>(tween_length);

		return interpolate(t);
	}

	view_parameters interpolate_by_time(float time, float timecode) {

		if(end_frame <= start_frame)
			return start_key.camera_state;

		float start_time = start_frame / timecode;
		float end_time = end_frame / timecode;
		float duration = end_time - start_time;

		float t = cgv::math::clamp((time - start_time) / duration, 0.0f, 1.0f);

		return interpolate(t);
	}
};

struct animation_data {
	
	size_t timecode = 30; // fps
	float time = 0.0f;
	size_t frame = 0;
	bool use_continuous_time = false;

	interval_map <size_t, keyframe> keyframes;

	animation_data() {

		size_t i = 0;
		keyframe k;
		k.easing_function = &easing_functions::smoothstep7;
		k.camera_state = { cgv::render::vec3(0.0f, 0.8f, -1.3f), cgv::render::vec3(0.0f), normalize(cgv::render::vec3(0.0f, -0.851069f, -0.524985f)) };
		keyframes.insert(i, k);
			
		i += 30;
		k.camera_state.eye_position = cgv::render::vec3(-0.75f, 0.679759f, -0.957661f);
		keyframes.insert(i, k);

		i += 60;
		k.camera_state.eye_position = cgv::render::vec3(0.75f, 0.679759f, -0.957661f);
		keyframes.insert(i, k);

		i += 30;
		k.camera_state.eye_position = cgv::render::vec3(0.0f, 0.8f, -1.3f);
		keyframes.insert(i, k);

		i += 15;
		keyframes.insert(i, k);

		// 48hr_A
		i += 4*30;
		k.camera_state = {
			cgv::render::vec3(0.164111f, 0.121793f, -0.139523f),
			cgv::render::vec3(0.227631f, 0.0562004f, -0.0379729f),
			cgv::render::vec3(0.276855f, -0.718945f, -0.63755f)
		};
		keyframes.insert(i, k);

		i += 30;
		keyframes.insert(i, k);

		i += 4 * 30;
		k.camera_state = {
			cgv::render::vec3(0.312469f, 0.119438f, -0.124305f),
			cgv::render::vec3(0.227631f, 0.0562004f, -0.0379729f),
			cgv::render::vec3(-0.519882f, -0.360074f, -0.774641f)
		};
		keyframes.insert(i, k);

		
		
		// 40hr_A1
		/*i += 4*30;
		k.camera_state = {
			cgv::render::vec3(-0.195507f, 0.0609891f, -0.0932227f),
			cgv::render::vec3(-0.237523f, -0.02731f, -0.0136831f),
			cgv::render::vec3(-0.357586f, -0.524408f, -0.772741f)
		};
		keyframes.insert({ i, k });

		i += 30;
		keyframes.insert({ i, k });

		i += 4 * 30;
		k.camera_state = {
			cgv::render::vec3(-0.341674f, 0.000596929f, -0.0789718f),
			cgv::render::vec3(-0.237523f, -0.02731f, -0.0136831f),
			cgv::render::vec3(0.4766f, -0.212322f, -0.853095f)
		};
		keyframes.insert({ i, k });*/

		
			
			
		// 40hr_C1
		/*i += 4 * 30;
		k.camera_state = {
			cgv::render::vec3(0.0899829f, -0.190682f, -0.113816f),
			cgv::render::vec3(0.135027f, -0.232518, -0.0337936f),
			cgv::render::vec3(0.395473f, -0.545062f, -0.739262f)
		};
		keyframes.insert({ i, k });

		i += 30;
		keyframes.insert({ i, k });

		i += 4 * 30;
		k.camera_state = {
			cgv::render::vec3(0.188692f, -0.171772f, -0.0939006f),
			cgv::render::vec3(0.135027f, -0.232518f, -0.0337936f),
			cgv::render::vec3(-0.261154f, -0.353045f, -0.89842f)
		};
		keyframes.insert({ i, k });*/
	}

	static std::function<float(float)> default_easing_function() {

		return &easing_functions::linear;
	}

	bool find_tween(size_t frame, tween_data& tween) {

		auto pair = keyframes.bounds(frame);

		if(pair.first == keyframes.end() && pair.second == keyframes.end())
			return false;

		if(pair.first == keyframes.end())
			pair.first = pair.second;

		if(pair.second == keyframes.end())
			pair.second = pair.first;

		tween.start_frame = pair.first->first;
		tween.start_key = pair.first->second;
		tween.end_frame = pair.second->first;
		tween.end_key = pair.second->second;

		return true;
	}

	size_t frame_count() {

		if(keyframes.empty())
			return 0;
		
		return keyframes.rbegin()->first;
	}

	void reset() {

		time = 0.0f;
		frame = 0;
	}

	view_parameters current_view() {

		view_parameters view;

		float tc = static_cast<float>(timecode);
		size_t f = use_continuous_time ? round(tc * time) : frame;

		tween_data tween;
		if(!find_tween(f, tween))
			return view;

		return use_continuous_time ? tween.interpolate_by_time(time, tc) : tween.interpolate_by_frame(f);
	}

	bool apply(cgv::render::view* view_ptr) {

		float tc = static_cast<float>(timecode);
		size_t f = use_continuous_time ? round(tc * time) : frame;

		tween_data tween;
		if(!find_tween(frame, tween))
			return false;

		if(use_continuous_time)
			set_view(view_ptr, tween.interpolate_by_time(time, tc));
		else
			set_view(view_ptr, tween.interpolate_by_frame(frame));

		return frame < frame_count();
	}

	void set_view(cgv::render::view* view_ptr, const view_parameters& parameters) {

		view_ptr->set_view_up_dir(parameters.up_direction);
		view_ptr->set_focus(parameters.focus_position);
		view_ptr->set_eye_keep_view_angle(parameters.eye_position);
	}
};
