
// cookiedough -- gui

#pragma once

#include "../3rdparty/SDL2-2.28.5/include/SDL.h"


constexpr bool kGui_InfoWindowDefaultOpen = true;


bool Gui_Is_Visible();

bool Gui_Create(SDL_Window* window, SDL_Renderer* renderer);
void Gui_Destroy();
void Gui_Update();
void Gui_Process_Event(const SDL_Event* event);

void Gui_Begin_Draw(float audioTime, float runTime, float delta);
void Gui_End_Draw();


struct GUI_ScrollingBuffer;


//	Example GUI funcion
// 
//static float s_someValue = 0;
//static void someFunction() {};
//
//static void exampleGui()
//{
//#if defined(GUI_ENABLED)
//	if (Gui_Is_Visible())
//	{
//		if (ImGui::CollapsingHeader("Your Effect name here!", ImGuiTreeNodeFlags_DefaultOpen))
//		{
//			ImGui::SeparatorText("Options");
//			if (ImGui::Button("Call some function"))
//				someFunction();
//
//			ImGui::SeparatorText("Parameters");
//			ImGui::SliderFloat("Some value", &s_someValue, 0.0f, 1.0f, "%.3f");
//		}
//	}
//#endif // GUI_ENABLED
//}
