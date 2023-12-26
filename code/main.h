
// cookiedough -- main header, include on top of each .cpp!

#ifndef _MAIN_H_
#define _MAIN_H_

#define _CRT_SECURE_NO_WARNINGS // tell MSVC to shut up about it's well-intentioned *_s() functions

// Rocket: def. for sync. replay (instead of edit) mode
#define SYNC_PLAYER

#include "platform.h"

#if defined(_WIN32)
	// ignore:
	#pragma warning(disable:4530)   // unwind semantics missing
#endif

// CRT & STL
#include <stdint.h>
#include <math.h>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <thread>

// OpenMP
#include <omp.h>

// list of industry aspect ratios
#include "../3rdparty/aspectratios.h"

// output resolution
constexpr size_t kResX = 1280;
constexpr size_t kResY = 720;
constexpr size_t kHalfResX = kResX/2;
constexpr size_t kHalfResY = kResY/2;
constexpr size_t kOutputSize = kResX*kResY;
constexpr size_t kOutputBytes = kOutputSize*sizeof(uint32_t);
constexpr float kAspect = (float)kResY/kResX;
constexpr float kOneOverAspect = 1.f/kAspect;
constexpr bool kFullScreen = false;

// GUI
#define GUI_ENABLED // comment out to remove gui from project
constexpr bool kGui_VisibleByDefault = true;

#if defined(GUI_ENABLED)
	#include "../3rdparty/imgui-1.90-docking/imgui.h"
	#include "../3rdparty/implot-0.16/implot.h"

	// we should probably disable GUI in fullscreen mode, for now
	constexpr bool kGuiEnabled = !kFullScreen;
#else
	constexpr bool kGuiEnabled = false;
#endif // GUI_ENABLED

#include "gui.h"

// set description on failure (reported on shutdown)
void SetLastError(const std::string &description);

// basic utilities (memory, graphics, ISSE et cetera)
#include "util.h"

// (few) shared resources
#include "shared-resources.h"


#endif // _MAIN_H_
