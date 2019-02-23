

/*
	'FM. BISON' by syntherklaas.org, a subsidiary of visualizers.nl

	Third prototype of FM synthesizer
	To be released as VST by Tasty Chips Electronics

	Code is *very* verbose and not optimized at all; still in R&D stage

	Third party credits (not necessarily 100% complete):
		- Bits and pieces taken from Hexter by Sean Bolton (https://github.com/smbolton/hexter)
		- ADSR (modified), original by Nigel Redmon (earlevel.com)
		- Pink noise function by Paul Kellet (http://www.firstpr.com.au/dsp/pink-noise/)
		- JSON++ (https://github.com/hjiang/jsonxx)
		- SvfLinearTrapOptimised2.hpp by FAC (https://github.com/FredAntonCorvest/Common-DSP)

	Core goals:
		- DX7-like core FM
		- Subtractive synthesis on top

	Look at in VST phase: 
		- Different LFO waveforms
		- Filter envelope
		- Better (default) key scaling implementation (configurable range, on octaves or keys?)
		- (OPTIONAL) Additive & non-linear level scaling (default is now subtractive/linear)
		- (OPTIONAL) Optimize filter
		- (OPTIONAL) Enhance chorus (though it's pretty fine as-is)
		- (OPTIONAL) Machine learning for patches (this would be so awesome to have)

	(OPTIONAL) For a better super/hyper saw:
		- Read: https://www.nada.kth.se/utbildning/grukth/exjobb/rapportlistor/2010/rapporter10/szabo_adam_10131.pdf

	(OPTIONAL) For FM-X support:
		- More core waveforms (look at FM-X or TX81Z(?) documentation)
		- 'Skirt' and 'Res' (basically a resonant lowpass) per operator
		  This can be cut down to 8 discrete steps and thus be precalculated at some point
		- I am covering part of this b ehaviour with my overdrive parameter
		- More information: https://www.youtube.com/watch?v=YWvSglv3iEA
	
	Optimizations:
		- Tables for all oscillators, also look at: https://github.com/logicomacorp/WaveSabre/blob/master/WaveSabreCore/src/Helpers.cpp
		- Eliminate branches and needless logic
		  + A lot of branches can be eliminated through use of mask values, which in turn opens us up
		    to SIMD optimizations (big one, plan it on paper)
		  + (OPTIONAL) Go for 8 operators?
		- Profile and solve hotspots (lots of floating point function calls, to name one)

	Plumbing:
		- (OPTIONAL) Make envelope a 'DADSR'
		- Move algorithms to dedicated file
		- Patch save & load

	Priority plus:
		- (PRIO) Top-end distortion for piano (grit)
		- Implement parameter to flatten the ADSR
		- Figure out how to interpret aftertouch in ADSR
		- Try a different form of voice allocation so that a voice can be reused before NOTE_OFF

	Priority:
	    - Pitch envelope: refine, add time stretch!
		- Migrate to VST
		  + Allows to better estimate which paramaters need range adjustment
		  + Right now I'm setting the patch according to MIDI values, but that should not be done that way
		    in a VST harness
		  + Consider Yamaha EG-style envelopes
		- Finish 'pickup' mode
		- Look at recent FIXMEs, rethink your life, read http://people.ece.cornell.edu/land/courses/ece4760/Math/GCC644/FM_synth/Chowning.pdf

	Missing top-level features:
		- Jitter is only partially implemented
		- Unison mode, portamento (monophonic) perhaps

	A few good tips for better liveliness and Rhodes: 
		- https://www.youtube.com/watch?v=72WoueTI354
		- http://recherche.ircam.fr/anasyn/falaize/applis/rhodes/
		- http://www.dafx17.eca.ed.ac.uk/papers/DAFx17_paper_79.pdf

	Golden rules:
		- Basic FM right first, party tricks second: consider going full VST when basic FM works right
		- Don't chase the DX7: you'll be spending hours emulating algorithms based on past restrictions (reading doesn't hurt, though)

	Issues:
		- Oxygen 49 MIDI driver hangs notes every now and then; not really worth looking into

	Keep yellow & blue on the Oxygen 49, the subtractive part, mostly, on the BeatStep.
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

	void TriggerVoice(unsigned *pIndex /* Will receive index to use with ReleaseVoice() */, unsigned key, float velocity);
	void ReleaseVoice(unsigned index, float velocity /* Aftertouch */);
}
