# General Makefile for CGV framework and standard plugins
# mspehr 04.02.09

ROOT=/home/marsl/release/
include $(ROOT)make/linux/Makefile

###################################################################
##################### TARGET DEFINITIONS ##########################
###################################################################
all_cgv: 				cgv_viewer cgv_f cg_fltk crg_spherical_view crg_view crg_planar_view cmi_io crg_grid

all_cgv_clean: 			cgv_viewer_clean cgv_clean cg_fltk_clean crg_spherical_view_clean crg_view_clean crg_planar_view_clean cmi_io_clean crg_grid_clean

all_cgv_debug:			cgv_viewer_debug cgv_debug cg_fltk_debug crg_spherical_view_debug crg_view_debug crg_planar_view_debug cmi_io_debug crg_grid_debug

all_cgv_debug_clean: 	cgv_viewer_debug_clean cgv_debug_clean cg_fltk_debug_clean crg_spherical_view_debug_clean crg_view_debug_clean crg_planar_view_debug_clean cmi_io_debug_clean crg_grid_debug_clean

##################### EXECUTABLES #################################
CGV_VIEWER=cgv_viewer
CGV_VIEWER_LIBS=-lcgv
CGV_VIEWER_LIBS_DEBUG=-lcgv_debug
CGV_VIEWER_DIR=$(ROOT)apps/cgv_viewer/

cgv_viewer: cgv_f cg_fltk
	$(call compile_std,$(CGV_VIEWER_DIR),$(CGV_VIEWER_LIBS),$(SIMPLE_TARGET_APP),$(CGV_VIEWER))
		
cgv_viewer_clean:
	$(call clean_std,$(CGV_VIEWER_DIR),$(CGV_VIEWER))

cgv_viewer_debug: cgv_debug cg_fltk_debug
	$(call compile_debug,$(CGV_VIEWER_DIR),$(CGV_VIEWER_LIBS_DEBUG),$(SIMPLE_TARGET_APP),$(CGV_VIEWER))

cgv_viewer_debug_clean:
	$(call clean_debug,$(CGV_VIEWER_DIR),$(CGV_VIEWER))

######################## PLUGINS ##################################
CG_FLTK=cg_fltk
CG_FLTK_LIBS = 	-lGL -lGLU -lcgv -lXi -lpthread \
				-lm -lXext -lsupc++ -lfltk2_gl -lfltk2_images \
				-lfltk2 -lfltk2_glut -lpng -lz -lXft -lX11 \
				-lXinerama -lfontconfig -lXrender -lfreetype
CG_FLTK_LIBS_DEBUG = 	-lGL -lGLU -lcgv_debug -lXi -lpthread \
						-lm -lXext -lsupc++ -lfltk2_gl -lfltk2_images \
						-lfltk2 -lfltk2_glut -lpng -lz -lXft -lX11 \
						-lXinerama -lfontconfig -lXrender -lfreetype
CG_FLTK_DIR=$(ROOT)plugins/cg_fltk

cg_fltk: cgv_f
	$(call compile_std,$(CG_FLTK_DIR),$(CG_FLTK_LIBS),$(SIMPLE_TARGET_LIB),$(CG_FLTK))

cg_fltk_clean:
	$(call clean_std,$(CG_FLTK_DIR),$(CG_FLTK))

cg_fltk_debug: cgv_debug
	$(call compile_debug,$(CG_FLTK_DIR),$(CG_FLTK_LIBS_DEBUG),$(SIMPLE_TARGET_LIB),$(CG_FLTK))

cg_fltk_debug_clean:
	$(call clean_debug,$(CG_FLTK_DIR),$(CG_FLTK))

#-------------------------------------------------------------#
CRG_SPHERICAL_VIEW=crg_spherical_view
CRG_SPHERICAL_VIEW_LIBS = -lGL -lGLU -lcgv
CRG_SPHERICAL_VIEW_LIBS_DEBUG = -lGL -lGLU -lcgv_debug
CRG_SPHERICAL_VIEW_DIR = $(ROOT)plugins/crg_spherical_view/

crg_spherical_view: cgv_f
	$(call compile_std,$(CRG_SPHERICAL_VIEW_DIR),$(CRG_SPHERICAL_VIEW_LIBS),$(SIMPLE_TARGET_LIB),$(CRG_SPHERICAL_VIEW))

crg_spherical_view_clean:
	$(call clean_std,$(CRG_SPHERICAL_VIEW_DIR),$(CRG_SPHERICAL_VIEW))

crg_spherical_view_debug: cgv_debug
	$(call compile_debug,$(CRG_SPHERICAL_VIEW_DIR),$(CRG_SPHERICAL_VIEW_LIBS_DEBUG),$(SIMPLE_TARGET_LIB),$(CRG_SPHERICAL_VIEW))

crg_spherical_view_debug_clean:
	$(call clean_debug,$(CRG_SPHERICAL_VIEW_DIR),$(CRG_SPHERICAL_VIEW))


#------------------------------------------------------------#
CRG_VIEW=crg_view
CRG_VIEW_LIBS = -lGL -lGLU -lcgv
CRG_VIEW_LIBS_DEBUG = -lGL -lGLU -lcgv_debug
CRG_VIEW_DIR = $(ROOT)plugins/crg_view/

