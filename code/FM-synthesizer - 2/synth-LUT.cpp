
/*
	Syntherklaas FM -- Global lookup tables.
*/

#include "synth-global.h"
// #include "synth-LUT.h"

namespace SFM
{
	/*
		Table taken from Hexter: converts LFO speed to frequency.
		It is based on one Jamie Bullock's measurements.
	*/

	float g_dx7_voice_lfo_frequency[128] = 
	{
		0.062506f,  0.124815f,  0.311474f,  0.435381f,  0.619784f,
		0.744396f,  0.930495f,  1.116390f,  1.284220f,  1.496880f,
		1.567830f,  1.738994f,  1.910158f,  2.081322f,  2.252486f,
		2.423650f,  2.580668f,  2.737686f,  2.894704f,  3.051722f,
		3.208740f,  3.366820f,  3.524900f,  3.682980f,  3.841060f,
		3.999140f,  4.159420f,  4.319700f,  4.479980f,  4.640260f,
		4.800540f,  4.953584f,  5.106628f,  5.259672f,  5.412716f,
		5.565760f,  5.724918f,  5.884076f,  6.043234f,  6.202392f,
		6.361550f,  6.520044f,  6.678538f,  6.837032f,  6.995526f,
		7.154020f,  7.300500f,  7.446980f,  7.593460f,  7.739940f,
		7.886420f,  8.020588f,  8.154756f,  8.288924f,  8.423092f,
		8.557260f,  8.712624f,  8.867988f,  9.023352f,  9.178716f,
		9.334080f,  9.669644f, 10.005208f, 10.340772f, 10.676336f,
		11.011900f, 11.963680f, 12.915460f, 13.867240f, 14.819020f,
		15.770800f, 16.640240f, 17.509680f, 18.379120f, 19.248560f,
		20.118000f, 21.040700f, 21.963400f, 22.886100f, 23.808800f,
		24.731500f, 25.759740f, 26.787980f, 27.816220f, 28.844460f,
		29.872700f, 31.228200f, 32.583700f, 33.939200f, 35.294700f,
		36.650200f, 37.812480f, 38.974760f, 40.137040f, 41.299320f,
		42.461600f, 43.639800f, 44.818000f, 45.996200f, 47.174400f,
		47.174400f, 47.174400f, 47.174400f, 47.174400f, 47.174400f,
		47.174400f, 47.174400f, 47.174400f, 47.174400f, 47.174400f,
		47.174400f, 47.174400f, 47.174400f, 47.174400f, 47.174400f,
		47.174400f, 47.174400f, 47.174400f, 47.174400f, 47.174400f,
		47.174400f, 47.174400f, 47.174400f, 47.174400f, 47.174400f,
		47.174400f, 47.174400f, 47.174400f
	};

	// Sinus
	alignas(16) float g_sinLUT[kOscLUTSize];

	/*
		Depending on target platform/hardware I may want to dump this to disk/ROM.
		Also, in reality, I only neet 1/4th of it.
	*/

	void CalculateLUTs()
	{
		/* 
			Gordon-Smith oscillator (sine wave generator)
		*/

		const float frequency = 1.f;
		const float theta = k2PI*frequency/kOscLUTSize;
		const float epsilon = 2.f*sinf(theta/2.f);
		
		float N, prevN = sinf(-1.f*theta);
		float Q, prevQ = cosf(-1.f*theta);

		for (unsigned iStep = 0; iStep < kOscLUTSize; ++iStep)
		{
			Q = prevQ - epsilon*prevN;
			N = epsilon*Q + prevN;
			prevQ = Q;
			prevN = N;
			g_sinLUT[iStep] = Clamp(N);
		}
	}
}

