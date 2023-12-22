#if defined(_WIN32)
	#include <windows.h>
	#include <crtdbg.h>
#endif

#include "main.h"
#include "image.h"
#include "fx-blitter.h"
#include "random.h"

#include "eh-util.h"
#include "reaction-diffusion-v1.h"

using namespace ErnstHot;


// Private functions //////////////////////////////////////////////////////////

// Gaussian kernel generating code adapted from T.L in:
//	https://stackoverflow.com/questions/66153071/how-to-create-a-gaussian-kernel-of-arbitrary-width

// Compute a sigma^2 that 'fits' half the width of the kernel
// Now set up only for 3x3 kernels
static float computeSquaredVariance(const float epsilon = 0.001f)
{
	VIZ_ASSERT(0.0f < epsilon && epsilon < 1.0f); // Small value required

	return -3.125f / logf(epsilon);
}

static float gaussianExp(const float x, const float y, const float sigma2)
{
	VIZ_ASSERT(0.0 < sigma2);
	return expf(-(x * x + y * y) / (2.0f * sigma2));
}

static void updatePixel(REDI_V1_Buffers* const pBuffers, REDI_V1_Kernel const * const pKernel, REDI_V1_Parameters const * const pParameters,
	const size_t xIdx, const size_t yIdx, const bool wrap)
{
	const size_t bufferIndex = XYToIdx(xIdx, yIdx, pBuffers->BufferResX);
	
	const float aCenter = pBuffers->pBufferA[bufferIndex];
	const float bCenter = pBuffers->pBufferB[bufferIndex];
	
	float sumA = aCenter;
	float sumB = bCenter;

	// TODO: Unroll?
	for (int32_t kernelYIdx = -1; kernelYIdx <= 1; kernelYIdx++)
	{
		for (int32_t kernelXIdx = -1; kernelXIdx <= 1; kernelXIdx++)
		{
			const size_t kernelIdx = (kernelYIdx + 1) * REDI_V1_Kernel::Width + kernelXIdx + 1;
			
			// TODO: Maybe split function into a wrapped and a non-wrapped version?
			const size_t bufferIdx = wrap
				? XYToIdxWrap(static_cast<int32_t>(xIdx) + kernelXIdx, static_cast<int32_t>(yIdx) + kernelYIdx, pBuffers->BufferResX, pBuffers->BufferResY)
				: XYToIdx(xIdx + kernelXIdx, yIdx + kernelYIdx, pBuffers->BufferResX);

			sumA += pParameters->diffA * pKernel->pData[kernelIdx] * pBuffers->pBufferA[bufferIdx];
			sumB += pParameters->diffB * pKernel->pData[kernelIdx] * pBuffers->pBufferB[bufferIdx];
		}
	}

	const float addRemoveValue = aCenter * bCenter * bCenter;

	pBuffers->pBufferANext[bufferIndex] = sumA - addRemoveValue + pParameters->feed * (1.0f - aCenter);
	pBuffers->pBufferBNext[bufferIndex] = sumB + addRemoveValue - (pParameters->feed + pParameters->kill) * pBuffers->pBufferB[bufferIndex];
}


// Public functions ///////////////////////////////////////////////////////////

bool REDI_V1_Kernel_Create(REDI_V1_Kernel* pKernel)
{
	if (nullptr != pKernel->pData)
		freeAligned(pKernel->pData);

	pKernel->pData = static_cast<float*>(mallocAligned(pKernel->SizeInBytes, kAlignTo));

	if (nullptr == pKernel->pData)
		return false;

	return true;
}

void REDI_V1_Kernel_Destroy(REDI_V1_Kernel* pKernel)
{
	freeAligned(pKernel->pData);
}


void REDI_V1_Kernel_Generate(REDI_V1_Kernel* const pKernel)
{
	VIZ_ASSERT(nullptr != pKernel);
	VIZ_ASSERT(nullptr != pKernel->pData);

	const float sigma2 = computeSquaredVariance(0.001f);

	for (size_t yIdx = 0; yIdx < REDI_V1_Kernel::Width; yIdx++)
	{
		for (size_t xIdx = 0; xIdx < REDI_V1_Kernel::Width; xIdx++)
		{
			const float x = (float)xIdx - 1.0f;
			const float y = (float)yIdx - 1.0f;
			float z = gaussianExp(x, y, sigma2);
			pKernel->pData[XYToIdx(xIdx, yIdx, REDI_V1_Kernel::Width)] = z; // powf(z, 1.0f); // TODO: Do something with the power or remove it.
		}
	}
}


void REDI_V1_Kernel_Normalize(REDI_V1_Kernel* const pKernel)
{
	// The center of the kernel must be -1.
	// The sum of all values in the kernel must be 0.

	VIZ_ASSERT(nullptr != pKernel);
	VIZ_ASSERT(nullptr != pKernel->pData);

	float sum = 0.0f;

	for (size_t i = 0; i < REDI_V1_Kernel::Size; i++)
	{
		if (i == REDI_V1_Kernel::CenterIndex)
			continue;

		sum += pKernel->pData[i];
	}

	VIZ_ASSERT(0.0f < sum);

	const float rSum = 1.0f / sum;

	for (size_t i = 0; i < 9; i++)
		pKernel->pData[i] *= rSum;

	pKernel->pData[REDI_V1_Kernel::CenterIndex] = -1.0f;
}


