
/*
	Syntherklaas FM -- Carrier wave.
*/

#pragma once

#include "synth-global.h"
#include "synth-carrier.h"

namespace SFM
{
	void Carrier::Initialize(unsigned sampleCount, Waveform form, float amplitude, float frequency)
	{
		m_sampleOffs = sampleCount;
		m_form = form;
		m_amplitude = amplitude;
		m_frequency = frequency;
		m_pitch = CalculateOscPitch(frequency);
		m_numHarmonics = GetCarrierHarmonics(frequency);
	
		switch (m_form)
		{
		case kKick808:
			m_cycleLen = getOscKick808().GetLength();
			break;

		case kSnare808:
			m_cycleLen = getOscSnare808().GetLength();
			break;

		case kGuitar:
			m_cycleLen = getOscGuitar().GetLength();
			break;

		case kElectricPiano:
			m_cycleLen = getOscElecPiano().GetLength();
			break;
			
		default:
			m_cycleLen = kOscPeriod;
			break;
		}
	}

	float Carrier::Sample(unsigned sampleCount, float modulation, float pulseWidth)
	{
		const unsigned sample = sampleCount-m_sampleOffs;
		const float phase = sample*m_pitch;

		// Convert modulation to LUT period
		modulation *= kRadToOscLUT;

		float signal = 0.f;
		switch (m_form)
		{
		default:
			SFM_ASSERT(false); // Unsupported oscillator

		case kSine:
			signal = oscSine(phase+modulation);
			break;

		case kPolySaw:
//			signal = oscSoftSaw(phase+modulation, m_numHarmonics);
			signal = oscPolySaw(phase+modulation, m_frequency);
			break;

		// FIXME
		case kPolySquare:
			signal = oscSoftSquare(phase+modulation, m_numHarmonics);
//			signal = oscPolySquare(phase+modulation, m_frequency);
			break;

		case kPolyPulse:
			signal = oscPolyPulse(phase+modulation, m_frequency, pulseWidth);
			break;

		case kDigiTriangle:
			signal = oscDigiTriangle(phase+modulation);
			break;

		case kPinkNoise:
			signal = oscPinkNoise(phase+modulation);
			break;

		// No FM for wavetable carriers
		case kKick808:
			signal = getOscKick808().Sample(phase);
			break;

		case kSnare808:
			signal = getOscSnare808().Sample(phase);
			break;

		case kGuitar:
			signal = getOscGuitar().Sample(phase);
			break;

		case kElectricPiano:
			signal = getOscElecPiano().Sample(phase);
			break;
		}

		signal *= m_amplitude;
		SampleAssert(signal);

		return signal;
	}
}
