#include <vector>

#include "Render/GLDriver.h"
#include "Profiler/profiler.h"
#include "Utility/Time.h"
#include "IO/Input.h"

#include "UI/UI.h"
#include "UI/imgui/imgui_impl_opengl3.h"

IMGUI_IMPL_API bool ImGui_ImplSDL2_Init(struct SDL_Window*);
IMGUI_IMPL_API void ImGui_ImplSDL2_Shutdown();
IMGUI_IMPL_API void ImGui_ImplSDL2_UpdateMouseCursor();


struct window_t
{
	window_t(uint32_t _id, void (*_callback)(uint32_t)):
		id(_id), callback(_callback)
	{}

	uint32_t id;
	void (*callback)(uint32_t);
};

void (*menubar)() = NULL;
std::vector<window_t> windows;


void UI::create_window(void (*callback)(uint32_t), uint32_t id)
{
	windows.emplace_back(id, callback);
}

void UI::set_menubar(void (*callback)())
{
	menubar = callback;
}

void UI::frame()
{
	MICROPROFILE_SCOPEI("UI", "frame");

	vec2 size = Input::window_size();
	ivec2 mp = Input::mouse_position(false);

	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = Time::delta_time;
	io.DisplaySize = ImVec2(size.x, size.y);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	io.MousePos = ImVec2((float)mp.x, (float)mp.y);
	io.MouseWheel += Input::wheel_scroll();
	io.MouseDown[0] = Input::button_down(Button::Left);
	io.MouseDown[1] = Input::button_down(Button::Right);
	io.MouseDown[2] = Input::button_down(Button::Middle);

	memcpy(io.KeysDown, Input::keys_down(), Key::COUNT * sizeof(bool));
	io.KeyShift = Input::key_down(Key::LeftShift);
	io.KeyCtrl = Input::key_down(Key::LeftControl);
	io.KeyAlt = Input::key_down(Key::LeftAlt);

	io.AddInputCharactersUTF8(Input::input_chars());

	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

#ifdef DEBUG
	static bool show_demo_window = false;
	if (Input::key_pressed(Key::F9)) show_demo_window = !show_demo_window;
	if (show_demo_window)	ImGui::ShowDemoWindow(&show_demo_window);
#endif

	if (menubar && ImGui::BeginMainMenuBar())
	{
		menubar();
		ImGui::EndMainMenuBar();
	}

	for (int i = 0; i < windows.size(); i++)
		windows[i].callback(windows[i].id);
}

static inline void set_style()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 0.0f;
	style->WindowBorderSize = 1.0f;
	style->FrameRounding = 2.0f;
	style->FramePadding = ImVec2(15.0f, 5.0f);
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	ImVec4 transparent(0.0f, 0.0f, 0.0f, 0.0f);

	ImVec4 grey0(0.095f, 0.095f, 0.095f, 1.00f);
	ImVec4 grey1(0.09f, 0.09f, 0.09f, 1.00f);
	ImVec4 grey2(0.12f, 0.12f, 0.12f, 1.00f);
	ImVec4 grey3(0.145f, 0.145f, 0.145f, 1.00f);
	ImVec4 grey4(0.263f, 0.263f, 0.263f, 1.00f);
	ImVec4 grey5(0.54f, 0.54f, 0.54f, 1.00f);
	ImVec4 grey6(0.80f, 0.80f, 0.83f, 1.0f);

	style->Colors[ImGuiCol_PopupBg] = grey0;

	style->Colors[ImGuiCol_WindowBg] = grey1;
	style->Colors[ImGuiCol_Border] = grey2;
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);

	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);

	style->Colors[ImGuiCol_FrameBg] = grey3;
	style->Colors[ImGuiCol_FrameBgHovered] = grey4;
	style->Colors[ImGuiCol_FrameBgActive] = grey5;

	style->Colors[ImGuiCol_Button] = grey3;
	style->Colors[ImGuiCol_ButtonHovered] = grey4;
	style->Colors[ImGuiCol_ButtonActive] = grey5;

	style->Colors[ImGuiCol_Header] = transparent;
	style->Colors[ImGuiCol_HeaderHovered] = grey3;
	style->Colors[ImGuiCol_HeaderActive] = grey0;

	style->Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.82f, 0.82f, 0.82f, 0.39f);
	style->Colors[ImGuiCol_CheckMark] = grey6;
	style->Colors[ImGuiCol_SliderGrab] = grey6;
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);

	style->Colors[ImGuiCol_MenuBarBg] = grey3;
	style->Colors[ImGuiCol_ScrollbarBg] = grey2;
	style->Colors[ImGuiCol_ScrollbarGrab] = grey5;
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = grey5;
	style->Colors[ImGuiCol_ScrollbarGrabActive] = grey4;

	style->Colors[ImGuiCol_Tab] = grey1;
	style->Colors[ImGuiCol_TabHovered] = grey4;
	style->Colors[ImGuiCol_TabActive] = grey3;

	style->Colors[ImGuiCol_ResizeGrip] = transparent;
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

static inline void set_keymap()
{
	ImGuiIO& io = ImGui::GetIO();

	// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
	io.KeyMap[ImGuiKey_Tab] = Key::Tab;
	io.KeyMap[ImGuiKey_LeftArrow] = Key::Left;
	io.KeyMap[ImGuiKey_RightArrow] = Key::Right;
	io.KeyMap[ImGuiKey_UpArrow] = Key::Up;
	io.KeyMap[ImGuiKey_DownArrow] = Key::Down;
	io.KeyMap[ImGuiKey_PageUp] = Key::PageUp;
	io.KeyMap[ImGuiKey_PageDown] = Key::PageDown;
	io.KeyMap[ImGuiKey_Home] = Key::Home;
	io.KeyMap[ImGuiKey_End] = Key::End;
	io.KeyMap[ImGuiKey_Insert] = Key::Insert;
	io.KeyMap[ImGuiKey_Delete] = Key::Delete;
	io.KeyMap[ImGuiKey_Backspace] = Key::Backspace;
	io.KeyMap[ImGuiKey_Space] = Key::Space;
	io.KeyMap[ImGuiKey_Enter] = Key::Enter;
	io.KeyMap[ImGuiKey_Escape] = Key::Escape;
	io.KeyMap[ImGuiKey_KeyPadEnter] = Key::Enter;
	io.KeyMap[ImGuiKey_A] = Key::A;
	io.KeyMap[ImGuiKey_C] = Key::C;
	io.KeyMap[ImGuiKey_V] = Key::V;
	io.KeyMap[ImGuiKey_X] = Key::X;
	io.KeyMap[ImGuiKey_Y] = Key::Y;
	io.KeyMap[ImGuiKey_Z] = Key::Z;
}

void UI::init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.Fonts->AddFontFromFileTTF("Assets/fonts/Ruda-Bold.ttf", 14);

	set_style();
	set_keymap();

	ImGui_ImplSDL2_Init(Input::window());
	ImGui_ImplOpenGL3_Init("#version 410");
}

void UI::destroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void UI::flush()
{
	MICROPROFILE_SCOPEI("RENDER_ENGINE", "UI");
	MICROPROFILE_SCOPEGPUI("imgui", -1);

	GL::BindFramebuffer(0);

	ImGui::Render();
	ImGui_ImplSDL2_UpdateMouseCursor();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
