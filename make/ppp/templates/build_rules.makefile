
# ppp header file rule
%.h: %.ph
	$(CGV_DIR)/bin/ppp $< $@

# ppp refelection header rule
%.tdh %.tih  %_info.cxx : %.hh
	$(CGV_DIR)/bin/ppp $<  $(CGV_DIR)/cgv/type/info/type_description_h.tm $(basename $<).tdh \
	$(CGV_DIR)/cgv/type/info/type_interface_h.tm 	$(basename $<).tih \
	$(CGV_DIR)/cgv/type/info/type_interface_cxx.tm 	$(basename $<)_info.cxx


# shader test rules
#
# create_context must be implemented in linux
#
%.log : %.glvs
	$(CGV_DIR)/bin/shader_test $< $(PROJECT_OUTPUT_DIR)/shader/$(notdir $<).log
%.log : %.glfs
	$(CGV_DIR)/bin/shader_test $< $(PROJECT_OUTPUT_DIR)/shader/$(notdir $<).log
%.log : %.glpr
	$(CGV_DIR)/bin/shader_test $< $(PROJECT_OUTPUT_DIR)/shader/$(notdir $<).log
%.log : %.glsl
	$(CGV_DIR)/bin/shader_test $< $(PROJECT_OUTPUT_DIR)/shader/$(notdir $<).log


# res_prep rules 
%.cxx : %.bmp
	$(CGV_DIR)/bin/res_prep $< $(PROJECT_OUTPUT_DIR)/res_prep/$(notdir $<).cxx
%.cxx : %.jpg
	$(CGV_DIR)/bin/res_prep $< $(PROJECT_OUTPUT_DIR)/res_prep/$(notdir $<).cxx
%.cxx : %.jpeg
	$(CGV_DIR)/bin/res_prep $< $(PROJECT_OUTPUT_DIR)/res_prep/$(notdir $<).cxx
%.cxx : %.png
	$(CGV_DIR)/bin/res_prep $< $(PROJECT_OUTPUT_DIR)/res_prep/$(notdir $<).cxx
%.cxx : %.tif
	$(CGV_DIR)/bin/res_prep $< $(PROJECT_OUTPUT_DIR)/res_prep/$(notdir $<).cxx
%.cxx : %.tiff
	$(CGV_DIR)/bin/res_prep $< $(PROJECT_OUTPUT_DIR)/res_prep/$(notdir $<).cxx

