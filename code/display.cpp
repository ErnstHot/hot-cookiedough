
// cookiedough -- SDL software display window (32-bit)

#include "main.h"
#include "display.h"

const char* kGuiTitle = "Cookiedough";
constexpr int kGui_WindowMinWidth = 300;

Display::Display() :
	m_window(nullptr),
	m_renderer(nullptr),
	m_texture(nullptr),
	m_pitch(0)
{
	// smooth scaling (if necessary)
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

Display::~Display()
{
	Gui_Destroy();
	
	SDL_DestroyTexture(m_texture);
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
} 

bool Display::Open(const std::string &title, unsigned int xRes, unsigned int yRes, bool fullScreen)
{
	bool displayInit = true;

	// Demo window
	const int demoResult = (false == fullScreen)
		? SDL_CreateWindowAndRenderer(xRes, yRes, SDL_RENDERER_ACCELERATED, &m_window, &m_renderer)
		: SDL_CreateWindowAndRenderer(0, 0, int(SDL_WINDOW_FULLSCREEN_DESKTOP) | int(SDL_RENDERER_ACCELERATED), &m_window, &m_renderer);

	if (-1 != demoResult)
	{
		// set output resolution (regardless of what we're blitting to)
		SDL_RenderSetLogicalSize(m_renderer, xRes, yRes);

		// set title
		SDL_SetWindowTitle(m_window, title.c_str());

		m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, xRes, yRes);
		m_pitch = xRes*4;

		displayInit &= (nullptr != m_texture);
		displayInit &= Gui_Create(m_window, m_renderer);
	}
	else
	{
		const std::string SDLerror = SDL_GetError();
		SetLastError("Can't open SDL display window: " + SDLerror);

		return false;
	}

	return displayInit;
}

void Display::Update(const uint32_t *pPixels)
{
	SDL_RenderClear(m_renderer);

	if (nullptr != pPixels)
	{
		SDL_UpdateTexture(m_texture, nullptr, pPixels, m_pitch);
		SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
	}

	Gui_Update();

	SDL_RenderPresent(m_renderer);
}
