cmake_minimum_required(VERSION 2.6)
project(FLTK2)

# Include this file for standard build settings
include("../../cmake/buildCoreComponents.cmake")


cgv_find_package(OpenGL)
cgv_find_package(JPEG)
cgv_find_package(PNG)
cgv_find_package(GLEW)

if (unix)
	cgv_find_package(X11)
endif()

# FIXME: the JPEG includes are a hack, because the libraries
# which are found by our find file will not set an include path
# to the actual library but to the parent library.
# This is due to the fact that most 3rd-party-libraries are
# included using #include <libname/libfile>. 
# On the other hand it is not currently possible to also add
# the library path itself as FLTK2 has a math.h in that path
# that would be preferred then, leading to undefined mathematical
# operations. ouch!

include_directories(
	${JPEG_INCLUDE_DIR}
	${JPEG_INCLUDE_DIR}/jpeg
	${OPENGL_INCLUDE_DIR}
	${X11_INCLUDE_DIR}
	${CGV_DIR}/3rd
	${CGV_DIR}/3rd/fltk/visualc
	${CGV_DIR}/3rd/fltk/compat
)	


set(HEADERS
	include/fltk/Adjuster.h
	include/fltk/AlignGroup.h
	include/fltk/AnsiWidget.h
	include/fltk/ask.h
	include/fltk/BarGroup.h
	include/fltk/Box.h
	include/fltk/Browser.h
	include/fltk/Button.h
	include/fltk/CheckButton.h
	include/fltk/Choice.h
	include/fltk/Clock.h
	include/fltk/ColorChooser.h
	include/fltk/Color.h
	include/fltk/ComboBox.h
	include/fltk/Cursor.h
	include/fltk/CycleButton.h
	include/fltk/damage.h
	include/fltk/Dial.h
	include/fltk/dirent.h
	include/fltk/Divider.h
	include/fltk/DoubleBufferWindow.h
	include/fltk/draw.h
	include/fltk/error.h
	include/fltk/events.h
	include/fltk/FileBrowser.h
	include/fltk/FileChooser.h
	include/fltk/file_chooser.h
	include/fltk/FileIcon.h
	include/fltk/FileInput.h
	include/fltk/filename.h
	include/fltk/FillDial.h
	include/fltk/FillSlider.h
	include/fltk/Flags.h
	include/fltk/FL_API.h
	include/fltk/FloatInput.h
	include/fltk/fltk_cairo.h
	include/fltk/FL_VERSION.h
	include/fltk/Font.h
	include/fltk/forms.h
	include/fltk/gl2opengl.h
	include/fltk/gl.h
	include/fltk/glut.h
	include/fltk/GlWindow.h
	include/fltk/Group.h
	include/fltk/HelpDialog.h
	include/fltk/HelpView.h
	include/fltk/HighlightButton.h
	include/fltk/Image.h
	include/fltk/InputBrowser.h
	include/fltk/Input.h
	include/fltk/IntInput.h
	include/fltk/InvisibleBox.h
	include/fltk/ItemGroup.h
	include/fltk/Item.h
	include/fltk/LabelType.h
	include/fltk/layout.h
	include/fltk/LightButton.h
	include/fltk/LineDial.h
	include/fltk/load_plugin.h
	include/fltk/math.h
	include/fltk/MenuBar.h
	include/fltk/MenuBuild.h
	include/fltk/Menu.h
	include/fltk/MenuWindow.h
	include/fltk/Monitor.h
	include/fltk/MultiBrowser.h
	include/fltk/MultiImage.h
	include/fltk/MultiLineInput.h
	include/fltk/MultiLineOutput.h
	include/fltk/NumericInput.h
	include/fltk/osx.h
	include/fltk/Output.h
	include/fltk/PackedGroup.h
	include/fltk/PixelType.h
	include/fltk/pnmImage.h
	include/fltk/PopupMenu.h
	include/fltk/Preferences.h
	include/fltk/ProgressBar.h
	include/fltk/RadioButton.h
	include/fltk/RadioItem.h
	include/fltk/RadioLightButton.h
	include/fltk/Rectangle.h
	include/fltk/RepeatButton.h
	include/fltk/ReturnButton.h
	include/fltk/rgbImage.h
	include/fltk/run.h
	include/fltk/Scrollbar.h
	include/fltk/ScrollGroup.h
	include/fltk/SecretInput.h
	include/fltk/ShapedWindow.h
	include/fltk/SharedImage.h
	include/fltk/show_colormap.h
	include/fltk/Slider.h
	include/fltk/StatusBarGroup.h
	include/fltk/string.h
	include/fltk/string.h.in
	include/fltk/StringList.h
	include/fltk/Style.h
	include/fltk/StyleSet.h
	include/fltk/Symbol.h
	include/fltk/SystemMenuBar.h
	include/fltk/TabGroup.h
	include/fltk/TextBuffer.h
	include/fltk/TextDisplay.h
	include/fltk/TextEditor.h
	include/fltk/Threads.h
	include/fltk/ThumbWheel.h
	include/fltk/TiledGroup.h
	include/fltk/TiledImage.h
	include/fltk/ToggleButton.h
	include/fltk/ToggleItem.h
	include/fltk/Tooltip.h
	include/fltk/utf.h
	include/fltk/Valuator.h
	include/fltk/ValueInput.h
	include/fltk/ValueOutput.h
	include/fltk/ValueSlider.h
	include/fltk/visual.h
	include/fltk/WidgetAssociation.h
	include/fltk/Widget.h
	include/fltk/win32.h
	include/fltk/Window.h
	include/fltk/WizardGroup.h
	include/fltk/WordwrapInput.h
	include/fltk/WordwrapOutput.h
	include/fltk/x11.h
	include/fltk/xbmImage.h
	include/fltk/x.h
	include/fltk/xpmImage.h
)

