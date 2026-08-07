#pragma once

#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

constexpr int WIDTH = 640;
constexpr int HEIGHT = 480;

constexpr int iPixelSizeX = 2;
constexpr int iPixelSizeY = 2;

uint32_t* Graphics_Init(HWND hwnd);
void Graphics_Present(HWND hwnd, uint64_t iRenderTimeNs );
void Graphics_Shotdown();