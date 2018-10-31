
/*
	Syntherklaas FM -- Lookup tables.
*/

#pragma once

#include "synth-global.h"
#include "synth-util.h"

namespace SFM
{
	// Generated Carrier:Modulation ratio table
	extern unsigned g_CM_table[][2];
	extern unsigned g_CM_table_size;

	// For oscillators
	extern float g_sinLUT[kOscPeriod];
	extern float g_noiseLUT[kOscPeriod];

	// Early MOOG style 3-setting pulse wave
	const float kPulseWidths[3] = 
	{
		0.1f, 0.314f, 0.75f
	};

	void CalculateLUTs();

	/*
		LUT functions
	*/

	SFM_INLINE float SampleOscLUT(const float *LUT, float index)
	{
//		unsigned from = unsigned(index);
//		const float A = LUT[from&kOscTabAnd];
//		return A;

		unsigned from = unsigned(index);
		unsigned to = from+1;
		const float delta = fracf(index);
		const float A = LUT[from&kOscTabAnd];
		const float B = LUT[to&kOscTabAnd]; // FIXME: one extra entry saves me this AND
		const float sample = lerpf<float>(A, B, delta);
		return sample;
	}

	SFM_INLINE float lutsinf(float index)   { return SampleOscLUT(g_sinLUT, index);      }
	SFM_INLINE float lutcosf(float index)   { return lutsinf(index + kOscPeriod/4.f); }
	SFM_INLINE float lutnoisef(float index) { return SampleOscLUT(g_noiseLUT, index);    }
}
