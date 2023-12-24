#include "gui.h"
#include "main.h"


#if defined (GUI_ENABLED)
	#include "../3rdparty/imgui-1.90/imgui_internal.h"
	#include "../3rdparty/imgui-1.90/imgui_impl_sdl2.h"
	#include "../3rdparty/imgui-1.90/imgui_impl_sdlrenderer2.h"

	constexpr float kWindowDefaultWidth = 300;

	constexpr ImVec4 kGreyTextColor = { 0.60f, 0.60f, 0.60f, 1.00f };
	constexpr ImGuiKey kGuiKeys_ToggleShowGui	= ImGuiKey_F2;
	constexpr ImGuiKey kGuiKeys_ShowInfo		= ImGuiKey_F3;
	//constexpr ImGuiKey kGuiKeys_CollapseAll		= ImGuiKey_F5;

	// Utility structure for realtime plot
	struct GUI_ScrollingBuffer
	{
		int MaxSize;
		int Offset;
		ImVector<ImVec2> Data;

		GUI_ScrollingBuffer(int max_size = 2000)
		{
			MaxSize = max_size;
			Offset = 0;
			Data.reserve(MaxSize);
		}

		void AddPoint(float x, float y)
		{
			if (Data.size() < MaxSize)
				Data.push_back(ImVec2(x, y));
			else
			{
				Data[Offset] = ImVec2(x, y);
				Offset = (Offset + 1) % MaxSize;
			}
		}

		void Erase()
		{
			if (Data.size() > 0)
			{
				Data.shrink(0);
				Offset = 0;
			}
		}
	};

	static GUI_ScrollingBuffer s_frameTimeHistory;
#endif // GUI_ENABLED

static bool s_guiIsVisible			= kGui_VisibleByDefault;
static bool s_InfoWindowIsOpen		= kGui_InfoWindowDefaultOpen;


bool Gui_Is_Visible()
{
#if defined(GUI_ENABLED)
	return s_guiIsVisible && kGuiEnabled;
#endif // GUI_ENABLED

	return false;
}


bool Gui_Create(SDL_Window* window, SDL_Renderer* renderer)
{
#if defined(GUI_ENABLED)
	if (kGuiEnabled)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImPlot::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigWindowsMoveFromTitleBarOnly = true;
		io.Fonts->AddFontFromFileTTF("devassets/Roboto-Medium.ttf", 14.0f);

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding.x = 8;
		style.WindowPadding.y = 8;
		style.WindowRounding = 4;
		style.FramePadding.x = 8;
		style.FramePadding.y = 2;
		style.ItemSpacing.y = 6;
		style.ItemInnerSpacing.x = 6;
		style.WindowRounding = 4;
		style.FrameRounding = 2;
		style.GrabMinSize = 10;
		style.GrabRounding = 2;
		style.WindowMenuButtonPosition = 1;

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text]					= ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled]			= ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg]				= ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_ChildBg]				= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg]				= ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
		colors[ImGuiCol_Border]					= ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
		colors[ImGuiCol_BorderShadow]			= ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
		colors[ImGuiCol_FrameBg]				= ImVec4(0.03f, 0.03f, 0.03f, 0.54f);
		colors[ImGuiCol_FrameBgHovered]			= ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		colors[ImGuiCol_FrameBgActive]			= ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_TitleBg]				= ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TitleBgActive]			= ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]		= ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_MenuBarBg]				= ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg]			= ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_ScrollbarGrab]			= ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabHovered]	= ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabActive]	= ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colors[ImGuiCol_CheckMark]				= ImVec4(0.67f, 0.67f, 0.67f, 1.00f);
		colors[ImGuiCol_SliderGrab]				= ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_SliderGrabActive]		= ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colors[ImGuiCol_Button]					= ImVec4(0.20f, 0.20f, 0.20f, 0.85f);
		colors[ImGuiCol_ButtonHovered]			= ImVec4(0.25f, 0.25f, 0.25f, 0.85f);
		colors[ImGuiCol_ButtonActive]			= ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		colors[ImGuiCol_Header]					= ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_HeaderHovered]			= ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
		colors[ImGuiCol_HeaderActive]			= ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
		colors[ImGuiCol_Separator]				= ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_SeparatorHovered]		= ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colors[ImGuiCol_SeparatorActive]		= ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_ResizeGrip]				= ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_ResizeGripHovered]		= ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colors[ImGuiCol_ResizeGripActive]		= ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_Tab]					= ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TabHovered]				= ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_TabActive]				= ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
		colors[ImGuiCol_TabUnfocused]			= ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TabUnfocusedActive]		= ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_PlotLines]				= ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered]		= ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
		colors[ImGuiCol_PlotHistogram]			= ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered]	= ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
		colors[ImGuiCol_TableHeaderBg]			= ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TableBorderStrong]		= ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TableBorderLight]		= ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_TableRowBg]				= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt]			= ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg]			= ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_DragDropTarget]			= ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_NavHighlight]			= ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight]	= ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg]		= ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg]		= ImVec4(1.00f, 0.00f, 0.00f, 0.35f);
		//colors[ImGuiCol_DockingPreview]		= ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		//colors[ImGuiCol_DockingEmptyBg]		= ImVec4(1.00f, 0.00f, 0.00f, 1.00f);

		ImPlotStyle& ipStyle = ImPlot::GetStyle();
		ipStyle.PlotPadding.x = 0;
		ipStyle.PlotPadding.y = 0;
		ipStyle.LabelPadding.y = 4;
		ipStyle.MousePosPadding.x = 4;
		ipStyle.MousePosPadding.y = 4;

		ImVec4* ipColors = ImPlot::GetStyle().Colors;
		ipColors[ImPlotCol_PlotBorder]			= ImVec4(1.00f, 1.00f, 1.00f, 0.05f);
		ipColors[ImPlotCol_PlotBg]				= ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		ipColors[ImPlotCol_AxisGrid]			= ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
		ipColors[ImPlotCol_FrameBg]				= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

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