crg_view: cgv_f
	$(call compile_std,$(CRG_VIEW_DIR),$(CRG_VIEW_LIBS),$(SIMPLE_TARGET_LIB),$(CRG_VIEW))

crg_view_clean:
	$(call clean_std,$(CRG_VIEW_DIR),$(CRG_VIEW))

crg_view_debug: cgv_debug
	$(call compile_debug,$(CRG_VIEW_DIR),$(CRG_VIEW_LIBS_DEBUG),$(SIMPLE_TARGET_LIB),$(CRG_VIEW))

crg_view_debug_clean:
	$(call clean_debug,$(CRG_VIEW_DIR),$(CRG_VIEW))

#------------------------------------------------------------#
CRG_PLANAR_VIEW=crg_planar_view
CRG_PLANAR_VIEW_LIBS = -lGL -lGLU -lcgv
CRG_PLANAR_VIEW_LIBS_DEBUG = -lGL -lGLU -lcgv_debug
CRG_PLANAR_VIEW_DIR = $(ROOT)plugins/crg_planar_view/

crg_planar_view: cgv_f
	$(call compile_std,$(CRG_PLANAR_VIEW_DIR),$(CRG_PLANAR_VIEW_LIBS),$(SIMPLE_TARGET_LIB),$(CRG_PLANAR_VIEW))

crg_planar_view_clean:
	$(call clean_std,$(CRG_PLANAR_VIEW_DIR),$(CRG_PLANAR_VIEW))

crg_planar_view_debug: cgv_debug
	$(call compile_debug,$(CRG_PLANAR_VIEW_DIR),$(CRG_PLANAR_VIEW_LIBS_DEBUG),$(SIMPLE_TARGET_LIB),$(CRG_PLANAR_VIEW))

crg_planar_view_debug_clean:
	$(call clean_debug,$(CRG_PLANAR_VIEW_DIR),$(CRG_PLANAR_VIEW))
	

#------------------------------------------------------------#	
CMI_IO=cmi_io
CMI_IO_LIBS = -ljpeg -ltiff -lpng -ltiffxx -lcgv
CMI_IO_LIBS_DEBUG = -ljpeg -ltiff -lpng -ltiffxx -lcgv_debug
CMI_IO_DIR = $(ROOT)plugins/cmi_io

cmi_io: cgv_f
	$(call compile_std,$(CMI_IO_DIR),$(CMI_IO_LIBS),$(SIMPLE_TARGET_LIB),$(CMI_IO))

cmi_io_clean:
	$(call clean_std,$(CMI_IO_DIR),$(CMI_IO))

cmi_io_debug: cgv_debug
	$(call compile_debug,$(CMI_IO_DIR),$(CMI_IO_LIBS_DEBUG),$(SIMPLE_TARGET_LIB),$(CMI_IO))

cmi_io_debug_clean:
	$(call clean_debug,$(CMI_IO_DIR),$(CMI_IO))

#------------------------------------------------------------#		
CRG_GRID=crg_grid
CRG_GRID_LIBS = -lGL -lGLU -lcgv
CRG_GRID_LIBS_DEBUG =  -lGL -lGLU -lcgv_debug
CRG_GRID_DIR = $(ROOT)plugins/crg_grid/

crg_grid: cgv_f
	$(call compile_std,$(CRG_GRID_DIR),$(CRG_GRID_LIBS),$(SIMPLE_TARGET_LIB),$(CRG_GRID))

crg_grid_clean:
	$(call clean_std,$(CRG_GRID_DIR),$(CRG_GRID))

crg_grid_debug: cgv_debug
	$(call compile_debug,$(CRG_GRID_DIR),$(CRG_GRID_LIBS_DEBUG),$(SIMPLE_TARGET_LIB),$(CRG_GRID))

crg_grid_debug_clean:
	$(call clean_debug,$(CRG_GRID_DIR),$(CRG_GRID))
	
	
################################ FRAMEWORK #######################################
CGV_FRAMEWORK=cgv
CGV_FRAMEWORK_LIBS = -lGL -lGLU -ldl
CGV_FRAMEWORK_LIBS_DEBUG = -lGL -lGLU -ldl
CGV_FRAMEWORK_DIR = $(ROOT)

cgv_f:
	$(call compile_std,$(CGV_FRAMEWORK_DIR),$(CGV_FRAMEWORK_LIBS), -t lib -r -nopwd cgv/ "DESTDIR=$(TARGET_LIB_DIR)"  -o,$(CGV_FRAMEWORK))

cgv_clean:
	$(call clean_std,$(CGV_FRAMEWORK_DIR),$(CGV_FRAMEWORK))

cgv_debug:
	$(call compile_debug,$(CGV_FRAMEWORK_DIR),$(CGV_FRAMEWORK_LIBS_DEBUG), -t lib -r -nopwd cgv/ "DESTDIR=$(TARGET_LIB_DIR)"  -o,$(CGV_FRAMEWORK))

cgv_debug_clean:
	$(call clean_debug,$(CGV_FRAMEWORK_DIR),$(CGV_FRAMEWORK))

	
	
	
	
	
	
	
	
	
	
	
	