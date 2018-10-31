
/*
	Syntherklaas FM -- Master (voice) filters.
*/

#pragma once

#include "synth-global.h"
#include "synth-math.h"
#include "synth-ADSR.h"

namespace SFM
{
	/*
		Filter parameters.
	*/

	struct FilterParameters
	{
		// [0..N]
		float drive;
	
		// Normalized [0..1]
		float cutoff;
		float resonance;
		float envInfl;
	};

	/*
		Interface (base class).

		Notes:
			- Reset() needs to be called per voice
			- SetADSRParameters() must be called per voice
			- SetLiveParameters() can be called whilst rendering
	*/

	class LadderFilter
	{
	public:
		LadderFilter() {}
		virtual ~LadderFilter() {}

		virtual void Reset() = 0;

		void SetADSRParameters(unsigned sampleCount, const ADSR::Parameters &parameters)
		{
			m_ADSR.Start(sampleCount, parameters, 1.f);
		}

		void SetLiveParameters(const FilterParameters &parameters)
		{
			SetCutoff(parameters.cutoff);
			SetResonance(parameters.resonance);
			SetEnvelopeInfluence(parameters.envInfl);
		}

		virtual void Apply(float *pSamples, unsigned numSamples, float globalWetness, unsigned sampleCount) = 0;

	protected:
		ADSR m_ADSR;

		virtual void SetCutoff(float value) = 0;
		virtual void SetResonance(float value) = 0;
		virtual void SetEnvelopeInfluence(float value) = 0;
	};

	/*
		Microtracker MOOG filter
		
		Credit:
		- Based on an implementation by Magnus Jonsson
		- https://github.com/magnusjonsson/microtracker (unlicense)
	*/

	class MicrotrackerMoogFilter : public LadderFilter
	{
	private:
		float m_cutoff;
		float m_resonance;
		float m_envInfl;

		float m_P0, m_P1, m_P2, m_P3;
		float m_resoCoeffs[3];

		void SetCutoff(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			value *= 1000.f;
			value = value*2.f*kPI/kSampleRate;
			m_cutoff = std::min<float>(1.f, value); // Also known as omega
		}

		void SetResonance(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			m_resonance = std::max<float>(kEpsilon, value*4.f);
		}

		void SetEnvelopeInfluence(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			m_envInfl = value;
		}
	
	public:
		virtual void Reset()
		{
			m_P0 = m_P1 = m_P2 = m_P3 = 0.f;
			m_resoCoeffs[0] = m_resoCoeffs[1] = m_resoCoeffs[2] = 0.f;
		}

		virtual void Apply(float *pSamples, unsigned numSamples, float globalWetness, unsigned sampleCount);
	};

	/*
		Improved MOOG ladder filter.

		This model is based on a reference implementation of an algorithm developed by
		Stefano D'Angelo and Vesa Valimaki, presented in a paper published at ICASSP in 2013.
		This improved model is based on a circuit analysis and compared against a reference
		Ngspice simulation. In the paper, it is noted that this particular model is
		more accurate in preserving the self-oscillating nature of the real filter.
		References: "An Improved Virtual Analog Model of the Moog Ladder Filter"

		Original Implementation: D'Angelo, Valimaki
	*/

	// Thermal voltage (26 milliwats at room temperature)
	const double kVT = 0.312;

	class ImprovedMoogFilter : public LadderFilter
	{
	private:
		double m_cutoff;
		double m_resonance;
		float m_envInfl;

		double m_V[4];
		double m_dV[4];
		double m_tV[4];

		void SetCutoff(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			value *= 1000.f;
			const double omega = (kPI*value)/kSampleRate;
			m_cutoff = 4.0*kPI * kVT * value * (1.0-omega) / (1.0+omega);
		}

		void SetResonance(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			m_resonance = std::max<double>(kEpsilon, value*4.0);
		}

		void SetEnvelopeInfluence(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			m_envInfl = value;
		}

	public:
		virtual void Reset()
		{
			for (unsigned iPole = 0; iPole < 4; ++iPole)
				m_V[iPole] = m_dV[iPole] = m_tV[iPole] = 0.0;
		}

		virtual void Apply(float *pSamples, unsigned numSamples, float globalWetness, unsigned sampleCount);
	};

	/*
		Transistor ladder filter by Teemu Voipio
		Source: https://www.kvraudio.com/forum/viewtopic.php?t=349859
	*/

	class TeemuFilter : public LadderFilter
	{
	private:
		double m_cutoff;
		double m_resonance;
		float m_envInfl;

		double m_inputDelay;
		double m_state[4];

		void SetCutoff(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			value *= 1000.f;
			value = value*2.f*kPI/kSampleRate;
			m_cutoff = value; // Also known as omega
		}

		void SetResonance(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			m_resonance = value;
		}

		void SetEnvelopeInfluence(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			m_envInfl = value;
		}

	public:
		virtual void Reset()
		{
			m_inputDelay = 0.0;
			m_state[0] = m_state[1] = m_state[2] = m_state[3] = 0.0;
		}

		virtual void Apply(float *pSamples, unsigned numSamples, float globalWetness, unsigned sampleCount);
	};
}
