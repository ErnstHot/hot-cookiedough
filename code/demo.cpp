
// cookiedough -- demo flow for: <?> by Bypass (Shader Superfights Grand Prix 2019 commercial)

#include "main.h"
// #include <windows.h> // for audio.h
#include "demo.h"
// #include "audio.h"
#include "rocket.h"

// filters & blitters
#include "boxblur.h"
#include "polar.h"

// effects
#include "ball.h"
#include "landscape.h"
#include "torus-twister.h"
#include "heartquake.h"
#include "tunnelscape.h"
#include "shadertoy.h"

SyncTrack trackPart;

bool Demo_Create()
{
	if (false == Rocket::Launch())
		return false;

	bool fxInit = true;
	fxInit &= Twister_Create();
	fxInit &= Landscape_Create();
	fxInit &= Ball_Create();
	fxInit &= Heartquake_Create();
	fxInit &= Tunnelscape_Create();
	fxInit &= Shadertoy_Create();

	trackPart = Rocket::AddTrack("part");

	return fxInit;
}

void Demo_Destroy()
{
	Rocket::Land();

	Twister_Destroy();
	Landscape_Destroy();
	Ball_Destroy();
	Heartquake_Destroy();
	Tunnelscape_Destroy();
	Shadertoy_Destroy();
}

void Demo_Draw(uint32_t *pDest, float timer, float delta)
{
	// for this production (FIXME)
	// VIZ_ASSERT(kResX == 800 && kResY == 600);

	Rocket::Boost();

	int part = Rocket::geti(trackPart);

	if (part == 1)
		Twister_Draw(pDest, timer, delta);
	
	if (part == 2)
		Landscape_Draw(pDest, timer, delta);
	
	if (part == 3)
		Ball_Draw(pDest, timer, delta);
	
	if (part == 4)
		Tunnelscape_Draw(pDest, timer, delta);
	
	if (part == 5)
		Plasma_Draw(pDest, timer, delta);
	
	if (part == 6)
		Nautilus_Draw(pDest, timer, delta);
	
	if (part == 7)
		Laura_Draw(pDest, timer, delta);
	
	if (part == 8)
		Spikey_Draw(pDest, timer, delta, true);
	
	if (part == 9)
		Spikey_Draw(pDest, timer, delta, false);

	// blit logo 
	uint32_t *pWrite = pDest + kResX*(kResY-145);
	for (int iY = 0; iY < 136; ++iY)
	{
		pWrite += (kResX-640)/2;
		MixSrc32(pWrite, g_pDesireLogo3 + iY*640, 640);
		pWrite += kResX - (kResX-640)/2;
	}

	return;
}