void Gui_Begin_Draw(float audioTime, float runTime, float delta)
{
#if defined(GUI_ENABLED)
	if (kGuiEnabled)
	{
		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();

		ImGui::NewFrame();

		s_frameTimeHistory.AddPoint(runTime, 1000.0f * delta);

		if (s_InfoWindowIsOpen)
		{
			const float pad = 8.0f;
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			const ImVec2 workPos = viewport->WorkPos;
			const ImVec2 workSize = viewport->WorkSize;
			
			ImVec2 windowPos, windowPosPivot;
			windowPos.x = workPos.x + workSize.x - pad;
			windowPos.y = workPos.y + pad;
			windowPosPivot.x = 1.0f;
			windowPosPivot.y = 0.0f;

			ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver, windowPosPivot);
			ImGui::SetNextWindowSize(ImVec2(kWindowDefaultWidth, 0));

			if (ImGui::Begin("Info"))
			{
				if (ImGui::CollapsingHeader("Frame time plot", ImGuiTreeNodeFlags_DefaultOpen))
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

						ImPlot::SetNextLineStyle(ImColor(0.12f, 0.75f, 0.5f, 0.8f), 1.5f);
						ImPlot::PlotLine("Frame time", &s_frameTimeHistory.Data[0].x, &s_frameTimeHistory.Data[0].y, s_frameTimeHistory.Data.size(), 0, s_frameTimeHistory.Offset, 2 * sizeof(float));

						ImPlot::SetNextLineStyle(ImColor(0.8f, 0.0f, 0.0f, 0.75f), 1.5f);
						ImPlot::PlotInfLines("60 fps marker", yVal, 1, ImPlotInfLinesFlags_Horizontal);

						ImPlot::EndPlot();
					}
				}

				const float smoothingCoeffA = 0.995f;
				const float smoothingCoeffB = 1.0f - smoothingCoeffA;

				static float s_minDelta = FLT_MAX;
				static float s_maxDelta = FLT_MIN;
				static float s_minFPS = FLT_MAX;
				static float s_maxFPS = FLT_MIN;
				static float s_lastFps = 60.0f;
				static float s_lastDelta = 1.0f / 60.0f;
				static size_t s_framesDropped = 0;

				if (ImGui::CollapsingHeader("Frame stats", ImGuiTreeNodeFlags_DefaultOpen))
				{
					const ImVec4 frameRateWarnColor = ImVec4(0.90f, 0.00f, 0.00f, 1.00f);
					const char* frameRateFormat = "%.1f";
					const char* frameTimeFormat = "%.2f";

					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8, 0));

					if (ImGui::BeginTable("InfoTable1", 4, ImGuiTableFlags_SizingStretchProp))
					{
						const float fps = 1.0f / delta;

						if (fps < 60.0)
							s_framesDropped++;

						s_minDelta = s_minDelta < delta ? s_minDelta : delta;
						s_maxDelta = s_maxDelta > delta ? s_maxDelta : delta;
						s_minFPS = s_minFPS < fps ? s_minFPS : fps;
						s_maxFPS = s_maxFPS > fps ? s_maxFPS : fps;

						// Quick and dirty smoothing
						s_lastDelta = smoothingCoeffA * s_lastDelta + smoothingCoeffB * delta;
						s_lastFps = smoothingCoeffA * s_lastFps + smoothingCoeffB * fps;

						ImGui::TableNextRow();
						ImGui::TableSetupColumn("", 0, 0.4f);
						ImGui::TableNextColumn();
						ImGui::Text("");
						
						ImGui::TableSetupColumn("Avg.", 0, 0.2f);
						ImGui::TableNextColumn();
						ImGui::TextColored(kGreyTextColor, "Avg.");

						ImGui::TableSetupColumn("Min.", 0, 0.2f);
						ImGui::TableNextColumn();
						ImGui::TextColored(kGreyTextColor, "Min");

						ImGui::TableSetupColumn("Max.", 0, 0.2f);
						ImGui::TableNextColumn();
						ImGui::TextColored(kGreyTextColor, "Max");

						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::TextColored(kGreyTextColor, "Frame rate");
						ImGui::TableNextColumn();
						ImGui::Text(frameRateFormat, s_lastFps);
						ImGui::TableNextColumn();
						
						if (s_framesDropped > 0)
							ImGui::TextColored(frameRateWarnColor, frameRateFormat, s_minFPS);
						else
							ImGui::Text(frameRateFormat, s_minFPS);
						
						ImGui::TableNextColumn();
						ImGui::Text(frameRateFormat, s_maxFPS);

						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::TextColored(kGreyTextColor, "Frame time (ms)");
						ImGui::TableNextColumn();
						ImGui::Text(frameTimeFormat, 1000.0f * s_lastDelta);
						ImGui::TableNextColumn();
						ImGui::Text(frameTimeFormat, 1000.0f * s_minDelta);


						ImGui::TableNextColumn();
						if (s_framesDropped > 0)
							ImGui::TextColored(frameRateWarnColor, frameTimeFormat, 1000.0f * s_maxDelta);
						else
							ImGui::Text(frameTimeFormat, 1000.0f * s_maxDelta);

						ImGui::EndTable();
					}

					if (ImGui::BeginTable("InfoTable2", 4, ImGuiTableFlags_SizingStretchProp))
					{
						ImGui::TableNextRow();
						ImGui::TableSetupColumn("", 0, 0.4f);
						ImGui::TableNextColumn();
						ImGui::TextColored(kGreyTextColor, "Frames dropped");

						ImGui::TableSetupColumn("", 0, 0.2f);
						ImGui::TableNextColumn();
						ImGui::Text("%d", s_framesDropped);

						ImGui::TableSetupColumn("", 0, 0.2f);
						ImGui::TableNextColumn();

						ImGui::TableSetupColumn("", 0, 0.2f);
						ImGui::TableNextColumn();
						if (ImGui::Button("Reset"))
						{
							s_minDelta = FLT_MAX;
							s_maxDelta = FLT_MIN;
							s_minFPS = FLT_MAX;
							s_maxFPS = FLT_MIN;
							s_lastFps = 60.0f;
							s_lastDelta = 1.0f / 60.0f;
							s_framesDropped = 0;
						}
						ImGui::EndTable();
					}
					ImGui::PopStyleVar();
				}
			}
			ImGui::End();
		}

		if (Gui_Is_Visible())
		{
			ImGui::SetNextWindowPos(ImVec2(4, 4), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(kWindowDefaultWidth, 0));
			ImGui::Begin("GUI");
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

		if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(kGuiKeys_ToggleShowGui)))
			s_guiIsVisible = !s_guiIsVisible;

		if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(kGuiKeys_ShowInfo)))
			s_InfoWindowIsOpen = !s_InfoWindowIsOpen;

		//if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(kGuiKeys_CollapseAll)))
		//{
		//}
	}
#endif // GUI_ENABLED
}
