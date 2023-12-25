#include "gui.h"
#include "main.h"

#if defined(GUI_ENABLED)
	// Constants
	constexpr ImVec4	kGreyTextColor				= ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	constexpr ImVec4	kFrameRateWarnColor			= ImVec4(0.90f, 0.20f, 0.20f, 1.00f);

	constexpr ImGuiKey	kGui_Keys_ToggleShowGui		= ImGuiKey_F2;
	constexpr ImGuiKey	kGui_Keys_ToggleShowInfo	= ImGuiKey_F3;

	static bool s_guiIsVisible = kGui_VisibleByDefault;
	static bool s_infoWindowIsOpen = kGui_InfoWindowDefaultOpen;
#endif

bool Gui_Is_Visible()
{
#if defined(GUI_ENABLED)
	return s_guiIsVisible && kGuiEnabled;
#endif // GUI_ENABLED

	return false;
}

#if defined (GUI_ENABLED)
	#include "../3rdparty/imgui-1.90/imgui_internal.h"
	#include "../3rdparty/imgui-1.90/imgui_impl_sdl2.h"
	#include "../3rdparty/imgui-1.90/imgui_impl_sdlrenderer2.h"

	// Utility structure for realtime plot
	struct GUI_ScrollingBuffer
	{
		int maxSize;
		int offset;
		ImVector<ImVec2> data;

		GUI_ScrollingBuffer(int max_size = 2000)
		{
			maxSize = max_size;
			offset = 0;
			data.reserve(maxSize);
		}

		void addPoint(float x, float y)
		{
			if (data.size() < maxSize)
				data.push_back(ImVec2(x, y));
			else
			{
				data[offset] = ImVec2(x, y);
				offset = (offset + 1) % maxSize;
			}
		}

		void erase()
		{
			if (data.size() > 0)
			{
				data.shrink(0);
				offset = 0;
			}
		}
	};

	static GUI_ScrollingBuffer s_frameTimeHistory;

	void drawMainMenu()
	{
		if (ImGui::BeginMenuBar())
		{
			bool menuExecuteFunction = false;

			if (ImGui::BeginMenu("File"))
			{
				menuExecuteFunction = false;
				ImGui::MenuItem("Reset window positions", NULL, &menuExecuteFunction);
				if (menuExecuteFunction)
				{
					// Do it.
				}
				
				menuExecuteFunction = false;
				ImGui::MenuItem("Reset black widow positions", NULL, &menuExecuteFunction);
				if (menuExecuteFunction)
				{
					// Do it.
				}
				
				static bool boo = false;
				ImGui::MenuItem("Stuffings", NULL, &boo);
				
				ImGui::Separator();

				menuExecuteFunction = false;
				ImGui::MenuItem("Exit demo", "Alt+F4", &menuExecuteFunction);
				if (menuExecuteFunction)
				{
					// Do it.
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}

	void drawInfoPanel(float audioTime, float runTime, float frameTimeInSeconds, size_t currentFrame)
	{
		const float frameTimeMs = 1000.0f * frameTimeInSeconds;
		const float frameRate = 1.0f / frameTimeInSeconds;

		if (ImGui::CollapsingHeader("Frame time stats", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImPlot::BeginPlot("Frame time", ImVec2(-1, 80), ImPlotFlags_NoLegend | ImPlotFlags_NoTitle))
			{
				ImPlot::SetupAxes(nullptr, nullptr,
					ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoGridLines,
					ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks);

				const float history = 10.0f; // Seconds
				const float yVal[] = { 1000.0f / 60.0f };

				ImPlot::SetupAxisLimits(ImAxis_X1, runTime - history, runTime, ImGuiCond_Always);
				ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1000.0f / 30.0f);
				ImPlot::SetupAxisFormat(ImAxis_X1, "%.1f s");
				ImPlot::SetupAxisFormat(ImAxis_Y1, "%.1f ms");

				ImPlot::SetNextLineStyle(ImColor(0.6f, 0.6f, 0.6f, 0.8f), 1.5f);
				ImPlot::PlotLine("Frame time", &s_frameTimeHistory.data[0].x, &s_frameTimeHistory.data[0].y, s_frameTimeHistory.data.size(), 0, s_frameTimeHistory.offset, 2 * sizeof(float));

				ImPlot::SetNextLineStyle(ImColor(0.8f, 0.0f, 0.0f, 0.75f), 1.5f);
				ImPlot::PlotInfLines("60 fps marker", yVal, 1, ImPlotInfLinesFlags_Horizontal);

				ImPlot::EndPlot();
			}

			const float smoothingCoeffA = 0.995f;
			const float smoothingCoeffB = 1.0f - smoothingCoeffA;

			static float s_minFrameTimeMs = FLT_MAX;
			static float s_maxFrameTimeMs = FLT_MIN;
			static float s_minFrameRate = FLT_MAX;
			static float s_maxFrameRate = FLT_MIN;
			static float s_lastFrameRate = 60.0f;
			static float s_lastFrameTimeMs = 1.0f / 60.0f;
			static size_t s_framesDropped = 0;

			const char* frameRateFormat = "%.1f";
			const char* frameTimeFormat = "%.2f";

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8, 0));

			if (ImGui::BeginTable("MinMaxAvgTable", 4, ImGuiTableFlags_SizingStretchProp))
			{
				if (frameRate < 60.0)
					s_framesDropped++;

				s_minFrameTimeMs = s_minFrameTimeMs < frameTimeMs ? s_minFrameTimeMs : frameTimeMs;
				s_maxFrameTimeMs = s_maxFrameTimeMs > frameTimeMs ? s_maxFrameTimeMs : frameTimeMs;
				s_minFrameRate = s_minFrameRate < frameRate ? s_minFrameRate : frameRate;
				s_maxFrameRate = s_maxFrameRate > frameRate ? s_maxFrameRate : frameRate;

				// Quick and dirty smoothing
				s_lastFrameTimeMs = smoothingCoeffA * s_lastFrameTimeMs + smoothingCoeffB * frameTimeMs;
				s_lastFrameRate = smoothingCoeffA * s_lastFrameRate + smoothingCoeffB * frameRate;

				ImGui::TableSetupColumn("", 0, 0.4f);
				ImGui::TableSetupColumn("Avg.", 0, 0.2f);
				ImGui::TableSetupColumn("Min.", 0, 0.2f);
				ImGui::TableSetupColumn("Max.", 0, 0.2f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("");

				ImGui::TableNextColumn();
				ImGui::TextColored(kGreyTextColor, "Avg.");

				ImGui::TableNextColumn();
				ImGui::TextColored(kGreyTextColor, "Min");

				ImGui::TableNextColumn();
				ImGui::TextColored(kGreyTextColor, "Max");

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::TextColored(kGreyTextColor, "Frame rate");
				ImGui::TableNextColumn();
				ImGui::Text(frameRateFormat, s_lastFrameRate);
				ImGui::TableNextColumn();

				if (s_framesDropped > 0)
					ImGui::TextColored(kFrameRateWarnColor, frameRateFormat, s_minFrameRate);
				else
					ImGui::Text(frameRateFormat, s_minFrameRate);

				ImGui::TableNextColumn();
				ImGui::Text(frameRateFormat, s_maxFrameRate);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::TextColored(kGreyTextColor, "Frame time (ms)");
				ImGui::TableNextColumn();
				ImGui::Text(frameTimeFormat, s_lastFrameTimeMs);
				ImGui::TableNextColumn();
				ImGui::Text(frameTimeFormat, s_minFrameTimeMs);

				ImGui::TableNextColumn();
				if (s_framesDropped > 0)
					ImGui::TextColored(kFrameRateWarnColor, frameTimeFormat, s_maxFrameTimeMs);
				else
					ImGui::Text(frameTimeFormat, s_maxFrameTimeMs);

				ImGui::EndTable();
			}

			if (ImGui::BeginTable("FramesDroppedTable", 4, ImGuiTableFlags_SizingStretchProp))
			{
				ImGui::TableSetupColumn("", 0, 0.4f);
				ImGui::TableSetupColumn("", 0, 0.2f);
				ImGui::TableSetupColumn("", 0, 0.2f);
				ImGui::TableSetupColumn("", 0, 0.2f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::TextColored(kGreyTextColor, "Frames dropped");

				ImGui::TableNextColumn();
				ImGui::Text("%d", s_framesDropped);

				ImGui::TableNextColumn();

				ImGui::TableNextColumn();
				if (ImGui::Button("Reset"))
				{
					s_minFrameTimeMs = FLT_MAX;
					s_maxFrameTimeMs = FLT_MIN;
					s_minFrameRate = FLT_MAX;
					s_maxFrameRate = FLT_MIN;
					s_lastFrameRate = frameRate;
					s_lastFrameTimeMs = frameTimeMs;
					s_framesDropped = 0;
				}
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();
		}
	}
#endif // GUI_ENABLED


bool Gui_Create(SDL_Window* window, SDL_Renderer* renderer)
{
#if defined(GUI_ENABLED)
	if (kGuiEnabled)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImPlot::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		
		// Keyboard navigation disabled because you easily end up in a situation where
		// pressing Escape seems reasonable ...and then you exit the demo :(
		// TODO: Flag to switch the key to exit the demo to something else, for development?
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		io.ConfigWindowsMoveFromTitleBarOnly = true;
		io.Fonts->AddFontFromFileTTF("dev-assets/Roboto-Medium.ttf", 14.0f);

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding.x = 8;
		style.WindowPadding.y = 8;
		style.FramePadding.x = 8;
		style.FramePadding.y = 2;
		style.ItemSpacing.y = 6;
		style.ItemInnerSpacing.x = 6;
		style.GrabMinSize = 10;
		style.WindowMenuButtonPosition = 1;
		style.WindowRounding = 2;
		style.PopupRounding = 2;
		style.FrameRounding = 1;
		style.GrabRounding = 1;

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Border] = ImVec4(0.18f, 0.18f, 0.18f, 0.28f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.23f);
		colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 0.85f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.25f, 0.85f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.67f, 0.67f, 0.67f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.28f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.28f);
		colors[ImGuiCol_Header] = ImVec4(0.28f, 0.28f, 0.28f, 0.28f);
		colors[ImGuiCol_HeaderActive] = colors[ImGuiCol_Header];
		colors[ImGuiCol_HeaderHovered] = colors[ImGuiCol_Header];
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.34f);
		colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.37f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.37f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.18f, 0.92f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.28f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.43f, 0.46f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.43f, 0.43f, 0.43f, 0.28f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
		colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.28f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.43f, 0.46f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.43f, 0.43f, 0.43f, 0.28f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.28f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.05f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_Text] = ImVec4(0.93f, 0.93f, 0.93f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.20f, 0.23f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);

		ImPlotStyle& ipStyle = ImPlot::GetStyle();
		ipStyle.PlotPadding.x = 0;
		ipStyle.PlotPadding.y = 0;
		ipStyle.LabelPadding.y = 4;
		ipStyle.MousePosPadding.x = 4;
		ipStyle.MousePosPadding.y = 4;

		colors = ImPlot::GetStyle().Colors;
		colors[ImPlotCol_PlotBorder] = ImVec4(1.00f, 1.00f, 1.00f, 0.05f);
		colors[ImPlotCol_PlotBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImPlotCol_AxisGrid] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
		colors[ImPlotCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		if (!ImGui_ImplSDL2_InitForSDLRenderer(window, renderer))
			return false;
		
		if (!ImGui_ImplSDLRenderer2_Init(renderer))
			return false;
	}
