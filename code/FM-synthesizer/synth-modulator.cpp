
/*
	Syntherklaas FM -- Frequency modulator (can also be used as LFO).
*/

#include "synth-global.h"
#include "synth-modulator.h"
#include "synth-oscillators.h"

namespace SFM
{
	void Modulator::Initialize(unsigned sampleOffs, float index, float frequency, float phaseShift)
	{
		m_index = index;
		m_pitch = CalculateOscPitch(frequency);
		m_sampleOffs = sampleOffs;
		m_phaseShift = (phaseShift*kOscPeriod)/k2PI;
	}

	float Modulator::Sample(unsigned sampleCount, const float *pLFO)
	{
		const unsigned sample = sampleCount-m_sampleOffs;
		const float phase = sample*m_pitch + m_phaseShift;
		
		float envelope = 1.f;
		if (nullptr != pLFO)
		{
			envelope = LUTsample(pLFO, phase);
		}

		// FIXME: try other oscillators (not without risk of noise, of course)
		const float modulation = oscSine(phase); 

		return (envelope*m_index)*modulation;
	}
}
