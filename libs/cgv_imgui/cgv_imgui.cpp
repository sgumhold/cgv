#include "cgv_imgui.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <cgv/gui/key_event.h>
#include <cgv/utils/file.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/trigger.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace imgui {

cgv_imgui::cgv_imgui(bool _use_offline_rendering, const char* _ttf_font_file_name, float _ttf_font_size) 
    : tex("uint16[R,G,B,A]")
{
    if (_ttf_font_file_name)
        ttf_font_file_name = _ttf_font_file_name;
    ttf_font_size = _ttf_font_size;
	width = 0;
	height = 0;
    font_atlas = 0;
    use_offline_rendering = _use_offline_rendering;
    has_focus = false;
    cursor_x = -1;
    cursor_y = -1;
    show_demo_window = true;
    show_another_window = true;
    clear_color = rgba(0.0f, 0.0f, 0.0f, 0.0f);
    tex.set_mag_filter(cgv::render::TF_LINEAR);
    tex.set_min_filter(cgv::render::TF_LINEAR);
    mouse_down[0] = mouse_down[1] = mouse_down[2] = false;
}

/// destructor
cgv_imgui::~cgv_imgui()
{
    if (font_atlas) {
        ImFontAtlas* atlas_ptr = reinterpret_cast<ImFontAtlas*>(font_atlas);
        delete atlas_ptr;
    }
}

void cgv_imgui::resize(unsigned int w, unsigned int h)
{
	width = w;
	height = h;
	post_redraw();
}

void cgv_imgui::clear(cgv::render::context& ctx)
{

}

/// construct offline render target
bool cgv_imgui::init(cgv::render::context& ctx)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
    ImFontAtlas* atlas_ptr = 0;
    if (!ttf_font_file_name.empty() && cgv::utils::file::exists(ttf_font_file_name)) {
        atlas_ptr = new ImFontAtlas();
        atlas_ptr->AddFontFromFileTTF(ttf_font_file_name.c_str(), ttf_font_size);
    }
	ImGui::CreateContext(atlas_ptr);
	ImGuiIO& io = ImGui::GetIO(); 
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	//io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
	io.BackendPlatformName = "cgv_imgui";
															  
	// setup keys
	io.KeyMap[ImGuiKey_Tab] = cgv::gui::KEY_Tab;
	io.KeyMap[ImGuiKey_LeftArrow] = cgv::gui::KEY_Left;
	io.KeyMap[ImGuiKey_RightArrow] = cgv::gui::KEY_Right;
	io.KeyMap[ImGuiKey_UpArrow] = cgv::gui::KEY_Up;
	io.KeyMap[ImGuiKey_DownArrow] = cgv::gui::KEY_Down;
	io.KeyMap[ImGuiKey_PageUp] = cgv::gui::KEY_Page_Up;
	io.KeyMap[ImGuiKey_PageDown] = cgv::gui::KEY_Page_Down;
	io.KeyMap[ImGuiKey_Home] = cgv::gui::KEY_Home;
	io.KeyMap[ImGuiKey_End] = cgv::gui::KEY_End;
	io.KeyMap[ImGuiKey_Insert] = cgv::gui::KEY_Insert;
	io.KeyMap[ImGuiKey_Delete] = cgv::gui::KEY_Delete;
	io.KeyMap[ImGuiKey_Backspace] = cgv::gui::KEY_Back_Space;
	io.KeyMap[ImGuiKey_Space] = cgv::gui::KEY_Space;
	io.KeyMap[ImGuiKey_Enter] = cgv::gui::KEY_Enter;
	io.KeyMap[ImGuiKey_Escape] = cgv::gui::KEY_Escape;
	io.KeyMap[ImGuiKey_KeyPadEnter] = cgv::gui::KEY_Num_Enter;
	io.KeyMap[ImGuiKey_A] = int('A');
	io.KeyMap[ImGuiKey_C] = int('C');
	io.KeyMap[ImGuiKey_V] = int('V');
	io.KeyMap[ImGuiKey_X] = int('X');
	io.KeyMap[ImGuiKey_Y] = int('Y');
	io.KeyMap[ImGuiKey_Z] = int('Z');

	ImGui_ImplOpenGL3_Init("#version 130");

	return true;
}

/// overload to create ui
void cgv_imgui::create_imgui()
{
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit4("clear color", &clear_color[0]);      // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
}

void cgv_imgui::prepare_imgui_draw_data(int w, int h, float scale_x, float scale_y)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");
    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2(1.0f,1.0f);

    // Setup time step
    std::chrono::high_resolution_clock::time_point current_time =
        std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> D = std::chrono::duration_cast<std::chrono::duration<double>>(current_time - g_Time);
    io.DeltaTime = float(D.count());
    if (io.DeltaTime <= 0.0)
        io.DeltaTime = (float)(1.0f / 60.0f);
    g_Time = current_time;
    ImGui::NewFrame();
    create_imgui();
    ImGui::Render();
}

void cgv_imgui::draw_imgui_data(cgv::render::context& ctx, bool do_clear)
{
    auto is_depth = glIsEnabled(GL_DEPTH_TEST);
    auto is_blend = glIsEnabled(GL_BLEND);
    ctx.set_color(rgb(1, 1, 1));
    ctx.push_window_transformation_array();
    ctx.set_viewport(ivec4(0, 0, width, height));
    ctx.push_pixel_coords();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GLfloat clr_col[4];
    if (do_clear) {
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clr_col);
        glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (is_depth)
        glEnable(GL_DEPTH_TEST);
    if (!is_blend)
        glDisable(GL_BLEND);
    if (do_clear)
        glClearColor(clr_col[0], clr_col[1], clr_col[2], clr_col[3]);
    ctx.pop_pixel_coords();
    ctx.pop_window_transformation_array();
}

