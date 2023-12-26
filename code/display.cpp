
// cookiedough -- SDL software display window (32-bit)

#include "main.h"
#include "display.h"

const char* kGuiTitle = "Cookiedough";
constexpr int kGui_WindowMinWidth = 300;

Display::Display() :
	m_demoWindow(nullptr),
	m_guiWindow(nullptr),
	m_demoRenderer(nullptr),
	m_guiRenderer(nullptr),
	m_demoTexture(nullptr),
	m_pitch(0)
{
	// smooth scaling (if necessary)
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

Display::~Display()
{
	Gui_Destroy();
	
#if defined(GUI_ENABLED)
	SDL_DestroyRenderer(m_guiRenderer);
	SDL_DestroyWindow(m_guiWindow);
#endif

	SDL_DestroyTexture(m_demoTexture);
	SDL_DestroyRenderer(m_demoRenderer);
	SDL_DestroyWindow(m_demoWindow);
} 

bool Display::Open(const std::string &title, unsigned int xRes, unsigned int yRes, bool fullScreen)
{
	bool displayInit = true;

#if defined(GUI_ENABLED)
	// GUI window
	// TODO: Remember last pos and size

	const int displayIndex = 0; // TODO: What's the correct index here?
	SDL_DisplayMode displayMode;
	SDL_GetDesktopDisplayMode(displayIndex, &displayMode);
	
	const int guiHeight = displayMode.h - 128; // TODO: Find max useable height
	const int guiResult = SDL_CreateWindowAndRenderer(kGui_WindowMinWidth, guiHeight, SDL_RENDERER_ACCELERATED, &m_guiWindow, &m_guiRenderer);

	if (-1 != guiResult)
	{
		SDL_SetWindowTitle(m_guiWindow, kGuiTitle);
		SDL_SetWindowPosition(m_guiWindow, 0, 32);
		SDL_RenderPresent(m_guiRenderer);  // Needed for SDL_GetWindowBordersSize to report accurate results?

		int top, left, bottom, right;
		SDL_GetWindowBordersSize(m_guiWindow, &top, &left, &bottom, &right);
		SDL_SetWindowResizable(m_guiWindow, SDL_TRUE);
		SDL_SetWindowPosition(m_guiWindow, 0, top);
		SDL_SetWindowMinimumSize(m_guiWindow, kGui_WindowMinWidth, 64);

		if (!Gui_Create(m_guiWindow, m_guiRenderer))
			return false;
	}
	else
	{
		const std::string SDLerrorGui = SDL_GetError();
		SetLastError("Can't open SDL display window: " + SDLerrorGui);
		
		return false;
	}
#endif

	// Demo window
	const int demoResult = (false == fullScreen)
		? SDL_CreateWindowAndRenderer(xRes, yRes, SDL_RENDERER_ACCELERATED, &m_demoWindow, &m_demoRenderer)
		: SDL_CreateWindowAndRenderer(0, 0, int(SDL_WINDOW_FULLSCREEN_DESKTOP) | int(SDL_RENDERER_ACCELERATED), &m_demoWindow, &m_demoRenderer);

	if (-1 != demoResult)
	{
		// set output resolution (regardless of what we're blitting to)
		SDL_RenderSetLogicalSize(m_demoRenderer, xRes, yRes);

		// set title
		SDL_SetWindowTitle(m_demoWindow, title.c_str());

		m_demoTexture = SDL_CreateTexture(m_demoRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, xRes, yRes);
		m_pitch = xRes*4;

		displayInit &= (nullptr != m_demoTexture);
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
	SDL_RenderClear(m_guiRenderer);
	SDL_RenderClear(m_demoRenderer);

	if (nullptr != pPixels)
	{
		SDL_UpdateTexture(m_demoTexture, nullptr, pPixels, m_pitch);
		SDL_RenderCopy(m_demoRenderer, m_demoTexture, nullptr, nullptr);
	}

	Gui_Update();

	SDL_RenderPresent(m_demoRenderer);
	SDL_RenderPresent(m_guiRenderer);
}
