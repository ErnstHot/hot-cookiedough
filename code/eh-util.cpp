#include <stdint.h>
#include <stdlib.h>
#include <utility>

#include "main.h"
#include "eh-util.h"


namespace ErnstHot
{
	void Uint8BufferToFloatBuffer(float* pDestination, uint8_t* pSource, size_t numPixels)
	{
		const float r8bitMax = 1.0f / 255.0f;

		for (size_t i = 0; i < numPixels; i++)
			pDestination[i] = (float)pSource[i] * r8bitMax;
	}
} // namespace ErnstHot