///////////////////////////////////////////////////////////////////////////////

bool REDI_V1_Buffers_Create(REDI_V1_Buffers* const pBuffers)
{
	VIZ_ASSERT(nullptr != pBuffers);

	pBuffers->pBufferA = static_cast<float*>(mallocAligned(pBuffers->BufferBytesFloat, kAlignTo));
	pBuffers->pBufferB = static_cast<float*>(mallocAligned(pBuffers->BufferBytesFloat, kAlignTo));
	pBuffers->pBufferANext = static_cast<float*>(mallocAligned(pBuffers->BufferBytesFloat, kAlignTo));
	pBuffers->pBufferBNext = static_cast<float*>(mallocAligned(pBuffers->BufferBytesFloat, kAlignTo));

	//uint8_t* pModMap = Image_Load8(pBuffers->mapAssetPath);

	if (   nullptr == pBuffers->pBufferA
		|| nullptr == pBuffers->pBufferB
		|| nullptr == pBuffers->pBufferANext
		|| nullptr == pBuffers->pBufferBNext
		)	return false;

	//Uint8BufferToFloatBuffer(pBuffers->pModMapF, pModMap, pBuffers->BufferSize);

	return true;
}

void REDI_V1_Buffers_Destroy(REDI_V1_Buffers* const pBuffers)
{
	VIZ_ASSERT(nullptr != pBuffers);

	freeAligned(pBuffers->pBufferA);
	freeAligned(pBuffers->pBufferB);
	freeAligned(pBuffers->pBufferANext);
	freeAligned(pBuffers->pBufferBNext);
}

void REDI_V1_Buffers_Reset(REDI_V1_Buffers* const pBuffers, const float fillChance)
{
	VIZ_ASSERT(nullptr != pBuffers);

	const float rChance = 1.0f - 0.01f * fillChance;

	for (size_t i = 0; i < pBuffers->BufferSize; i++)
	{
		pBuffers->pBufferA[i] = 1.0;
		pBuffers->pBufferB[i] = 0.0;

		if (mt_randf() > rChance)
		{
			pBuffers->pBufferA[i] = 0.0;
			pBuffers->pBufferB[i] = 1.0;
		}
	}
}

void REDI_V1_Buffers_Calc_Range(REDI_V1_Buffers* const pBuffers)
{
	VIZ_ASSERT(nullptr != pBuffers);

	if (kCalcBufferRange)
	{
		for (size_t i = 0; i < pBuffers->BufferSize; i++)
		{
			pBuffers->minValue = pBuffers->pBufferA[i] < pBuffers->minValue ? pBuffers->pBufferA[i] : pBuffers->minValue;
			pBuffers->maxValue = pBuffers->pBufferA[i] > pBuffers->maxValue ? pBuffers->pBufferA[i] : pBuffers->maxValue;
			pBuffers->minValue = pBuffers->pBufferB[i] < pBuffers->minValue ? pBuffers->pBufferB[i] : pBuffers->minValue;
			pBuffers->maxValue = pBuffers->pBufferB[i] > pBuffers->maxValue ? pBuffers->pBufferB[i] : pBuffers->maxValue;

			pBuffers->minValue = pBuffers->pBufferANext[i] < pBuffers->minValue ? pBuffers->pBufferANext[i] : pBuffers->minValue;
			pBuffers->maxValue = pBuffers->pBufferANext[i] > pBuffers->maxValue ? pBuffers->pBufferANext[i] : pBuffers->maxValue;
			pBuffers->minValue = pBuffers->pBufferBNext[i] < pBuffers->minValue ? pBuffers->pBufferBNext[i] : pBuffers->minValue;
			pBuffers->maxValue = pBuffers->pBufferBNext[i] > pBuffers->maxValue ? pBuffers->pBufferBNext[i] : pBuffers->maxValue;
		}
	}
}

void REDI_V1_Buffers_Print_Range(REDI_V1_Buffers const * const pBuffers)
{
	VIZ_ASSERT(nullptr != pBuffers);

	if (kCalcBufferRange)
	{
		const size_t maxStringSize = 256;
		char outString[maxStringSize];

		snprintf(outString, maxStringSize, "\n\n********************************************************************************\n");
		OutputDebugString(outString);
		snprintf(outString, maxStringSize, "minValue: %f, maxValue: %f", pBuffers->minValue, pBuffers->maxValue);
		OutputDebugString(outString);
		snprintf(outString, maxStringSize, "\n********************************************************************************\n\n");
		OutputDebugString(outString);
	}
}


