#include "winGraphics.h"
#include "Common/Globals.h"

HBITMAP hBitmapFrameBuffer = nullptr;
HDC hDCFrameBuffer = nullptr;
uint32_t* pFramebuffer = nullptr;

HBITMAP hBitmapPresent = nullptr;
HDC hDCPresent = nullptr;

// ============================================================
// FRAMEBUFFER
// ============================================================

void CreateFramebuffer(HDC windowDC)
{
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

void CreatePresentBuffer(HDC windowDC)
{
	hDCPresent = CreateCompatibleDC(windowDC);

	hBitmapPresent = CreateCompatibleBitmap(windowDC, WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY);

	SelectObject(hDCPresent, hBitmapPresent);
}

uint32_t* Graphics_Init(HWND hwnd)
{
	HDC dc = GetDC(hwnd);
	CreateFramebuffer(dc);
	CreatePresentBuffer(dc);
	ReleaseDC(hwnd, dc);
	return pFramebuffer;	
}

void Graphics_Present(HWND hwnd, uint64_t iRenderTimeNs )
{
	{
		SetStretchBltMode(hDCPresent, COLORONCOLOR);  // nearest neighbour
		StretchBlt(hDCPresent, 0, 0, WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY, hDCFrameBuffer, 0, 0, WIDTH, HEIGHT, SRCCOPY);
	}

	{
		wchar_t msg[256];
		swprintf(msg, 256, L" Render(%dx%d) %.3f ms (%.2f fps)", WIDTH, HEIGHT, (double)iRenderTimeNs/1000000.0, 1000000000.0/(double)iRenderTimeNs);
		SetBkMode(hDCPresent, TRANSPARENT);
		SetTextColor(hDCPresent, RGB(255, 255, 255));
		TextOut(hDCPresent, 0, 0, msg, (int)wcslen(msg));
	}

	{
		HDC windowDC = GetDC(hwnd);
		BitBlt(windowDC, 0, 0, WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY, hDCPresent, 0, 0, SRCCOPY);
		ReleaseDC(hwnd, windowDC);
	}
}

void Graphics_Shotdown()
{
	DeleteObject(hBitmapFrameBuffer);
	DeleteDC(hDCFrameBuffer);
}