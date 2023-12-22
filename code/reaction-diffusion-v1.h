
//	Reaction diffusion framework V1

#pragma once

// Keep track of the numerical range of the buffers, in case someone wants to recreate this with fixedpoint maths.
// Is there any benefit to that? I dunno.
constexpr bool kCalcBufferRange = false;


struct REDI_V1_Parameters
{
	uint32_t	fillSeed	= 0;		// Not currently used.
	float		fillChance	= 0.0;		// Chance that a pixel will be set when clearing the reaction diffusion buffers

	float		diffA		= 0.8;		// Default value ~1.0
	float		diffB		= 0.5;		// Default value ~0.5
	float		feed		= 0.001;
	float		kill		= 0.05;
};


struct REDI_V1_Kernel
{
	static const size_t	Width = 3; // Must be 3. Do not change.
	static const size_t	Size = Width * Width;
	static const size_t	SizeInBytes = Size * sizeof(float);
	static const size_t	HalfWidthI = Width / 2;
	static const size_t	CenterIndex = HalfWidthI * Width + HalfWidthI;

	float* pData = nullptr;
};

bool REDI_V1_Kernel_Create(REDI_V1_Kernel* const pKernel);
void REDI_V1_Kernel_Destroy(REDI_V1_Kernel* const pKernel);
void REDI_V1_Kernel_Generate(REDI_V1_Kernel* const pKernel);
void REDI_V1_Kernel_Normalize(REDI_V1_Kernel* const pKernel);


struct REDI_V1_Maps
{
	REDI_V1_Maps(const size_t inResX, const size_t inResY)
		: ResX(inResX)
		, ResY(inResY)
		, BufferSize(inResX* inResY)
		, BufferBytesFloat(inResX* inResY * sizeof(float))
		, BufferBytesUint32(inResX* inResY * sizeof(uint32_t))
	{
		VIZ_ASSERT(0 < inResX);
		VIZ_ASSERT(0 < inResY);
	}

	const size_t ResX;
	const size_t ResY;
	const size_t BufferSize;
	const size_t BufferBytesFloat;
	const size_t BufferBytesUint32;

	float* pDiffA = nullptr;
	float* pDiffB = nullptr;
	float* pFeed = nullptr;
	float* pKill = nullptr;
};


struct REDI_V1_Buffers
{
	REDI_V1_Buffers(const size_t inBufferResX, const size_t inBufferResY)
		: BufferResX(inBufferResX)
		, BufferResY(inBufferResY)
		, BufferSize(inBufferResX * inBufferResY)
		, BufferBytesFloat(inBufferResX* inBufferResY * sizeof(float))
		, BufferBytesUint32(inBufferResX* inBufferResY * sizeof(uint32_t))
	{
		VIZ_ASSERT(0 < inBufferResX);
		VIZ_ASSERT(0 < inBufferResY);
	}

	const size_t BufferResX;
	const size_t BufferResY;		
	const size_t BufferSize;
	const size_t BufferBytesFloat;
	const size_t BufferBytesUint32;

	float* pBufferA = nullptr;
	float* pBufferB = nullptr;
	float* pBufferANext = nullptr;
	float* pBufferBNext = nullptr;

	float minValue = FLT_MAX;
	float maxValue = FLT_MIN;
};

bool REDI_V1_Buffers_Create(REDI_V1_Buffers* const);
void REDI_V1_Buffers_Destroy(REDI_V1_Buffers* const);
void REDI_V1_Buffers_Reset(REDI_V1_Buffers* const, const float fillChance);
void REDI_V1_Buffers_Calc_Range(REDI_V1_Buffers* const);
void REDI_V1_Buffers_Print_Range(REDI_V1_Buffers const * const);

// Just takes in the parameters, no maps
void REDI_V1_Buffers_Update(REDI_V1_Buffers* const, REDI_V1_Kernel const * const, REDI_V1_Parameters const * const, const bool wrapEdges = true);
void REDI_V1_Buffers_Update(REDI_V1_Buffers* const, REDI_V1_Kernel const * const, REDI_V1_Maps const * const, const bool wrapEdges = true);
