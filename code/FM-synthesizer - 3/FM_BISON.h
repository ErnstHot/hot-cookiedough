

/*
	'FM. BISON' by syntherklaas.org, a subsidiary of visualizers.nl

	Third prototype of FM synthesizer
	To be released as VST by Tasty Chips Electronics

	Third party credits (not necessarily 100% complete):
		- Bits and pieces taken from Hexter by Sean Bolton (https://github.com/smbolton/hexter)
		- ADSR (modified), original by Nigel Redmon (earlevel.com)
		- Pink noise function by Paul Kellet (http://www.firstpr.com.au/dsp/pink-noise/)
		- Vowel shaper by alex@smartelectronix.com via http://www.musicdsp.org 
		- JSON++ (https://github.com/hjiang/jsonxx)

	Core goals:
		- DX7-like core FM
		- Subtractive synthesis on top

	Look at later (TM):
		- Level scaling
		  + Primitive implementation: breakpoint, subtractive/linear, amount L/R, range in semitones
		    This may well be what's needed apart from exponential (maybe!) until I see a point in going additive
		- Key rate scaling
		  + Primitive implementation: need to define key range to sensibly map linear or non-linear response
		- Enhance chorus

	Optimizations:
		- Use tables for all oscillators
		- Eliminate branches and needless oscillators
		  + A lot of branches can be eliminated by using mask values, which in turns opens us up
		    to possible SIMD optimization
		- Profile and solve hotspots (lots of floating point function calls, to name one)

	Priority:
		- Figure out how to interpret aftertouch in ADSR
		- Figure out proper pitch envelope strategy
		- Patch save & load

	Missing top-level features:
		- Jitter
		  + Partially implemented
		- Filters (LPF, vowel)
		- Unison mode?
		  + I'd suggest perhaps per 4 voices (limiting the polyphony)

	Golden rules:
		- Basic FM right first, party tricks second: consider going full VST when basic FM works right
		- Optimization is possible nearly everywhere; do not do any before the instrument is nearly done

	Issues:
		- Feedback depth(s)
		- Oxygen 49 MIDI driver hangs notes every now and then; not really worth looking into

	Keep yellow & blue on the Oxygen 49, the subtractive part on the BeatStep.
*/

#pragma once

#include "synth-global.h"
#include "synth-stateless-oscillators.h"

bool Syntherklaas_Create();
void Syntherklaas_Destroy();
void Syntherklaas_Render(uint32_t *pDest, float time, float delta);

namespace SFM
{
	/*
		API exposed to (MIDI) input.
		I'm assuming all TriggerVoice() and ReleaseVoice() calls will be made from the same thread, or at least not concurrently.
	*/

	void TriggerVoice(unsigned *pIndex /* Will receive index to use with ReleaseVoice() */, Waveform form, unsigned key, float velocity);
	void ReleaseVoice(unsigned index, float velocity /* Aftertouchs */);
}
