#pragma once

#include <cgv/render/render_types.h>

#include "easing_functions.h"
#include "interval_map.h"

struct view_parameters {
	cgv::render::vec3 eye_position = cgv::render::vec3(0.0f, 0.0f, 1.0f);
	cgv::render::vec3 focus_position = cgv::render::vec3(0.0f);
	cgv::render::vec3 up_direction = cgv::render::vec3(0.0f, 1.0f, 0.0f);

	void extract(const cgv::render::view* view_ptr) {

		eye_position = view_ptr->get_eye();
		focus_position = view_ptr->get_focus();
		up_direction = view_ptr->get_view_up_dir();
	}

	void apply(cgv::render::view* view_ptr) const {

		view_ptr->set_view_up_dir(up_direction);
		view_ptr->set_focus(focus_position);
		view_ptr->set_eye_keep_view_angle(eye_position);
	}

	cgv::render::vec3 view_direction() const {

		return normalize(focus_position - eye_position);
	}

	cgv::render::vec3 side_direction() const {

		return cross(view_direction(), up_direction);
	}
};

class keyframe {
private:
	easing_functions::Id _easing_id = easing_functions::Id::kNone;
	std::function<float(float)> _easing_function = nullptr;

public:
	view_parameters camera_state;

	keyframe() {}

	keyframe(const view_parameters& view, easing_functions::Id id) : camera_state(view) {
		
		ease(id);
	}

	void ease(easing_functions::Id id) {

		_easing_id = id;
		_easing_function = easing_functions::from_id(id);
	}

	easing_functions::Id easing_id() const { return _easing_id; }

	std::function<float(float)> easing_function() const {

		return _easing_function;
	}
};

struct tween_data {
	size_t start_frame = 0;
	size_t end_frame = 0;
	keyframe start_key;
	keyframe end_key;

	view_parameters interpolate(float t) {

		auto ease = start_key.easing_function();
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

	animation_data() {}

	keyframe* keyframe_at(size_t frame) {

		auto it = keyframes.find(frame);
		return it != keyframes.end() ? &it->second : nullptr;
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

	float duration() {

		return static_cast<float>(frame_count()) / static_cast<float>(timecode);
	}

	size_t frame_count() {

		if(keyframes.empty())
			return 0;
		
		return keyframes.rbegin()->first;
	}

	size_t time_to_frame() const {

		return static_cast<size_t>(static_cast<float>(timecode) * time);
	}

	void reset() {

		time = 0.0f;
		frame = 0;
	}

	 bool current_view(view_parameters& parameters) {

		float tc = static_cast<float>(timecode);
		size_t f = use_continuous_time ? time_to_frame() : frame;

		tween_data tween;
		if(!find_tween(f, tween))
			return false;

		parameters = use_continuous_time ? tween.interpolate_by_time(time, tc) : tween.interpolate_by_frame(f);
		return f <= frame_count();
	}
};
