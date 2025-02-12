
// cookiedough -- polar blits (FIXME: tons of code duplication))

#include "main.h"
// #include "polar.h"
#include "bilinear.h"
#include "fx-blitter.h"

static int *s_pMap        = nullptr;
static int *s_pInvMap     = nullptr;
static int *s_pMap2x2     = nullptr;
static int *s_pInvMap2x2  = nullptr;

static void CalculateMaps(int *pDest, int *pInvDest, unsigned srcResX, unsigned srcResY, unsigned destResX, unsigned destResY)
{
	const float halfResX = destResX/2.f;
	const float halfResY = destResY/2.f;

	// calculate cartesian-to-polar transform map (srcRes -> destRes)
	unsigned iPixel = 0;
	const float maxDist = sqrtf(halfResX*halfResX + halfResY*halfResY);
	for (float Y = -halfResY; Y < halfResY; Y += 1.f)
	{
		for (float X = -halfResX + kEpsilon; X < halfResX; X += 1.f)
		{
			const float distance = sqrtf(X*X + Y*Y) / maxDist;
			float theta = atan2f(Y, X);
			theta += kPI;
			theta /= kPI*2.f;
			const float U    = distance*(srcResX-1.f);      
			const float invU = (1.f-distance) * (srcResX-1.f); 
			const float V    = theta * (srcResY-1.f);          

			// non-zero edges must be patched in absence of tiling logic
			// it's a simple reverse (read previous pixel first & invert weight)

			if (U >= srcResX-1.f)
				pDest[iPixel] = ((srcResX-2)<<8) | 0xff;
			else
				pDest[iPixel] = ftofp24(U);
	
			if (invU >= srcResX-1.f)
				pInvDest[iPixel] = ((srcResX-2)<<8) | 0xff;
			else
				pInvDest[iPixel] = ftofp24(invU);

			if (V >= srcResY-1.f)
				pInvDest[iPixel+1] = pDest[iPixel+1] = ((srcResY-2)<<8) | 0xff;
			else
				pInvDest[iPixel+1] = pDest[iPixel+1] = ftofp24(V);

			iPixel += 2;
		}
	}
}

bool Polar_Create()
{
	s_pMap       = static_cast<int*>(mallocAligned(kOutputSize*sizeof(int)*2, kAlignTo));
	s_pInvMap    = static_cast<int*>(mallocAligned(kOutputSize*sizeof(int)*2, kAlignTo));
	s_pMap2x2    = static_cast<int*>(mallocAligned(kFxMapSize*sizeof(int)*2, kAlignTo));
	s_pInvMap2x2 = static_cast<int*>(mallocAligned(kFxMapSize*sizeof(int)*2, kAlignTo));

	CalculateMaps(s_pMap, s_pInvMap, kTargetResX, kTargetResY, kResX, kResY);
	CalculateMaps(s_pMap2x2, s_pInvMap2x2, kFxMapResX, kFxMapResY, kFxMapResX, kFxMapResY);

	return true;
}

void Polar_Destroy() 
{
	freeAligned(s_pMap);
	freeAligned(s_pInvMap);
	freeAligned(s_pMap2x2);
	freeAligned(s_pInvMap2x2);
}

VIZ_INLINE __m128i Fetch32(int *pRead, const uint32_t *pSrc, const unsigned targetResX)
{
	const int U = pRead[0];
	const int V = pRead[1];

	// prepare UVs
	const unsigned int U0 = U >> 8;
	const unsigned int V0 = (V >> 8) * targetResX;
	const unsigned int fracU = (U & 0xff) * 0x01010101;
	const unsigned int fracV = (V & 0xff) * 0x01010101;

	// sample & return
	return bsamp32_32(pSrc, U0, V0, U0+1, V0+targetResX, fracU, fracV);
}

VIZ_INLINE __m128i Fetch16(int *pRead, const uint32_t *pSrc, const unsigned targetResX)
{
	const int U = pRead[0];
	const int V = pRead[1];

	// prepare UVs
	const unsigned int U0 = U >> 8;
	const unsigned int V0 = (V >> 8) * targetResX;
	const unsigned int fracU = (U & 0xff) * 0x01010101;
	const unsigned int fracV = (V & 0xff) * 0x01010101;

	// sample & return
	return bsamp32_16(pSrc, U0, V0, U0+1, V0+targetResX, fracU, fracV);
}

// minor cache penalties are incurred, but L1 should always have us covered on modern CPUs
void Polar_Blit(uint32_t *pDest, const uint32_t *pSrc, bool inverse /* = false */)
{
	int *pRead = (!inverse) ? s_pMap : s_pInvMap;
	__m128i *pDest128 = reinterpret_cast<__m128i*>(pDest);

	#pragma omp parallel for schedule(static)
	for (int iPixel = 0; iPixel < kOutputSize>>2; ++iPixel)
	{
		auto mapIndex = iPixel<<3;
		__m128i A = Fetch32(pRead+mapIndex+0, pSrc, kTargetResX);
		__m128i B = Fetch32(pRead+mapIndex+2, pSrc, kTargetResX);
		__m128i C = Fetch32(pRead+mapIndex+4, pSrc, kTargetResX);
		__m128i D = Fetch32(pRead+mapIndex+6, pSrc, kTargetResX);
		__m128i AB = _mm_packus_epi32(A, B);
		__m128i CD = _mm_packus_epi32(C, D);
		pDest128[iPixel] = _mm_packus_epi16(AB, CD);
	}
}

void Polar_BlitA(uint32_t *pDest, const uint32_t *pSrc, bool inverse /* = false */)
{
	int *pRead = (!inverse) ? s_pMap : s_pInvMap;

	#pragma omp parallel for schedule(static)
	for (int iPixel = 0; iPixel < kOutputSize; ++iPixel)
	{
		auto mapIndex = iPixel<<1;
		const __m128i srcColor  = Fetch16(pRead+mapIndex, pSrc, kTargetResX);
		const __m128i alphaUnp = _mm_shufflelo_epi16(srcColor, 0xff);
		const __m128i destColor = _mm_unpacklo_epi8(_mm_cvtsi32_si128(pDest[iPixel]), _mm_setzero_si128());
		const __m128i delta = _mm_mullo_epi16(alphaUnp, _mm_sub_epi16(srcColor, destColor));
		const __m128i color = _mm_srli_epi16(_mm_add_epi16(_mm_slli_epi16(destColor, 8), delta), 8);
		pDest[iPixel] = _mm_cvtsi128_si32(_mm_packus_epi16(color, _mm_setzero_si128()));
	}
}

void Polar_Blit_2x2(uint32_t *pDest, const uint32_t *pSrc, bool inverse /* = false */)
{
	int *pRead = (!inverse) ? s_pMap2x2 : s_pInvMap2x2;
	__m128i *pDest128 = reinterpret_cast<__m128i*>(pDest);

	#pragma omp parallel for schedule(static)
	for (int iPixel = 0; iPixel < kFxMapSize>>2; ++iPixel)
	{
		auto mapIndex = iPixel<<3;
		__m128i A = Fetch32(pRead+mapIndex+0, pSrc, kFxMapResX);
		__m128i B = Fetch32(pRead+mapIndex+2, pSrc, kFxMapResX);
		__m128i C = Fetch32(pRead+mapIndex+4, pSrc, kFxMapResX);
		__m128i D = Fetch32(pRead+mapIndex+6, pSrc, kFxMapResX);
		__m128i AB = _mm_packus_epi32(A, B);
		__m128i CD = _mm_packus_epi32(C, D);
		pDest128[iPixel] = _mm_packus_epi16(AB, CD);
	}
}
