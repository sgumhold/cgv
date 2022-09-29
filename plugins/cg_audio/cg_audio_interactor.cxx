#include "cg_audio_interactor.h"

#include <algorithm>

#include <libs/cgv_oal/al_context.h>

AudioInteractor::AudioInteractor()
{
	
}

void AudioInteractor::create_menu() {
	const auto device_name_strings = cgv::audio::OALContext::enumerate_devices();
	std::transform(std::cbegin(device_name_strings), std::cend(device_name_strings), std::back_inserter(menu_btn_ptrs),
				   [this](const std::string& dev_name) {
					   constexpr auto menu_prefix = "Devices/";
					   return add_menu_button(menu_prefix + dev_name);
		});
}

void AudioInteractor::destroy_menu() {
	for (const auto& btn : menu_btn_ptrs) {
		remove_menu_element(btn);
	}
}
