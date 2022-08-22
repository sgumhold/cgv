add_library(cg_audio
  waveform.h
  oal_audio_server.h
  oal_audio_server.cxx
  cg_audio_interactor.h
  cg_audio_interactor.cxx
)

target_compile_features(cg_audio PUBLIC cxx_std_17)
set_target_properties(cg_audio PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(cg_audio PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(cg_audio cgv_base cgv_gui cgv_oal)

install(TARGETS cg_audio EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})