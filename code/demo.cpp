
// cookiedough -- Bypass featuring TPB. presents 'Arrested Development'

#include "main.h"
// #include <windows.h> // for audio.h
#include "demo.h"
// #include "audio.h"
#include "rocket.h"
#include "image.h"

// filters & blitters
//#include "boxblur.h"
//#include "polar.h"
#include "fx-blitter.h"
//#include "satori-lumablur.h"
//#include "lens.h"

// effects
#include "reaction-diffusion-v1-effect.h"

// for this production:
static_assert(kResX == 1280 && kResY == 720);

// --- Sync. tracks ---

SyncTrack trackEffect;

// --------------------

bool Demo_Create()
{
	if (false == Rocket::Launch())
		return false;

	bool fxInit = true;
	fxInit &= Reaction_Diffusion_V1_Effect_Create();

	// init. sync.
	trackEffect = Rocket::AddTrack("demo:Effect");

	return fxInit;
}

void Demo_Destroy()
{
	Rocket::Land();

	Reaction_Diffusion_V1_Effect_Destroy();
}

bool Demo_Draw(uint32_t *pDest, float timer, float delta)
{
	// update sync.
#if defined(SYNC_PLAYER)
	if (false == Rocket::Boost())
		return false; // demo is over!
#else
	Rocket::Boost();
#endif

	// render effect/part
	const int effect = Rocket::geti(trackEffect);

	Reaction_Diffusion_V1_Effect_Draw(pDest, timer, delta);

	return true;
}
