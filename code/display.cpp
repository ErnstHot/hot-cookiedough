
// cookiedough -- SDL software display window (32-bit)

#include "main.h"
#include "display.h"

Display::Display() :
	m_window(nullptr),
	m_renderer(nullptr),
	m_texture(nullptr)
{
	// smooth scaling (if necessary)
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

Display::~Display()
{
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	SDL_DestroyTexture(m_texture);
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
} 

bool Display::Open(const std::string &title, unsigned int xRes, unsigned int yRes, bool fullScreen)
{
	const int result = (false == fullScreen)
		? SDL_CreateWindowAndRenderer(xRes, yRes, SDL_RENDERER_ACCELERATED, &m_window, &m_renderer)
		: SDL_CreateWindowAndRenderer(0, 0, int(SDL_WINDOW_FULLSCREEN_DESKTOP) | int(SDL_RENDERER_ACCELERATED), &m_window, &m_renderer);
	if (-1 != result)
	{
		// set output resolution (regardless of what we're blitting to)
		SDL_RenderSetLogicalSize(m_renderer, xRes, yRes);

		// set title
		SDL_SetWindowTitle(m_window, title.c_str());

		m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, xRes, yRes);
		m_pitch = xRes*4;

		// Set up ImGui
		if (!kFullScreen)
		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImPlot::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigWindowsMoveFromTitleBarOnly = true;

			ImGuiStyle& style				= ImGui::GetStyle();
			style.WindowPadding.x			= 4;
			style.WindowPadding.y			= 4;
			style.WindowRounding			= 4;
			style.ItemSpacing.y				= 5;
			style.ItemInnerSpacing.x		= 6;
			style.WindowRounding			= 2;
			style.FrameRounding				= 2;
			style.GrabMinSize				= 10;
			style.GrabRounding				= 2;
			style.WindowMenuButtonPosition	= 1;

			ImVec4* colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
			colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.03f, 0.03f, 0.03f, 0.54f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.67f, 0.67f, 0.67f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
			colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
			colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
			colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			//colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
			//colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotLines] = ImVec4(0.6f, 0.6f, 0.6f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.7f, 0.7f, 0.7f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.6f, 0.6f, 0.6f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.7f, 0.7f, 0.7f, 1.00f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
			colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
			colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

			ImGui_ImplSDL2_InitForSDLRenderer(m_window, m_renderer);
			ImGui_ImplSDLRenderer2_Init(m_renderer);
		}

		if (nullptr != m_texture)
		{
			return true;
		}
	}

	const std::string SDLerror = SDL_GetError();
	SetLastError("Can't open SDL display window: " + SDLerror);

	return false;
}

void Display::Update(const uint32_t *pPixels)
{
	SDL_RenderClear(m_renderer);

	if (nullptr != pPixels)
	{
		SDL_UpdateTexture(m_texture, nullptr, pPixels, m_pitch);
		SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
	}

	if (!kFullScreen)
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

	SDL_RenderPresent(m_renderer);
}
