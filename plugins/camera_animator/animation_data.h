#pragma once

#include <cgv/data/interval_map.h>
#include <cgv/math/fvec.h>
#include <cgv/render/view.h>

#include "easing_functions.h"

struct view_parameters {
	cgv::vec3 eye_position = cgv::vec3(0.0f, 0.0f, 1.0f);
	cgv::vec3 focus_position = cgv::vec3(0.0f);
	cgv::vec3 up_direction = cgv::vec3(0.0f, 1.0f, 0.0f);

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

	cgv::vec3 view_direction() const {

		return normalize(focus_position - eye_position);
	}

	cgv::vec3 side_direction() const {

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

class animation_data {
public:
	using keyframe_map = cgv::data::interval_map<size_t, keyframe>;

private:
	/// Stores keyframes indexed by their frame number.
	keyframe_map keyframes;

public:
	size_t timecode = 30; // fps
	float time = 0.0f;
	size_t frame = 0;
	bool use_continuous_time = false;

	animation_data() {}

	/// Clears the content of this animation and resets it.
	void clear() {

		reset();
		keyframes.clear();
	}

	/// Resets the current frame and time to zero.
	void reset() {

		time = 0.0f;
		frame = 0;
	}

	const keyframe_map& ref_keyframes() const {

		return keyframes;
	};

	/// Returns a pointer to the keyframe at the given frame. If such a keyframe does not exist, returns a null pointer.
	keyframe* keyframe_at(size_t frame) {

		auto it = keyframes.find(frame);
		return it != keyframes.end() ? &it->second : nullptr;
	}

	bool insert_keyframe(size_t frame, keyframe key) {

		auto ret = keyframes.insert(frame, key);
		return ret.second;
	}

	bool erase_keyframe(size_t frame) {

		return keyframes.erase(frame);
	}

	bool move_keyframe(size_t source_frame, size_t target_frame) {

		auto target_it = keyframes.find(target_frame);

		if(target_it == keyframes.end()) {
			auto source_it = keyframes.find(source_frame);

			if(source_it != keyframes.end()) {
				keyframe value_copy = source_it->second;

				keyframes.erase(source_it);
				keyframes.insert(target_frame, value_copy);

				return true;
			}
		}
		return false;
	}

	/// Find the tween covering the given frame. The tween is defined by a start keyframe located before or at the given
	/// frame and an end keyframe located after the given frame. Returns true for a valid tween where at least one keyframe
	/// is found. If the start/end keyframe is missing, it is duplicated from the end/start keyframe. Returns false for an
	/// invalid tween where both the start an end keyframes are undefined.
	bool find_tween(size_t frame, tween_data& tween) const {
	
		auto pair = keyframes.bounds(frame);

		if(pair.first == keyframes.cend() && pair.second == keyframes.cend())
			return false;

		if(pair.first == keyframes.cend())
			pair.first = pair.second;

		if(pair.second == keyframes.cend())
			pair.second = pair.first;

		tween.start_frame = pair.first->first;
		tween.start_key = pair.first->second;
		tween.end_frame = pair.second->first;
		tween.end_key = pair.second->second;

		return true;
	}

	/// Returns the total duration in seconds using the stored timecode.
	float duration() const {

		return frame_to_time(frame_count());
	}

	/// Returns the total frame count defined by the keyframe with the highest frame number.
	size_t frame_count() const {

		if(keyframes.empty())
			return 0;
		
		return keyframes.crbegin()->first;
	}

	/// Returns the time in seconds of the given frame using the stored timecode.
	float frame_to_time(size_t frame) const {

		return static_cast<float>(frame) / static_cast<float>(timecode);
	}

	/// Returns the nearest frame number (rounded down) of the given time in seconds using the stored timecode.
	size_t time_to_frame(float time) const {

		return static_cast<size_t>(static_cast<float>(timecode) * time);
	}

	/// Gets interpolated view parameters for the currently set frame. Return true if the interpolated view is
	/// defined by a valid tween, false otherwise. If the use_continuous_time flag is set, uses the currently
	/// set time for interpolation instead.
	bool current_view(view_parameters& parameters) const {

		float tc = static_cast<float>(timecode);
		size_t f = use_continuous_time ? time_to_frame(time) : frame;

		tween_data tween;
		if(!find_tween(f, tween))
			return false;

		parameters = use_continuous_time ? tween.interpolate_by_time(time, tc) : tween.interpolate_by_frame(f);
		return f <= frame_count();
	}

	/// Changes the duration after the given frame to the number of given frames. All subsequent frames are moved
	/// to preserve their durations. The minimum allowed duration in frames is 1. No changes are made if a duration
	/// of 0 frames is given. If -1 if given for the frame, the duration before the first frame is changed.
	void change_duration_after(size_t frame, size_t frames) {

		if(frame != -1 && frames == 0ull || keyframes.empty())
			return;

		auto it = keyframes.begin();
		int delta = static_cast<int>(frames);
		std::vector<std::pair<int, int>> moves;

		if(frame == -1) {
			if(it != keyframes.end()) {
				int current_frames = static_cast<int>(it->first);
				delta = std::max(delta - current_frames, -current_frames);
			}
		} else {
			auto bounds = keyframes.bounds(frame);

			if(bounds.first != keyframes.end() && bounds.second != keyframes.end()) {
				it = bounds.second;
				delta -= static_cast<int>(bounds.second->first - bounds.first->first);
			}
		}

		if(delta != 0) {
			std::vector<std::pair<int, int>> moves;

			for(; it != keyframes.end(); ++it) {
				int it_frame = static_cast<int>(it->first);
				moves.push_back({ it_frame, it_frame + delta });
			}

			if(delta > 0) {
				for(auto it = moves.rbegin(); it != moves.rend(); ++it)
					move_keyframe(it->first, it->second);
			} else {
				for(auto it = moves.begin(); it != moves.end(); ++it)
					move_keyframe(it->first, it->second);
			}
		}
	}
};
