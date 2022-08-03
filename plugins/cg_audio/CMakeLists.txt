cgv_create_lib(
  cg_audio
  SOURCES
  waveform.h
  oal_audio_server.h
  oal_audio_server.cxx
  cg_audio_interactor.h
  cg_audio_interactor.cxx
  DEPENDENCIES
  cgv_base
  cgv_gui
)

target_compile_features(cg_audio PUBLIC cxx_std_17)
target_compile_features(cg_audio_static PUBLIC cxx_std_17)

set_target_properties(cg_audio PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(cg_audio_static PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(cg_audio PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(cg_audio_static PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(cg_audio PUBLIC OpenAL::OpenAL)
target_link_libraries(cg_audio_static PUBLIC OpenAL::OpenAL)