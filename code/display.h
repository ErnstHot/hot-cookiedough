
// cookiedough -- SDL software display window (32-bit)

#pragma once

#include "../3rdparty/SDL2-2.28.5/include/SDL.h"
#include "gui.h"

class Display
{
public:
	Display();
	~Display();

	bool Open(const std::string &title, unsigned int xRes, unsigned int yRes, bool fullScreen);

	// pass a kResX*kResY ARGB8888 buffer or NULL to just clear the screen
	void Update(const uint32_t *pPixels);

private:
	SDL_Window   *m_demoWindow;
	SDL_Window   *m_guiWindow;
	SDL_Renderer *m_guiRenderer;
	SDL_Renderer *m_demoRenderer;
	SDL_Texture  *m_demoTexture;

	unsigned m_pitch;
};
