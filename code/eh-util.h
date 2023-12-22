
// Thorsten's utility functions
//	Feel free to move these somewhere else for consistency or whatever.

#pragma once

namespace ErnstHot
{
	void Uint8BufferToFloatBuffer(float* pDestinationBuffer, uint8_t* pSourceBuffer, size_t bufferSize);


	template <typename T>
	VIZ_INLINE T WrapBipolar(T value, T limit)
	{
		return (value + limit) % limit;
	}

	VIZ_INLINE size_t XYToIdx(size_t x, size_t y, size_t resX)
	{
		return y * resX + x;
	}

	VIZ_INLINE size_t XYToIdxWrap(int32_t x, int32_t y, size_t resX, size_t resY)
	{
		return XYToIdx(
			static_cast<size_t>(WrapBipolar(x, static_cast<int32_t>(resX))),
			static_cast<size_t>(WrapBipolar(y, static_cast<int32_t>(resY))),
			resX);
	}

	VIZ_INLINE uint32_t FloatToColorGreyscale32(float value)
	{
		const uint32_t vi = (uint32_t)(saturatef(value) * 255.0f);

		return vi << 16 | vi << 8 | vi;
	}

	VIZ_INLINE uint32_t FloatToColorSpectrum32(float value, float spread)
	{
		const uint32_t r = (uint32_t)(lutsinf(value) * 127.0f + 128.0f);
		const uint32_t g = (uint32_t)(lutsinf(value + spread) * 127.0f + 128.0f);
		const uint32_t b = (uint32_t)(lutsinf(value + spread + spread) * 127.0f + 128.0f);

		return r << 16 | g << 8 | b;
	}

	template <typename T>
	VIZ_INLINE T VarShape(const T& x, const T& shape)
	{
		return x / (x + (1 - (1 / shape)) * (x - 1));
	}

} // namespace ErnstHot
