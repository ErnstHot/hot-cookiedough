
// cookiedough -- simple Shadertoy ports

/*
	to do:
		- fix a *working* OpenMP implementation of the plasma (are it the split writes in 64-bit?)
		- a minor optimization is to get offsets and deltas to calculate current UV, but that won't parallelize with OpenMP
		- just optimize as needed, since all of this is a tad slow
*/

#include "main.h"
// #include "shadertoy.h"
#include "image.h"
// #include "bilinear.h"
#include "shadertoy-util.h"
#include "boxblur.h"

bool Shadertoy_Create()
{
	return true;
}

void Shadertoy_Destroy()
{
}

//
// Plasma (https://www.shadertoy.com/view/ldSfzm)
//

VIZ_INLINE float fPlasma(Vector3 point, float time)
{
	point.z += 5.f*time;
	const float sine = 0.2f*lutsinf(point.x-point.y);
	const float fX = sine + lutcosf(point.x*0.33f);
	const float fY = sine + lutcosf(point.y*0.33f);
	const float fZ = sine + lutcosf(point.z*0.33f);
//	return sqrtf(fX*fX + fY*fY + fZ*fZ)-0.8f;
	return 1.f/Q_rsqrt(fX*fX + fY*fY + fZ*fZ)-0.8f;
}

static void RenderPlasmaMap(uint32_t *pDest, float time)
{
	__m128i *pDest128 = reinterpret_cast<__m128i*>(pDest);

	const Vector3 colMulA(0.1f, 0.15f, 0.3f);
	const Vector3 colMulB(0.05f, 0.05f, 0.1f);

	#pragma omp parallel for schedule(static)
	for (int iY = 0; iY < kFineResY; ++iY)
	{
		const int yIndex = iY*kFineResX;
		for (int iX = 0; iX < kFineResX; iX += 4)
		{	
			Vector4 colors[4];
			for (int iColor = 0; iColor < 4; ++iColor)
			{

				auto& UV = Shadertoy::ToUV_FX_2x2(iX+iColor, iY, 2.f);

				const int cosIndex = tocosindex(time*0.314f);
				const float dirCos = lutcosf(cosIndex);
				const float dirSin = lutsinf(cosIndex);

				Vector3 direction(
					dirCos*UV.x - dirSin*0.6f,
					UV.y,
					dirSin*UV.x + dirCos*0.6f);

//				Vector3 direction(Shadertoy::ToUV_FX_2x2(iX+iColor, iY, 2.f), 0.6f);

				Vector3 origin = direction;
				for (int step = 0; step < 46; ++step)
					origin += direction*fPlasma(origin, time);
				
				colors[iColor] = colMulA*fPlasma(origin+direction, time) + colMulB*fPlasma(origin*0.5f, time); 
				colors[iColor] *= 8.f - origin.x*0.5f;
			}

			const int index = (yIndex+iX)>>2;
			pDest128[index] = Shadertoy::ToPixel4(colors);
		}
	}
}

void Plasma_Draw(uint32_t *pDest, float time, float delta)
{
	RenderPlasmaMap(g_pFXFine, time);
	MapBlitter_Colors_2x2(pDest, g_pFXFine);
//	HorizontalBoxBlur32(pDest, pDest, kResX, kResY, 0.01f);
}

//
// Nautilus Redux by Michiel v/d Berg
//

VIZ_INLINE float fNautilus(const Vector3 &position, float time)
{
	float cosX, cosY, cosZ;
	float cosTime7 = lutcosf(time/7.f); // ** i'm getting an internal compiler error if I write this where it should be **
	cosX = lutcosf(lutcosf(position.x + time*0.125f)*position.x - lutcosf(position.y + time/9.f)*position.y);
	cosY = lutcosf(position.z*0.33f*position.x - cosTime7*position.y);
	cosZ = lutcosf(position.x + position.y + position.z/1.25f + time);
	const float dotted = cosX*cosX + cosY*cosY + cosZ*cosZ;
	return dotted*0.5f - .7f;
};

static void RenderNautilusMap_2x2(uint32_t *pDest, float time)
{
	__m128i *pDest128 = reinterpret_cast<__m128i*>(pDest);

	const Vector3 colorization(
		.1f-lutcosf(time/3.f)/19.f, 
		.1f, 
		.1f+lutcosf(time/14.f)/8.f);

	#pragma omp parallel for schedule(static)
	for (int iY = 0; iY < kFineResY; ++iY)
	{
		const int yIndex = iY*kFineResX;
		for (int iX = 0; iX < kFineResX; iX += 4)
		{	
			Vector4 colors[4];
			for (int iColor = 0; iColor < 4; ++iColor)
			{
				auto UV = Shadertoy::ToUV_FX_2x2(iColor+iX, iY, 1.614f);

				Vector3 origin(0.f);
				Vector3 direction(UV.x, UV.y, 1.f); 
				direction.Normalize();

				Shadertoy::rot2D(kPI*cos(time*0.06234f), direction.x, direction.y);

				Vector3 hit(0.f);

				float march, total = 0.f;
				for (int iStep = 0; iStep < 64; ++iStep)
				{
					hit = origin + direction*total;
					march = fNautilus(hit, time);
					total += march*0.7f;

//					if (fabsf(march) < 0.001f*(total*0.125f + 1.f) || total>20.f)
//						break;
				}

				float nOffs = 0.1f;
				Vector3 normal(
					march-fNautilus(Vector3(hit.x+nOffs, hit.y, hit.z), time),
					march-fNautilus(Vector3(hit.x, hit.y+nOffs, hit.z), time),
					march-fNautilus(Vector3(hit.x, hit.y, hit.z+nOffs), time));

				float diffuse = -0.5f*normal.z + -0.5f*normal.y + 0.5f*normal.z;

				// gebruik normal en hit om slopes uit te rekenen!				

				Vector3 color(diffuse*0.1f);
				color += colorization*(1.56f*total);

				const float gamma = 2.20f;
				color.x = powf(color.x, gamma);
				color.y = powf(color.y, gamma);
				color.z = powf(color.z, gamma);

//				colors[iColor].vSIMD = color.vSIMD;
				colors[iColor].vSIMD = Vector3(diffuse).vSIMD;
			}

			const int index = (yIndex+iX)>>2;
			pDest128[index] = Shadertoy::ToPixel4(colors);
		}
	}
}

static void RenderNautilusMap_4x4(uint32_t *pDest, float time)
{
	// FIXME: implement
	VIZ_ASSERT(false);
}

void Nautilus_Draw(uint32_t *pDest, float time, float delta)
{
	RenderNautilusMap_2x2(g_pFXFine, time);
	
	// FIXME: this looks amazing if timed!
	// HorizontalBoxBlur32(g_pFXFine, g_pFXFine, kFineResX, kFineResY, fabsf(0.0314f*2.f*sin(time)));
	
	MapBlitter_Colors_2x2(pDest, g_pFXFine);

//	RenderNautilusMap_4x4(g_pFXCoarse, time);
//	MapBlitter_Colors_4x4(pDest, g_pFXCoarse);
}
