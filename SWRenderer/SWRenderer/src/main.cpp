#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "Common/Globals.h"
#include "Common/Perf.h"
#include "Engine/Engine.h"

// ============================================================
// CONFIG
// ============================================================

constexpr int WIDTH = 320;
constexpr int HEIGHT = 200;

constexpr int iPixelSizeX = 5;
constexpr int iPixelSizeY = 5;

// ============================================================
// GLOBALS
// ============================================================

bool bRunning = true;

bool bLockMouse = true;

double fRenderTime = 0.0;

HBITMAP hBitmapFrameBuffer = nullptr;
HDC hDCFrameBuffer = nullptr;
uint32_t* pFramebuffer = nullptr;

HBITMAP hBitmapPresent = nullptr;
HDC hDCPresent = nullptr;

// ============================================================
// DPI AWARENESS
// ============================================================

void EnableDPIAwareness()
{
	HMODULE user32 = LoadLibraryA("user32.dll");

	if (user32)
	{
		using SetDpiAwarenessContext_t = DPI_AWARENESS_CONTEXT(WINAPI*)(DPI_AWARENESS_CONTEXT);

		auto SetThreadDpiAwarenessContext = (SetDpiAwarenessContext_t)GetProcAddress(user32, "SetThreadDpiAwarenessContext");

		if (SetThreadDpiAwarenessContext)
		{
			SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		}

		FreeLibrary(user32);
	}
}

// ============================================================
// WINDOW PROC
// ============================================================

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CLOSE:
		case WM_DESTROY:
		{
			bRunning = false;
			PostQuitMessage(0);
			return 0;
		}

		case WM_SETCURSOR:
		{
			if (LOWORD(lParam) == HTCLIENT)
			{
				if ( bLockMouse )
				{
					SetCursor(nullptr);
					return TRUE;
				}
			}
		}

		case WM_KEYDOWN:
		{
			CEngine::GetInstance().On_KeyDown((uint32_t)wParam);
			return 0;
		}

		case WM_KEYUP:
		{
			CEngine::GetInstance().On_KeyUp((uint32_t)wParam);
			return 0;
		}

		case WM_MOUSEMOVE:
		{
			int iX = GET_X_LPARAM(lParam);
			int iY = GET_Y_LPARAM(lParam);

			if ( bLockMouse )
			{
				int centerX = (WIDTH * iPixelSizeX) / 2;
				int centerY = (HEIGHT * iPixelSizeY) / 2;

				if (iX != centerX || iY != centerY)
				{
					CEngine::GetInstance().On_MouseMove( iX - centerX, iY - centerY );

					POINT pt = { centerX, centerY };
					ClientToScreen(hwnd, &pt);
					SetCursorPos(pt.x, pt.y);
				}
			}

			
			return 0;
		}

		case WM_LBUTTONDOWN:
			CEngine::GetInstance().On_MouseButtonDown(0);
			return 0;

		case WM_LBUTTONUP:
			CEngine::GetInstance().On_MouseButtonUp(0);
			return 0;

		case WM_MBUTTONDOWN:
			CEngine::GetInstance().On_MouseButtonDown(2);
			bLockMouse = !bLockMouse;
			if ( !bLockMouse )
			{
				HCURSOR hDefault = LoadCursor(nullptr, IDC_ARROW);
				SetCursor(hDefault);
			}
			return 0;

		case WM_MBUTTONUP:
			CEngine::GetInstance().On_MouseButtonUp(2);
			return 0;

		case WM_RBUTTONDOWN:
			CEngine::GetInstance().On_MouseButtonDown(1);
			return 0;

		case WM_RBUTTONUP:
			CEngine::GetInstance().On_MouseButtonUp(1);
			return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

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

// ============================================================
// PRESENT
// ============================================================

void Present(HWND hwnd)
{
	{
		SetStretchBltMode(hDCPresent, COLORONCOLOR);  // nearest neighbour
		StretchBlt(hDCPresent, 0, 0, WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY, hDCFrameBuffer, 0, 0, WIDTH, HEIGHT, SRCCOPY);
	}

	{
		wchar_t msg[256];
		swprintf(msg, 256, L" Render(%dx%d) - %.3f ms (%.2f fps)", WIDTH, HEIGHT, fRenderTime * 1000, 1.0 / fRenderTime);
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

// ============================================================
// MAIN
// ============================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	EnableDPIAwareness();

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"MinimalWindow";

	RegisterClass(&wc);

	DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	RECT rect = {0, 0, WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY};

	AdjustWindowRect(&rect, style, FALSE);

	HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"SWRenderer", style, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);

	ShowWindow(hwnd, SW_SHOW);

	HDC dc = GetDC(hwnd);
	CreateFramebuffer(dc);
	CreatePresentBuffer(dc);
	ReleaseDC(hwnd, dc);

	MSG msg = {};

	SFrameBuffer sFrameBuffer(pFramebuffer, WIDTH, HEIGHT);
	CEngine::CreateInstance();
	CEngine::GetInstance().Create(sFrameBuffer);

	while (bRunning)
	{
		CPerf cPerfFrame;

		cPerfFrame.BeginPerf();

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		CEngine::GetInstance().Update();

		CPerf cPerfRender;
		cPerfRender.BeginPerf();
		CEngine::GetInstance().Render();
		fRenderTime = cPerfRender.EndPerf();

		Present(hwnd);

		double frameTime = cPerfFrame.EndPerf();

		{
			wchar_t title[256];
			swprintf(title, 256, L" SWRenderer - %dx%d - %.2f ms (%.2f fps)", WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY, frameTime * 1000, 1.0 / frameTime);
			SetWindowText(hwnd, title);
		}
	}

	CEngine::Destroy();

	DeleteObject(hBitmapFrameBuffer);
	DeleteDC(hDCFrameBuffer);

	return 0;
}