#include "main.h"
#include "image.h"
#include "fx-blitter.h"
#include "random.h"
#include "rocket.h"

#include "eh-util.h"
#include "reaction-diffusion-v1.h"
#include "reaction-diffusion-v1-effect.h"

// For saving parameters
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <format>

using namespace ErnstHot;

constexpr bool kHalfRes = true;

static REDI_V1_Kernel*		s_pKernel		= nullptr;
static REDI_V1_Parameters*	s_pParameters	= nullptr;
static REDI_V1_Buffers*		s_pBuffers		= nullptr;

static bool s_updateBuffers = true;
static float s_centerPixelValue = 0.0f;
static float s_shape = 0.0f;
static float s_gain = 0.0f;

SyncTrack trackRediClearBuffer;


// Private functions //////////////////////////////////////////////////////////

static std::string getCurrentTimeAndDate()
{
	auto const time = std::chrono::current_zone()
		->to_local(std::chrono::system_clock::now());
	return std::format("{:%Y-%m-%d %X}", time);
}


static void showGui()
{
#if defined(GUI_ENABLED)
	if (Gui_Is_Visible())
	{
		if (ImGui::CollapsingHeader("Reaction diffusion V1", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SeparatorText("Options");
				ImGui::Checkbox("Update", &s_updateBuffers);
				ImGui::SameLine();

				if (ImGui::Button("Reset"))
					REDI_V1_Buffers_Reset(s_pBuffers, s_pParameters->fillChance);

				ImGui::SliderFloat("Center pixel", &s_centerPixelValue, 0.0f, 1.0f, "%.3f");
				ImGui::SliderFloat("Fill Chance", &s_pParameters->fillChance, 0.0f, 1.0f, "%.3f");

			ImGui::SeparatorText("Parameters");
				//ImGui::DragFloat("drag small float", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");

				ImGui::SliderFloat("Diffuse A", &s_pParameters->diffA, 0.0f, 1.0f, "%.4f");
				ImGui::SliderFloat("Diffuse B", &s_pParameters->diffB, 0.0f, 1.0f, "%.4f");
				ImGui::SliderFloat("Feed", &s_pParameters->feed, 0.0f, 0.1f, "%.6f");
				ImGui::SliderFloat("Kill", &s_pParameters->kill, 0.0f, 0.1f, "%.6f");

				if (ImGui::Button("Save Parameters"))
				{
					std::fstream f;
					f.open("REDI_V1_Saved Parameters.txt", std::ios::out | std::ios::app);

					if (f)
					{
						f << std::fixed;
						f << std::setprecision(7);

						f << "Time: " << getCurrentTimeAndDate() << "\n";
						f << "Parameters:\n";
						f << "\tfloat diffA = " << s_pParameters->diffA << ";\n";
						f << "\tfloat diffB = " << s_pParameters->diffB << ";\n";
						f << "\tfloat feed = " << s_pParameters->feed << ";\n";
						f << "\tfloat kill = " << s_pParameters->kill << ";\n";
						f << "\n";

						f.close();
					} // else oops.
				}

			ImGui::SeparatorText("Brightness / contrast");
				ImGui::SliderFloat("Shape", &s_shape, -0.999f, 0.999f, "%.3f");
				ImGui::SliderFloat("Gain", &s_gain, -1.0f, 1.0f, "%.3f");
		}
	}
#endif // GUI_ENABLED
}

static void clearBuffers()
{
	const int syncClear = Rocket::geti(trackRediClearBuffer);

	static bool OKToInitBuffers = true;

	if (OKToInitBuffers && syncClear > 0)
	{
		REDI_V1_Buffers_Reset(s_pBuffers, s_pParameters->fillChance);
		OKToInitBuffers = false;
	}

	if (!OKToInitBuffers && syncClear == 0)
	{
		OKToInitBuffers = true;
	}
}


// Public functions //////////////////////////////////////////////////////////

void Reaction_Diffusion_V1_Effect_Draw(uint32_t* pDest, float time, float delta)
{
	clearBuffers();
	showGui();

	const float shape = s_shape * 0.5f + 0.5f;
	const float gain = expf(s_gain * 4.0f);

	if (s_updateBuffers)
		REDI_V1_Buffers_Update(s_pBuffers, s_pKernel, s_pParameters, true);

	if (s_centerPixelValue > 0.0f)
		s_pBuffers->pBufferB[s_pBuffers->BufferSize / 2 + s_pBuffers->BufferResX / 2] = s_centerPixelValue;

	const size_t sourceResX = s_pBuffers->BufferResX;
	const size_t sourceResY = s_pBuffers->BufferResY;
	const size_t destinationResX = kHalfRes ? kHalfResX : kResX;
	const size_t destinationResY = kHalfRes ? kHalfResY : kResY;

	for (size_t yIdx = 0; yIdx < destinationResY; ++yIdx)
	{
		for (size_t xIdx = 0; xIdx < destinationResX; ++xIdx)
		{
			const size_t srcXidx = xIdx - (destinationResX / 2 - sourceResX / 2);
			const size_t srcYidx = yIdx - (destinationResY / 2 - sourceResY / 2);

			uint32_t color = 0xff080808;

			if (!(srcXidx >= sourceResX || srcYidx >= sourceResY || srcXidx < 0 || srcYidx < 0))
			{
				float vf = 1.0f - s_pBuffers->pBufferA[XYToIdx(srcXidx, srcYidx, sourceResX)];
				vf = gain * VarShape(vf, shape);

				//float vf = lutsinf(kPI +  4.0f * s_pBuffers->pBufferA[XYToIdx(srcXidx, srcYidx, sourceResX)]) * 0.5f + 0.5f;
				color = FloatToColorGreyscale32(vf);
				//color = FloatToColorSpectrum32(3+vf * 6, 0.6f);
			}

			if (kHalfRes)
				g_pFxMap[0][XYToIdx(xIdx, yIdx, destinationResX)] = color;
			else
				pDest[XYToIdx(xIdx, yIdx, destinationResX)] = color;
		}
	}

	if (kHalfRes)
		Fx_Blit_2x2(pDest, g_pFxMap[0]);
}

bool Reaction_Diffusion_V1_Effect_Create()
{
	s_pKernel = static_cast<REDI_V1_Kernel*>(mallocAligned(sizeof(REDI_V1_Kernel), kAlignTo));
	s_pParameters = static_cast<REDI_V1_Parameters*>(mallocAligned(sizeof(REDI_V1_Parameters), kAlignTo));
	s_pBuffers = static_cast<REDI_V1_Buffers*>(mallocAligned(sizeof(REDI_V1_Buffers), kAlignTo));

	s_pKernel = new REDI_V1_Kernel();
	s_pParameters = new REDI_V1_Parameters();

	const size_t bufferWidth = kHalfRes ? 256 : 512;

	s_pBuffers = new REDI_V1_Buffers(bufferWidth, bufferWidth);

	if (nullptr == s_pKernel
		|| nullptr == s_pParameters
		|| nullptr == s_pBuffers
		)	return false;


	// Kernel
	REDI_V1_Kernel_Create(s_pKernel);
	REDI_V1_Kernel_Generate(s_pKernel);

	// Bend the kernel a bit
	const float mul = 0.1f;
	s_pKernel->pData[0] += 0.025f * mul;
	s_pKernel->pData[1] += 0.2f * mul;
	s_pKernel->pData[2] += 0.025f * mul;

	s_pKernel->pData[3] -= 0.0125f * mul;
	s_pKernel->pData[5] -= 0.0125f * mul;

	s_pKernel->pData[6] += 0.025f * mul;
	s_pKernel->pData[7] -= 0.2f * mul;
	s_pKernel->pData[8] += 0.025f * mul;

	REDI_V1_Kernel_Normalize(s_pKernel);

	// Parameters
	s_pParameters->diffA = 0.8620999;
	s_pParameters->diffB = 0.3366999;
	s_pParameters->feed = 0.0131419;
	s_pParameters->kill = 0.0439979;
	s_pParameters->fillChance = 0.001;

	// Buffers
	if (!REDI_V1_Buffers_Create(s_pBuffers))
		return false;

	REDI_V1_Buffers_Reset(s_pBuffers, s_pParameters->fillChance);

	// Tracks
	trackRediClearBuffer = Rocket::AddTrack("redi:Clear");

	return true;
}

void Reaction_Diffusion_V1_Effect_Destroy()
{
	REDI_V1_Buffers_Print_Range(s_pBuffers);
	REDI_V1_Buffers_Destroy(s_pBuffers);
	REDI_V1_Kernel_Destroy(s_pKernel);

	delete s_pKernel;
	delete s_pParameters;
	delete s_pBuffers;
}
