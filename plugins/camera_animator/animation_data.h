#pragma once

#include <cgv/render/render_types.h>

#include "easing_functions.h"



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

	std::map<size_t, keyframe> keyframes;

	animation_data() {

		size_t i = 0;
		keyframe k;
		k.easing_function = &easing_functions::smoothstep7;
		k.camera_state = { cgv::render::vec3(0.0f, 0.8f, -1.3f), cgv::render::vec3(0.0f), normalize(cgv::render::vec3(0.0f, -0.851069f, -0.524985f)) };
		keyframes.insert({ i, k });
			
		i += 30;
		k.camera_state.eye_position = cgv::render::vec3(-0.75f, 0.679759f, -0.957661f);
		keyframes.insert({ i, k });

		i += 60;
		k.camera_state.eye_position = cgv::render::vec3(0.75f, 0.679759f, -0.957661f);
		keyframes.insert({ i, k });

		i += 30;
		k.camera_state.eye_position = cgv::render::vec3(0.0f, 0.8f, -1.3f);
		keyframes.insert({ i, k });

		i += 15;
		keyframes.insert({ i, k });

		// 48hr_A
		i += 4*30;
		k.camera_state = {
			cgv::render::vec3(0.164111f, 0.121793f, -0.139523f),
			cgv::render::vec3(0.227631f, 0.0562004f, -0.0379729f),
			cgv::render::vec3(0.276855f, -0.718945f, -0.63755f)
		};
		keyframes.insert({ i, k });

		i += 30;
		keyframes.insert({ i, k });

		i += 4 * 30;
		k.camera_state = {
			cgv::render::vec3(0.312469f, 0.119438f, -0.124305f),
			cgv::render::vec3(0.227631f, 0.0562004f, -0.0379729f),
			cgv::render::vec3(-0.519882f, -0.360074f, -0.774641f)
		};
		keyframes.insert({ i, k });

		
		
		// 40hr_A1
		/*i += 4*30;
		k.camera_state = {
			cgv::render::vec3(-0.170842f, 0.0539438f, -0.0832538f),
			cgv::render::vec3(-0.237523f, -0.02731f, -0.0136831f),
			cgv::render::vec3(-0.393336f, -0.388872f, -0.833106f)
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

	std::map<size_t, keyframe>::iterator lower_keyframe(size_t frame) {
			
		if(keyframes.empty())
			return keyframes.end();

		auto it = keyframes.lower_bound(frame);

		if(it == keyframes.end())
			return std::prev(it);
		else if(it->first == frame || it == keyframes.begin())
			return it;
		else
			return std::prev(it);
	}

	std::map<size_t, keyframe>::iterator upper_keyframe(size_t frame) {

		if(keyframes.empty())
			return keyframes.end();

		auto it = keyframes.upper_bound(frame);

		if(it == keyframes.end())
			return std::prev(it);
		return it;
	}

	bool find_tween(size_t frame, tween_data& tween) {

		auto it_lower = lower_keyframe(frame);
		auto it_upper = upper_keyframe(frame);

		if(it_lower == keyframes.end() || it_upper == keyframes.end())
			return false;

		tween.start_frame = it_lower->first;
		tween.start_key = it_lower->second;
		tween.end_frame = it_upper->first;
		tween.end_key = it_upper->second;

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

	/*bool apply(cgv::render::view* view_ptr) {

		float tc = static_cast<float>(timecode);
		size_t f = record ? frame : round(tc * elapsed_time);

		bool valid = get_tween(f);

		if(!valid) {
			run = false;
			return;
		}

		view_parameters camera_state;
		if(record)
			camera_state = current_tween.interpolate_by_frame(f);
		else
			camera_state = current_tween.interpolate_by_time(elapsed_time, tc);

		view_ptr->set_view_up_dir(camera_state.up_direction);
		view_ptr->set_focus(camera_state.focus_position);
		view_ptr->set_eye_keep_view_angle(camera_state.eye_position);

		return f < frame_count())
	}*/

	bool apply(cgv::render::view* view_ptr) {

		if(use_continuous_time)
			return apply(view_ptr, time);
		else
			return apply(view_ptr, frame);
	}

	bool apply(cgv::render::view* view_ptr, size_t frame) {

		float tc = static_cast<float>(timecode);
		
		tween_data tween;
		if(!find_tween(frame, tween))
			return false;

		set_view(view_ptr, tween.interpolate_by_frame(frame));
		
		return frame < frame_count();
	}

	bool apply(cgv::render::view* view_ptr, float time) {

		float tc = static_cast<float>(timecode);
		size_t frame = round(tc * time);

		tween_data tween;
		if(!find_tween(frame, tween))
			return false;

		set_view(view_ptr, tween.interpolate_by_time(time, tc));

		return frame < frame_count();
	}

	void set_view(cgv::render::view* view_ptr, const view_parameters& parameters) {

		view_ptr->set_view_up_dir(parameters.up_direction);
		view_ptr->set_focus(parameters.focus_position);
		view_ptr->set_eye_keep_view_angle(parameters.eye_position);
	}
};
