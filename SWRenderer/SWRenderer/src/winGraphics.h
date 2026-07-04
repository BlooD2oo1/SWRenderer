#pragma once

#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

constexpr int WIDTH = 320;
constexpr int HEIGHT = 200;

constexpr int iPixelSizeX = 5;
constexpr int iPixelSizeY = 5;

uint32_t* Graphics_Init(HWND hwnd);
void Graphics_Present(HWND hwnd, float fRenderTimeMs);
void Graphics_Shotdown();