#endif // GUI_ENABLED

	return true;
}

void Gui_Destroy()
{
#if defined(GUI_ENABLED)
	if (kGuiEnabled)
	{
		ImPlot::DestroyContext();
		ImGui::DestroyContext();
	}
#endif // GUI_ENABLED
}

void Gui_Update()
{
#if defined(GUI_ENABLED)
	if (kGuiEnabled)
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
#endif // GUI_ENABLED
}

void Gui_Process_Event(const SDL_Event* event)
{
#if defined(GUI_ENABLED)
	if (kGuiEnabled)
		ImGui_ImplSDL2_ProcessEvent(event);
#endif // GUI_ENABLED
}

void Gui_Begin_Draw(float audioTime, float runTime, float frameTimeInSeconds, size_t currentFrame)
{
#if defined(GUI_ENABLED)
	if (kGuiEnabled)
	{
		const float frameTimeMs = 1000.0f * frameTimeInSeconds;
		const float frameRate = 1.0f / frameTimeInSeconds;

		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		//ImGui::ShowStyleEditor();
		//ImGui::ShowDemoWindow();

		s_frameTimeHistory.addPoint(runTime, frameTimeMs);

		if (Gui_Is_Visible())
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

			ImGui::Begin("GUI", nullptr, 0
				| ImGuiWindowFlags_AlwaysVerticalScrollbar // TODO: Find out why ImGui doesn't auto shows scroll bar
				| ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoDecoration
				| ImGuiWindowFlags_MenuBar
			);

			drawMainMenu();		
			drawInfoPanel(audioTime, runTime, frameTimeInSeconds, currentFrame);
		}
	}
#endif // GUI_ENABLED
}

void Gui_End_Draw()
{
#if defined(GUI_ENABLED)
	if (kGuiEnabled)
	{
		if (Gui_Is_Visible())
			ImGui::End();

		ImGui::Render();

		if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(kGui_Keys_ToggleShowGui)))
			s_guiIsVisible = !s_guiIsVisible;

		if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(kGui_Keys_ToggleShowInfo)))
			s_infoWindowIsOpen = !s_infoWindowIsOpen;

		//if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(kGuiKeys_CollapseAll)))
		//{
		//}
	}
#endif // GUI_ENABLED
}
