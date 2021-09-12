cmake_minimum_required(VERSION 3.15)

project(fltk)

set(SOURCES
        # Image
        images/FileIcon2.cxx
        images/Fl_Guess_Image.cxx
        images/fl_jpeg.cxx
        images/fl_png.cxx
        images/HelpDialog.cxx
        images/HelpDialog.fl
        images/images_core.cxx
        images/Makefile
        images/pnmImage.cxx
        images/xpmFileImage.cxx

        # OpenGL
        OpenGL/Fl_Gl_Choice.cxx
        OpenGL/Fl_Gl_Overlay.cxx
        OpenGL/Fl_Gl_Window.cxx
        OpenGL/GlChoice.h
        OpenGL/gl_draw.cxx
        OpenGL/gl_start.cxx

        # Sources
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

install(DIRECTORY include/. DESTINATION ${CGV_3RD_INCLUDE_DEST} FILES_MATCHING PATTERN "*.h")

# Shared Library
add_library(fltk SHARED ${SOURCES})
target_link_libraries(fltk
        jpeg
        png
        glew
        ${OPENGL_LIBRARIES}
        ${X11_LIBRARIES})

if (WIN32)
    set(ADD_DEFINITIONS "_WIN32" "WIN32_LEAN_AND_MEAN" "VC_EXTRA_LEAN" "WIN32_EXTRA_LEAN")
    target_link_libraries(fltk ws2_32)
else ()
    target_link_libraries(fltk Xext)
endif ()

if (MSVC)
    target_compile_options(fltk PRIVATE /w)
else ()
    target_compile_options(fltk PRIVATE -w -Wno-narrowing)
endif ()

target_compile_definitions(fltk PRIVATE
        ${ADD_DEFINITIONS}
        USE_GLEW
        FL_LIBRARY
        FL_GL_LIBRARY
        FL_GLUT_LIBRARY
        FL_IMAGES_LIBRARY)
target_compile_definitions(fltk PUBLIC FL_SHARED)

target_include_directories(fltk PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/visualc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compat>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)

install(TARGETS fltk EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})

# Static Library
add_library(fltk_static STATIC ${SOURCES})
target_link_libraries(fltk_static
        jpeg_static
        png_static
        glew_static
        ${OPENGL_LIBRARIES}
        ${X11_LIBRARIES})

if (WIN32)
    set(ADD_DEFINITIONS "_WIN32" "WIN32_LEAN_AND_MEAN" "VC_EXTRA_LEAN" "WIN32_EXTRA_LEAN")
    target_link_libraries(fltk_static ws2_32)
else ()
    target_link_libraries(fltk_static Xext)
endif ()

if (MSVC)
    target_compile_options(fltk_static PRIVATE /w)
else ()
    target_compile_options(fltk_static PRIVATE -w -Wno-narrowing)
endif ()

target_compile_definitions(fltk_static PRIVATE
        ${ADD_DEFINITIONS}
        USE_GLEW
        FL_LIBRARY
        FL_GL_LIBRARY
        FL_GLUT_LIBRARY
        FL_IMAGES_LIBRARY)

target_include_directories(fltk_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/visualc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compat>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)

install(TARGETS fltk_static EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
