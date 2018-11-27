
/*
	Syntherklaas FM -- Master (voice) filters.
*/

#pragma once

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
	};

	/*
		Interface (base class).

		Notes:
			- Reset() needs to be called per voice
			- Start() must be called per voice
			- SetLiveParameters() can be called whilst rendering
			- Stop() when a voice ending (optional)
	*/

	class LadderFilter
	{
	public:
		LadderFilter() :
			m_drive(1.f)
		{}
		
		virtual ~LadderFilter() {}

		void Start(const ADSR::Parameters &parameters, float velocity)
		{
			m_ADSR.Start(parameters, velocity);
		}

		void Stop(float velocity)
		{
			m_ADSR.Stop(velocity);
		}

		virtual void SetLiveParameters(const FilterParameters &parameters)
		{
			SetDrive(parameters.drive);	
			SetCutoff(parameters.cutoff);
			SetResonance(parameters.resonance);
		}

		virtual void Reset() = 0;
		virtual void Apply(float *pSamples, unsigned numSamples, float contour, bool invert) = 0;

	protected:
		float m_drive;
		ADSR m_ADSR;

		void SetDrive(float value)
		{
			SFM_ASSERT(value >= 0.f);
			m_drive = value;
		}

		virtual void SetCutoff(float value) = 0;
		virtual void SetResonance(float value) = 0;
	};

	/*
		Unknown filter (http://www.musicdsp.org)
	*/

	class UnknownFilter : public LadderFilter
	{
	private:
		float m_cutoff;
		float m_resonance;

		float m_mX1, m_mX2, m_mY1, m_mY2;

		virtual void SetCutoff(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			m_cutoff = kAudibleLowHz + value *(kAudibleHighHz-kAudibleLowHz); // Specific range for this filter
		}

		virtual void SetResonance(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			m_resonance = -25.f + 50.f*value; // -25dB to 25dB
		}
	
	public:
		virtual void SetLiveParameters(const FilterParameters &parameters)
		{
			SetDrive(parameters.drive);	
			SetCutoff(parameters.cutoff);
			SetResonance(parameters.resonance);
		}

		virtual void Reset()
		{
			m_mX1 = m_mX2 = m_mY1 = m_mY2 = 0.f;
		}

		virtual void Apply(float *pSamples, unsigned numSamples, float contour, bool invert);

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

	class ImprovedMOOGFilter : public LadderFilter
	{
	private:
		double m_cutoff;
		double m_resonance;

		double m_V[4];
		double m_dV[4];
		double m_tV[4];

		virtual void SetCutoff(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			value *= 1000.f;
			const double omega = (kPI*value)/kSampleRate;
			m_cutoff = 4.0*kPI * kVT * value * (1.0-omega) / (1.0+omega);
		}

		virtual void SetResonance(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			m_resonance = std::max<double>(kEpsilon, value*4.0);
		}

	public:
		virtual void Reset()
		{
			for (unsigned iPole = 0; iPole < 4; ++iPole)
				m_V[iPole] = m_dV[iPole] = m_tV[iPole] = 0.0;
		}

		virtual void Apply(float *pSamples, unsigned numSamples, float contour, bool invert);
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

		double m_inputDelay;
		double m_state[4];

		virtual void SetCutoff(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			value *= 1000.f;
			value = value*2.f*kPI/kSampleRate;
			m_cutoff = value; // Also known as omega
		}

		virtual void SetResonance(float value)
		{
			SFM_ASSERT(value >= 0.f && value <= 1.f);
			m_resonance = value;
		}

	public:
		virtual void Reset()
		{
			m_inputDelay = 0.0;
			m_state[0] = m_state[1] = m_state[2] = m_state[3] = 0.0;
		}

		virtual void Apply(float *pSamples, unsigned numSamples, float contour, bool invert);
	};
}