/// evaluate user interface
void cgv_imgui::init_frame(cgv::render::context& ctx)
{
    prepare_imgui_draw_data(ctx.get_width(), ctx.get_height());

    if (!use_offline_rendering) {
        if (fbo.is_created())
            fbo.destruct(ctx);
        if (tex.is_created())
            tex.destruct(ctx);
        return;
    }
	if (!tex.is_created() || tex.get_width() != width || tex.get_height() != height) {
		tex.destruct(ctx);
		tex.create(ctx, cgv::render::TT_2D, width, height);
		fbo.destruct(ctx);
		fbo.create(ctx, width, height);
		fbo.attach(ctx, tex);
	}
	if (!fbo.is_created())
		return;
   
    fbo.enable(ctx, 0);
    draw_imgui_data(ctx);
    fbo.disable(ctx);
    tex.write_to_file(ctx, "d:\\tex.png");
}

/// draw user interface
void cgv_imgui::after_finish(cgv::render::context& ctx)
{
    if (use_offline_rendering) {
        auto is_blend = glIsEnabled(GL_BLEND);
        auto is_depth = glIsEnabled(GL_DEPTH_TEST);
        ctx.set_color(rgba(1, 1, 1, 1));
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        tex.enable(ctx);
        float gamma = ctx.get_gamma();
        ctx.set_gamma(1.0f);
        cgv::render::gl::cover_screen(ctx, &ctx.ref_default_shader_program(true));
        ctx.set_gamma(gamma);
        tex.disable(ctx);
        if (!is_blend)
            glDisable(GL_BLEND);
        if (is_depth)
            glEnable(GL_DEPTH_TEST);
    }
    else {
        draw_imgui_data(ctx, false);
    }
}

/// translate events to imgui
bool cgv_imgui::handle(cgv::gui::event& e)
{
    ImGuiIO& io = ImGui::GetIO();
    if (e.get_kind() == cgv::gui::EID_KEY) {
        auto& ke = reinterpret_cast<cgv::gui::key_event&>(e);
        switch (ke.get_action()) {
        case cgv::gui::KA_PRESS:
            io.KeysDown[ke.get_key()] = true;
            break;
        case cgv::gui::KA_RELEASE:
            io.KeysDown[ke.get_key()] = false;
            break;
        }
        // Modifiers are not reliable across systems
        io.KeyCtrl = io.KeysDown[cgv::gui::KEY_Left_Ctrl] || io.KeysDown[cgv::gui::KEY_Right_Ctrl];
        io.KeyShift = io.KeysDown[cgv::gui::KEY_Left_Shift] || io.KeysDown[cgv::gui::KEY_Right_Shift];
        io.KeyAlt = io.KeysDown[cgv::gui::KEY_Left_Alt] || io.KeysDown[cgv::gui::KEY_Right_Alt];
        io.KeySuper = io.KeysDown[cgv::gui::KEY_Left_Meta] || io.KeysDown[cgv::gui::KEY_Right_Meta];

        if (ke.get_char() != 0)
            io.AddInputCharacter(ke.get_char());
        if (io.WantCaptureKeyboard)
            return true;
    }
    if (e.get_kind() == cgv::gui::EID_MOUSE) {
        auto& me = reinterpret_cast<cgv::gui::mouse_event&>(e);
        switch (me.get_action()) {
        case cgv::gui::MA_ENTER:
            has_focus = true;
            break;
        case cgv::gui::MA_LEAVE:
            has_focus = false;
            break;
        case cgv::gui::MA_WHEEL:
        {
            ImGuiIO& io = ImGui::GetIO();
            io.MouseWheelH += (float)me.get_dx();
            io.MouseWheel += (float)me.get_dy();
            break;
        }
        case cgv::gui::MA_MOVE:
        case cgv::gui::MA_DRAG:
            cursor_x = me.get_x();
            cursor_y = me.get_y();
            break;
        case cgv::gui::MA_PRESS:
            switch (me.get_button()) {
            case cgv::gui::MB_LEFT_BUTTON:   mouse_down[0] = true; break;
            case cgv::gui::MB_RIGHT_BUTTON:  mouse_down[1] = true; break;
            case cgv::gui::MB_MIDDLE_BUTTON: mouse_down[2] = true; break;
            default: break;
            }
            break;
        case cgv::gui::MA_RELEASE:
            switch (me.get_button()) {
            case cgv::gui::MB_LEFT_BUTTON:   mouse_down[0] = false; break;
            case cgv::gui::MB_RIGHT_BUTTON:  mouse_down[1] = false; break;
            case cgv::gui::MB_MIDDLE_BUTTON: mouse_down[2] = false; break;
            default: break;
            }
            break;
        }
        // Update buttons
        io.MouseDown[0] = mouse_down[0];
        io.MouseDown[1] = mouse_down[1];
        io.MouseDown[2] = mouse_down[2];
        // Update mouse position
        const ImVec2 mouse_pos_backup = io.MousePos;
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        if (io.WantSetMousePos)
            std::cout << "imgui wants to set mouse pos to " << (double)mouse_pos_backup.x << "|" << (double)mouse_pos_backup.y << std::endl;
        else
            io.MousePos = ImVec2((float)me.get_x(), (float)me.get_y());
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || io.WantCaptureMouse)
            return true;
    }
    return false;
}

	}
}