set(COMPAT_HEADERS
	compat/FL/Enumerations.H
	compat/FL/filename.H
	compat/FL/Fl_Adjuster.H
	compat/FL/fl_ask.H
	compat/FL/Fl_Box.H
	compat/FL/Fl_Browser.H
	compat/FL/Fl_Button.H
	compat/FL/Fl_Chart.H
	compat/FL/Fl_Check_Button.H
	compat/FL/Fl_Choice.H
	compat/FL/Fl_Clock.H
	compat/FL/Fl_Color_Chooser.H
	compat/FL/Fl_Counter.H
	compat/FL/Fl_Dial.H
	compat/FL/Fl_Double_Window.H
	compat/FL/fl_draw.H
	compat/FL/Fl_File_Chooser.H
	compat/FL/Fl_Gl_Window.H
	compat/FL/Fl_Group.H
	compat/FL/Fl.H
	compat/FL/Fl_Hold_Browser.H
	compat/FL/Fl_Hor_Slider.H
	compat/FL/Fl_Hor_Value_Slider.H
	compat/FL/Fl_Input.H
	compat/FL/Fl_Int_Input.H
	compat/FL/Fl_Light_Button.H
	compat/FL/Fl_Menu_Bar.H
	compat/FL/Fl_Menu_Button.H
	compat/FL/Fl_Menu_.H
	compat/FL/Fl_Menu_Item.H
	compat/FL/fl_message.H
	compat/FL/Fl_Output.H
	compat/FL/Fl_Overlay_Window.H
	compat/FL/Fl_Pack.H
	compat/FL/Fl_Pixmap.H
	compat/FL/Fl_Return_Button.H
	compat/FL/Fl_Roller.H
	compat/FL/Fl_Scrollbar.H
	compat/FL/Fl_Scroll.H
	compat/FL/Fl_Select_Browser.H
	compat/FL/Fl_Shared_Image.H
	compat/FL/Fl_Single_Window.H
	compat/FL/Fl_Slider.H
	compat/FL/Fl_Tabs.H
	compat/FL/Fl_Text_Buffer.H
	compat/FL/Fl_Text_Editor.H
	compat/FL/Fl_Tile.H
	compat/FL/Fl_Toggle_Button.H
	compat/FL/Fl_Toggle_Light_Button.H
	compat/FL/Fl_Valuator.H
	compat/FL/Fl_Value_Input.H
	compat/FL/Fl_Value_Output.H
	compat/FL/Fl_Value_Slider.H
	compat/FL/Fl_Widget.H
	compat/FL/Fl_Window.H
	compat/FL/forms.H
	compat/FL/gl.h
	compat/FL/glut.H
	compat/FL/math.h
	compat/FL/menubar.h
	compat/FL/x.H)

set(IMAGES_SOURCES
	images/FileIcon2.cxx
	images/Fl_Guess_Image.cxx
	images/fl_jpeg.cxx
	images/fl_png.cxx
	images/HelpDialog.cxx
	images/HelpDialog.fl
	images/images_core.cxx
	images/Makefile
	images/pnmImage.cxx
	images/xpmFileImage.cxx)


set(OPENGL_SOURCES
	OpenGL/Fl_Gl_Choice.cxx
	OpenGL/Fl_Gl_Overlay.cxx
	OpenGL/Fl_Gl_Window.cxx
	OpenGL/GlChoice.h
	OpenGL/gl_draw.cxx
	OpenGL/gl_start.cxx)

