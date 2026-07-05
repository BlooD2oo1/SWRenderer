#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#include <atomic>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "winAudio.h"
#include "winGraphics.h"

#include "Common/Globals.h"
#include "Common/Perf.h"
#include "Engine/Engine.h"

//#define VSYNC
#ifdef VSYNC
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

// Frame cap: enable to cap to TARGET_FPS (uses a coarse Sleep + spin-wait for precision)
#define FRAME_CAP
#ifdef FRAME_CAP
#include <chrono>
#include <thread>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
constexpr double TARGET_FPS = 60.0;
constexpr double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;
#endif

std::atomic<bool> bRunning( true );

bool bLockMouse = true;

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
			if ( wParam == VK_ESCAPE )
			{
				bRunning = false;
				PostQuitMessage(0);
				return 0;
			}
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

			//scroll wheel:
		case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			CEngine::GetInstance().On_MouseWheel(zDelta);
			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void AudioThread()
{
	if ( Audio_Init() == false )
	{
		return;
	}

	while (bRunning)
	{
		SAudioBuffer sAudioBuffer;
		Audio_UpdateBegin( sAudioBuffer.pData, sAudioBuffer.iNumFrames, sAudioBuffer.iSampleRate );
		CEngine::GetInstance().UpdateAudioThread( sAudioBuffer );
		Audio_UpdateEnd( sAudioBuffer.iNumFrames );
	}
	
	Audio_Shutdown();
}

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

	uint32_t* pFrameBuffer = Graphics_Init( hwnd );

	SFrameBuffer sFrameBuffer(pFrameBuffer, WIDTH, HEIGHT);
	CEngine::CreateInstance();
	CEngine::GetInstance().Create(sFrameBuffer);

	std::thread audioThread(AudioThread);

#ifdef FRAME_CAP
	// improve Sleep resolution for more accurate sleep durations
	timeBeginPeriod(1);
#endif

	float fElapsedTimeMs = 0.0f;
	float fRenderTimeMs = 0.0f;
	while (bRunning)
	{
		// high-resolution frame start timestamp (used for frame-capping)
#ifdef FRAME_CAP
		auto frameStart = std::chrono::high_resolution_clock::now();
#endif

		CPerf cPerfFrame;
		cPerfFrame.BeginPerf();

		{
			wchar_t title[256];
			swprintf(title, 256, L" SWRenderer - %dx%d %.2f ms (%.2f fps)", WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY, fElapsedTimeMs, 1000.0 / fElapsedTimeMs);
			SetWindowText(hwnd, title);
		}

#ifdef VSYNC
		DwmFlush();
#endif
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		CEngine::GetInstance().Update( fElapsedTimeMs );

		CPerf cPerfRender;
		cPerfRender.BeginPerf();
		CEngine::GetInstance().Render();
		//Sleep( 10 );
		fRenderTimeMs = (float)( cPerfRender.EndPerf() * 1000.0 );

		Graphics_Present(hwnd, fRenderTimeMs);

#ifdef FRAME_CAP
		// frameTime is measured by CPerf; we'll use chrono for the wait so it's independent of the perf helper.
		auto now = std::chrono::high_resolution_clock::now();
		double elapsed = std::chrono::duration<double>(now - frameStart).count();

		if (elapsed < TARGET_FRAME_TIME)
		{
			double remaining = TARGET_FRAME_TIME - elapsed;

			// coarse sleep for the integer-millisecond portion (leave ~1ms margin), then spin-wait
			if (remaining > 0.003) // larger than ~3us margin
			{
				DWORD ms = (DWORD)((remaining - 0.001) * 1000.0);
				if (ms > 0)
					Sleep(ms);
			}

			// busy-wait until exact target time (use yield to be nicer to scheduler)
			while (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - frameStart).count() < TARGET_FRAME_TIME)
			{
				std::this_thread::yield();
			}
		}
#endif
		fElapsedTimeMs = (float)( cPerfFrame.EndPerf() * 1000.0 );
	}

	if (audioThread.joinable())
	{
		audioThread.join(); // Új: megvárja, amíg az AudioThread lefut
	}

	CEngine::Destroy();

	Graphics_Shotdown();

#ifdef FRAME_CAP
	timeEndPeriod(1);
#endif

	return 0;
}