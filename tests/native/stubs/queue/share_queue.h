#pragma once

#include <cstdint>
#include <cstdlib>

// Minimal queue and clock seam for the real controller.cpp. No Windows APIs run.
struct share_queue { bool open; };
struct LARGE_INTEGER { long long QuadPart; };
inline void QueryPerformanceFrequency(LARGE_INTEGER* value) { value->QuadPart = 1000; }
inline void QueryPerformanceCounter(LARGE_INTEGER* value) { value->QuadPart = 1000; }
constexpr int ModeVideo = 0;
constexpr int AV_PIX_FMT_RGBA = 26;