set(SOURCES
	src/addarc.cxx
	src/addcurve.cxx
	src/add_idle.cxx
	src/Adjuster.cxx
	src/AlignGroup.cxx
	src/allfiles.xbm
	src/AnsiWidget.cxx
	src/args.cxx
	src/ARRAY.h
	src/BarGroup.cxx
	src/bmpImage.cxx
	src/Browser.cxx
	src/Browser_load.cxx
	src/Button.cxx
	src/CheckButton.cxx
	src/Choice.cxx
	src/clip.cxx
	src/Clock.cxx
	src/color_chooser.cxx
	src/Color.cxx
	src/colormap.h
	src/ComboBox.cxx
	src/compose.cxx
	src/Cursor.cxx
	src/CycleButton.cxx
	src/default_glyph.cxx
	src/Dial.cxx
	src/DiamondBox.cxx
	src/dnd.cxx
	src/drawtext.cxx
	src/EngravedLabel.cxx
	src/error.cxx
	src/event_key_state.cxx
	src/fastarrow.h
	src/FileBrowser.cxx
	src/FileChooser2.cxx
	src/FileChooser.cxx
	src/file_chooser.cxx
	src/FileIcon.cxx
	src/FileInput.cxx
	src/filename_absolute.cxx
	src/filename_ext.cxx
	src/filename_isdir.cxx
	src/filename_list.cxx
	src/filename_match.cxx
	src/filename_name.cxx
	src/fillrect.cxx
	src/Fl_Menu_Item.cxx
	src/FloatInput.cxx
	src/fltk_theme.cxx
	src/Font.cxx
	src/gifImage.cxx
	src/Group.cxx
	src/GSave.cxx
	src/HelpView.cxx
	src/HighlightButton.cxx
	src/Image.cxx
	src/InputBrowser.cxx
	src/Input.cxx
	src/InvisibleWidget.cxx
	src/Item.cxx
	src/key_name.cxx
	src/LightButton.cxx
	src/list_fonts.cxx
	src/load_plugin.cxx
	src/lock.cxx
	src/mediumarrow.h
	src/Menu_add.cxx
	src/MenuBar.cxx
	src/Menu.cxx
	src/Menu_global.cxx
	src/Menu_popup.cxx
	src/MenuWindow.cxx
	src/message.cxx
	src/MultiImage.cxx
	src/NumericInput.cxx
	src/numericsort.cxx
	src/Output.cxx
	src/OvalBox.cxx
	src/overlay_rect.cxx
	src/own_colormap.cxx
	src/PackedGroup.cxx
	src/path.cxx
	src/PlasticBox.cxx
	src/PopupMenu.cxx
	src/Preferences.cxx
	src/ProgressBar.cxx
	src/RadioButton.cxx
	src/readimage.cxx
	src/RepeatButton.cxx
	src/ReturnButton.cxx
	src/RoundBox.cxx
	src/RoundedBox.cxx
	src/run.cxx
	src/scandir.c
	src/Scrollbar.cxx
	src/ScrollGroup.cxx
	src/scrollrect.cxx
	src/setcolor.cxx
	src/setdisplay.cxx
	src/setvisual.cxx
	src/ShadowBox.cxx
	src/ShapedWindow.cxx
	src/SharedImage.cxx
	src/ShortcutAssignment.cxx
	src/show_colormap.cxx
	src/Slider.cxx
	src/slowarrow.h
	src/StatusBarGroup.cxx
	src/string.c
	src/StringList.cxx
	src/Style.cxx
	src/StyleSet.cxx
	src/Symbol.cxx
	src/SystemMenuBar.cxx
	src/TabGroup2.cxx
	src/TabGroup.cxx
	src/TextBuffer.cxx
	src/TextDisplay.cxx
	src/TextEditor.cxx
	src/ThumbWheel.cxx
	src/TiledGroup.cxx
	src/TiledImage.cxx
	src/Tooltip.cxx
	src/UpBox.cxx
	src/utf.c
	src/Valuator.cxx
	src/ValueInput.cxx
	src/ValueOutput.cxx
	src/ValueSlider.cxx
	src/vsnprintf.c
	src/WidgetAssociation.cxx
	src/Widget.cxx
	src/Widget_draw.cxx
	src/Window.cxx
	src/Window_fullscreen.cxx
	src/Window_hotspot.cxx
	src/Window_iconize.cxx
	src/WizardGroup.cxx
	src/xbmImage.cxx
	src/xpmImage.cxx)

set(CONFIG_HEADERS
	visualc/config.h)

if (WIN32)
	set(ADD_DEFINITIONS "_WIN32" "WIN32_LEAN_AND_MEAN" "VC_EXTRA_LEAN" "WIN32_EXTRA_LEAN")
else()
	set(ADD_DEFINITIONS "")
endif()

include_directories(include 
	${GLEW_INCLUDE_DIRS}
)
	
cgv_add_3rd_library(FLTK2
	SOURCES ${SOURCES} ${OPENGL_SOURCES} ${IMAGES_SOURCES}
	PUBLIC_HEADERS ${HEADERS} ${COMPAT_HEADERS} ${CONFIG_HEADERS}
	DEFINITIONS "USE_GLEW" ${ADD_DEFINITIONS}
	SHARED_DEFINITIONS "FL_SHARED" "FL_LIBRARY" "FL_GL_LIBRARY" "FL_GLUT_LIBRARY" "FL_IMAGES_LIBRARY"
	INSTALL_NAME "fltk"
)
	
target_link_libraries(FLTK2
	${GLEW_LIBRARIES}
	${PNG_LIBRARIES}
	${JPEG_LIBRARIES}
	${OPENGL_LIBRARIES}
	# ${X11_Xi_LIB}
	${X11_LIBRARIES}
)
	
if (WIN32)
	target_link_libraries(FLTK2 ws2_32)
else()
	target_link_libraries(FLTK2 Xext)
endif()

cgv_write_find_file(FLTK2 
	ADDITIONAL_INCLUDES include)

# export_package_file(FLTK)
# cgv_make_package_lib_file(FLTK)

