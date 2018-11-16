

/*
	'FM. BISON' by syntherklaas.org, a subsidiary of visualizers.nl
	Precursor to the 'GENERALISSIMO FM'
	
	A polyphonic hybrid synthesis engine.

	Beta testers for the VST:
		- Ronny Pries
		- Esa Ruoho
		- Maarten van Strien
		- Mark Smith
		- Bernd H.

	Third-party / References:
		- Transistor ladder filter impl. by Teemu Voipio (KVR forum)
		- Butterworth filter from http://www.musicdsp.org (see header file for details)
		- D'Angelo & Valimaki's improved MOOG filter (paper: "An Improved Virtual Analog Model of the Moog Ladder Filter")
		- ADSR implementation by Nigel Redmon of earlevel.com
		- Basic formant filter by alex@smartelectronix.com (http://www.musicdsp.org)
		- Barry Truax: http://www.sfu.ca/~truax/fmtut.html

		And a few more bits and pieces; all credited in the code.

	Notes:
		- This code is a product of ever changing knowledge & goals, and thus a bit inconsistent left and right
		- Some calculations in here are what is referred to as "bro science"
		- To be optimized

	MiniMOOG design: https://2.bp.blogspot.com/-RRXuwRC_EkQ/WdO_ZKs1AJI/AAAAAAALMnw/nYf5AlmjevQ1AirnXidFJCeNkomYxdt9QCLcBGAs/s1600/0.jpg

	Tasks for *now*:
		- Fix DX_Voice: performance, envelope
		- Implement a decent phase drift
		- Ask around if non-linear response to controls is something you'd want in VST

	Wait for musician(s) to decide:
		- How key velocity influences voices; this is easy to tweak by either modifying velocity at the start
		  of a voice *or* tweaking the individual use cases

	Wavetable VCOs:
		- Precalculate wavetables for all oscillators and use them
		  + Step 1: precalculate each oscillator at base frequency
		  + Step 2: Make oscillator (VCO) use them
		  + This is not a high-priority task

	R&D tasks:
		- Create interface and stash synthesizer into an object
		- Voice stealing (see KVR thread: https://www.kvraudio.com/forum/viewtopic.php?f=33&t=91557&sid=fbb06ae34dfe5e582bc8f9f6df8fe728&start=15)
		- Implement "sample & hold" noise
		- Update parameters multiple times per render cycle (eliminate rogue MIDI parameter calls)
		- Formant shaping is *very* basic, so: https://www.soundonsound.com/techniques/formant-synthesis
		- First draft of manual

	Plumbing:
		- Noise type switch is now done in MIDI driver (move it)
		- Flush ring buffer using 2 memcpy() calls
		- See if all global state needs to be global
		- Move all math needed from Std3DMath to synth-math.h; stop depending on Bevacqua as a whole
		- Profiling & optimization

	R&D low priority:
		- Learn more about portamento et cetera and consider a monophonic mode
		- Read about filters a bit more
		- MinBLEP
		- Real variable delay line

	Known bugs:
		- Some controls should respond in a non-linear fashion
		- NOTE_OFF doesn't always get processed (I think it's the MIDI code, reproduce by letting a key "bounce")
		- MIDI pots crackle a bit (not important for intended target, but can be fixed with MIDI_Smoothed!)
		- Crackle when bottlenecked (should not be the case in production phase)

	Lesson(s) learned:
		- It is important to follow through and finish this
		- Don't keep pushing a feature that's just not working
*/

#ifndef _FM_BISON_H_
#define _FM_BISON_H_

#include "synth-global.h"
#include "synth-stateless-oscillators.h"

bool Syntherklaas_Create();
void Syntherklaas_Destroy();

// Returns loudest voice (linear amplitude)
float Syntherklaas_Render(uint32_t *pDest, float time, float delta);

namespace SFM
{
	/*
		API exposed to (MIDI) input.
		I'm assuming all TriggerVoice() and ReleaseVoice() calls will be made from the same thread, or at least not concurrently.
	*/

	void TriggerVoice(unsigned *pIndex /* Will receive index to use with ReleaseVoice() */, Waveform form, float frequency, float velocity);
	void ReleaseVoice(unsigned index, float velocity /* Aftertouch */);
}
#endif // _FM_BISON_H_


