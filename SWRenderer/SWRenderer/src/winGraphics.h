#pragma once

#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

constexpr int WIDTH = 320;
constexpr int HEIGHT = 200;

extern int iPixelSizeX;
extern int iPixelSizeY;

uint32_t* Graphics_Init(HWND hwnd);
void Graphics_Draw( HWND hwnd, uint64_t iUpdateTimeNs, uint64_t iRenderTimeNs );
void Graphics_Present(HWND hwnd);

// Expose accessors for present buffer DC and dimensions
HDC Graphics_GetPresentDC();
void Graphics_GetPresentSize(int& outW, int& outH);

void Graphics_Shotdown();