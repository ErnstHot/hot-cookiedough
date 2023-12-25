
// cookiedough -- gui, from Purple with love

#pragma once

#include "../3rdparty/SDL2-2.28.5/include/SDL.h"

bool Gui_Is_Visible();

bool Gui_Create(SDL_Window*, SDL_Renderer*);
void Gui_Destroy();
void Gui_Update();
void Gui_Process_Event(const SDL_Event*);
void Gui_Begin_Draw(float audioTime, float runTime, float delta, size_t currentFrame);
void Gui_End_Draw();


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
