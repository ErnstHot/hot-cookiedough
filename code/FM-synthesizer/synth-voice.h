
/*
	Syntherklaas FM - Voice.

	This is a multiple FM operator setup but built from a different (subtractive synthesis) perspective; it will in due time
	be replaced by DX_Voice.

	Number of operators: 6 at max.
*/

#pragma once

#include "synth-global.h"
#include "synth-oscillator.h"
#include "synth-modulator.h"
// #include "synth-ADSR.h."
// #include "synth-filter.h"
#include "synth-parameters.h"
#include "synth-simple-filters.h"

namespace SFM
{
	// Initialized manually
	class Voice
	{
	public:
		Voice() :
			m_enabled(false)
		{}

		bool m_enabled;

		Algorithm m_algorithm;
		Oscillator m_carriers[3];
		Modulator m_modulator;
		Oscillator m_AM;

		// For Algorithm #3
		LowpassFilter m_LPF;

		// For wavetable samples
		bool m_oneShot;

		// For pulse-based waveforms
		float m_pulseWidth;
		
		// Filter instance for this particular voice
		LadderFilter *m_pFilter;

		float Sample(const Parameters &parameters);

		void PitchBend(float bend)
		{
			// Bend carriers
			m_carriers[0].PitchBend(bend);
			m_carriers[2].PitchBend(bend);
			m_carriers[3].PitchBend(bend);

			// Bend modulator
			m_modulator.m_oscSoft.PitchBend(bend);
			m_modulator.m_oscSharp.PitchBend(bend);
			m_modulator.m_indexLFO.PitchBend(bend);
		}

		// Can be used to determine if a one-shot is done
		bool HasCycled() /* const */
		{
			switch (m_algorithm)
			{
			default:

			case kMiniMOOG: // In MiniMOOG-mode, only the first carrier is allowed non-procedural
			case kSingle:
				return m_carriers[0].HasCycled();

			case kDoubleCarriers:
				return m_carriers[1].HasCycled() || m_carriers[1].HasCycled();
			}
		}
	};
}