void REDI_V1_Buffers_Update(REDI_V1_Buffers* const pBuffers, REDI_V1_Kernel const * const pKernel, REDI_V1_Parameters const * const pParameters, const bool wrapEdges)
{
	VIZ_ASSERT(nullptr != pBuffers);
	VIZ_ASSERT(nullptr != pKernel);
	VIZ_ASSERT(nullptr != pParameters);

	// Inner frame
	for (size_t yIdx = 1; yIdx < pBuffers->BufferResY - 1; yIdx++)
	{
		for (size_t xIdx = 1; xIdx < pBuffers->BufferResX - 1; xIdx++)
			updatePixel(pBuffers, pKernel, pParameters, xIdx, yIdx, false);
	}

	if (wrapEdges)
	{
		// Top and bottom edges
		for (size_t xIdx = 0; xIdx < pBuffers->BufferResX; xIdx++)
		{
			updatePixel(pBuffers, pKernel, pParameters, xIdx, 0, true);
			updatePixel(pBuffers, pKernel, pParameters, xIdx, pBuffers->BufferResY - 1, true);
		}

		// Left and right edges
		for (size_t yIdx = 0; yIdx < pBuffers->BufferResY; yIdx++)
		{
			updatePixel(pBuffers, pKernel, pParameters, 0, yIdx, true);
			updatePixel(pBuffers, pKernel, pParameters, pBuffers->BufferResX - 1, yIdx, true);
		}
	}

	std::swap(pBuffers->pBufferA, pBuffers->pBufferANext);
	std::swap(pBuffers->pBufferB, pBuffers->pBufferBNext);

	REDI_V1_Buffers_Calc_Range(pBuffers);
}

void REDI_V1_Buffers_Update(REDI_V1_Buffers* const pBuffers, REDI_V1_Kernel const * const pKernel, REDI_V1_Maps const * const pMaps, const bool wrapEdges)
{
	VIZ_ASSERT(nullptr != pBuffers);
	VIZ_ASSERT(nullptr != pKernel);
	VIZ_ASSERT(nullptr != pMaps);

	REDI_V1_Parameters params;

	// Inner frame
	for (size_t yIdx = 1; yIdx < pBuffers->BufferResY - 1; yIdx++)
	{
		for (size_t xIdx = 1; xIdx < pBuffers->BufferResX - 1; xIdx++)
		{
			const size_t prmIdx = yIdx * pMaps->ResX + xIdx;
			
			params.diffA = pMaps->pDiffA[prmIdx];
			params.diffB = pMaps->pDiffB[prmIdx];
			params.feed = pMaps->pFeed[prmIdx];
			params.kill = pMaps->pKill[prmIdx];

			updatePixel(pBuffers, pKernel, &params, xIdx, yIdx, false);
		}
	}

	if (wrapEdges)
	{
		// Top and bottom edges
		for (size_t xIdx = 0; xIdx < pBuffers->BufferResX; xIdx++)
		{
			const size_t prmIdx0 = pMaps->ResX + xIdx;
			const size_t prmIdx1 = (pBuffers->BufferResY - 1) * pMaps->ResX + xIdx;

			params.diffA = pMaps->pDiffA[prmIdx0];
			params.diffB = pMaps->pDiffB[prmIdx0];
			params.feed = pMaps->pFeed[prmIdx0];
			params.kill = pMaps->pKill[prmIdx0];

			updatePixel(pBuffers, pKernel, &params, xIdx, 0, true);

			params.diffA = pMaps->pDiffA[prmIdx1];
			params.diffB = pMaps->pDiffB[prmIdx1];
			params.feed = pMaps->pFeed[prmIdx1];
			params.kill = pMaps->pKill[prmIdx1];

			updatePixel(pBuffers, pKernel, &params, xIdx, pBuffers->BufferResY - 1, true);
		}

		// Left and right edges
		for (size_t yIdx = 0; yIdx < pBuffers->BufferResY; yIdx++)
		{
			const size_t prmIdx0 = yIdx * pMaps->ResX;
			const size_t prmIdx1 = yIdx * pMaps->ResX + pBuffers->BufferResX - 1;

			params.diffA = pMaps->pDiffA[prmIdx0];
			params.diffB = pMaps->pDiffB[prmIdx0];
			params.feed = pMaps->pFeed[prmIdx0];
			params.kill = pMaps->pKill[prmIdx0];

			updatePixel(pBuffers, pKernel, &params, 0, yIdx, true);

			params.diffA = pMaps->pDiffA[prmIdx1];
			params.diffB = pMaps->pDiffB[prmIdx1];
			params.feed = pMaps->pFeed[prmIdx1];
			params.kill = pMaps->pKill[prmIdx1];

			updatePixel(pBuffers, pKernel, &params, pBuffers->BufferResX - 1, yIdx, true);
		}
	}

	std::swap(pBuffers->pBufferA, pBuffers->pBufferANext);
	std::swap(pBuffers->pBufferB, pBuffers->pBufferBNext);

	REDI_V1_Buffers_Calc_Range(pBuffers);
}

