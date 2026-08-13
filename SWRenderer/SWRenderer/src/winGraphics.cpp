#include "winGraphics.h"
#include "Common/Globals.h"

#include <stdio.h>

int iPixelSizeX = 7;
int iPixelSizeY = 7;

HBITMAP hBitmapFrameBuffer = nullptr;
HDC hDCFrameBuffer = nullptr;
uint32_t* pFramebuffer = nullptr;

HBITMAP hBitmapPresent = nullptr;
HDC hDCPresent = nullptr;

int iPresentW = 0;
int iPresentH = 0;

// ============================================================
// FRAMEBUFFER
// ============================================================

void CreateFramebuffer(HDC windowDC)
{
	if ( hBitmapFrameBuffer ) DeleteObject( hBitmapFrameBuffer );
	if ( hDCFrameBuffer ) DeleteDC( hDCFrameBuffer );

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = WIDTH;
	bmi.bmiHeader.biHeight = -HEIGHT;  // top-down
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	hDCFrameBuffer = CreateCompatibleDC(windowDC);

	hBitmapFrameBuffer = CreateDIBSection(windowDC, &bmi, DIB_RGB_COLORS, (void**)&pFramebuffer, nullptr, 0);

	SelectObject(hDCFrameBuffer, hBitmapFrameBuffer);
}

void CreatePresentBuffer(HDC windowDC, int width, int height)
{
	if (hBitmapPresent) DeleteObject(hBitmapPresent);
	if (hDCPresent) DeleteDC(hDCPresent);

	iPresentW = width;
	iPresentH = height;

	hDCPresent = CreateCompatibleDC(windowDC);
	hBitmapPresent = CreateCompatibleBitmap(windowDC, iPresentW, iPresentH);
	SelectObject(hDCPresent, hBitmapPresent);
}

uint32_t* Graphics_Init(HWND hwnd)
{
	HDC dc = GetDC(hwnd);
	CreateFramebuffer(dc);

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	CreatePresentBuffer(dc, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

	ReleaseDC(hwnd, dc);

	return pFramebuffer;	
}

void Graphics_Present(HWND hwnd, uint64_t iUpdateTimeNs, uint64_t iRenderTimeNs )
{
	RECT clientRect;
	GetClientRect( hwnd, &clientRect );

	int clientW = clientRect.right - clientRect.left;
	int clientH = clientRect.bottom - clientRect.top;

	if (clientW <= 0 || clientH <= 0)
		return;

	if (clientW != iPresentW || clientH != iPresentH)
	{
		HDC windowDC = GetDC(hwnd);
		CreatePresentBuffer(windowDC, clientW, clientH);
		ReleaseDC(hwnd, windowDC);
	}

	{
		int renderW = WIDTH * iPixelSizeX;
		int renderH = HEIGHT * iPixelSizeY;

		PatBlt( hDCPresent, 0, 0, clientW, clientH, BLACKNESS );

		int destX = (clientW - renderW) / 2;
		int destY = (clientH - renderH) / 2;

		SetStretchBltMode(hDCPresent, COLORONCOLOR);  // nearest neighbour
		StretchBlt( hDCPresent, destX, destY, renderW, renderH, hDCFrameBuffer, 0, 0, WIDTH, HEIGHT, SRCCOPY );
	}

	{
		SetBkMode(hDCPresent, TRANSPARENT);
		SetTextColor(hDCPresent, RGB(255, 255, 255));

		wchar_t msg[256];
		swprintf(msg, 256, L"Update %.3f ms (%.2f fps)", (double)iUpdateTimeNs/1000000.0, 1000000000.0/(double)iUpdateTimeNs );
		TextOut(hDCPresent, 0, 0, msg, (int)wcslen(msg));
		swprintf(msg, 256, L"Render(%dx%d) %.3f ms (%.2f fps)", WIDTH, HEIGHT, (double)iRenderTimeNs/1000000.0, 1000000000.0/(double)iRenderTimeNs);
		TextOut(hDCPresent, 0, 20, msg, (int)wcslen(msg));
	}

	{
		HDC windowDC = GetDC(hwnd);
		BitBlt(windowDC, 0, 0, clientW, clientH, hDCPresent, 0, 0, SRCCOPY);
		ReleaseDC(hwnd, windowDC);
	}
}
/*
void Graphics_Present(HWND hwnd, uint64_t iUpdateTimeNs, uint64_t iRenderTimeNs )
{
	{
		SetStretchBltMode(hDCPresent, COLORONCOLOR);  // nearest neighbour
		StretchBlt(hDCPresent, 0, 0, WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY, hDCFrameBuffer, 0, 0, WIDTH, HEIGHT, SRCCOPY);
	}

	{
		SetBkMode(hDCPresent, TRANSPARENT);
		SetTextColor(hDCPresent, RGB(255, 255, 255));

		wchar_t msg[256];
		swprintf(msg, 256, L"Update %.3f ms (%.2f fps)", (double)iUpdateTimeNs/1000000.0, 1000000000.0/(double)iUpdateTimeNs );
		TextOut(hDCPresent, 0, 0, msg, (int)wcslen(msg));
		swprintf(msg, 256, L"Render(%dx%d) %.3f ms (%.2f fps)", WIDTH, HEIGHT, (double)iRenderTimeNs/1000000.0, 1000000000.0/(double)iRenderTimeNs);
		TextOut(hDCPresent, 0, 20, msg, (int)wcslen(msg));
	}

	{
		HDC windowDC = GetDC(hwnd);
		BitBlt(windowDC, 0, 0, WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY, hDCPresent, 0, 0, SRCCOPY);
		ReleaseDC(hwnd, windowDC);
	}
}
*/
void Graphics_Shotdown()
{
	DeleteObject(hBitmapFrameBuffer);
	DeleteDC(hDCFrameBuffer);
}