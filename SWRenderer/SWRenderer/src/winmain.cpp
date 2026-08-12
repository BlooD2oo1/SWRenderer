#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include "winAudio.h"
#include "winGraphics.h"

#include "Common/Globals.h"
#include "Common/Threading.h"
#include "Common/Time.h"
#include "Engine/Engine.h"

//#define VSYNC
#ifdef VSYNC
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

// Frame cap: enable to cap to TARGET_FPS (uses a coarse Sleep + spin-wait for precision)
#define FRAME_CAP
#ifdef FRAME_CAP
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
constexpr double TARGET_FPS = 60.0;
constexpr double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;
#endif

bool bFullScreenBorderless = false;

bool bLockMouse = true;

static WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

void ToggleFullscreen(HWND hwnd)
{
	DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

	if (!bFullScreenBorderless)
	{
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(hwnd, &g_wpPrev) &&
			GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
		{
			SetWindowLong(hwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW) | WS_POPUP);
			SetWindowPos(hwnd, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
		}
		bFullScreenBorderless = true;
	}
	else
	{
		SetWindowLong(hwnd, GWL_STYLE, (dwStyle & ~WS_POPUP) | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(hwnd, &g_wpPrev);
		SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
		bFullScreenBorderless = false;
	}
}

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
			g_bRunning = false;
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

		case WM_SYSKEYDOWN:
		{
			if (wParam == VK_RETURN && (lParam & (1 << 29)))
			{
				ToggleFullscreen(hwnd);
				return 0;
			}
			break;
		}

		case WM_KEYDOWN:
		{
			/*if ( wParam == VK_ESCAPE )
			{
				g_bRunning = false;
				return 0;
			}*/
			if ( wParam == VK_SUBTRACT )
			{
				iPixelSizeX = std::max(1, iPixelSizeX - 1);
				iPixelSizeY = std::max(1, iPixelSizeY - 1);
				return 0;
			}
			if ( wParam == VK_ADD )
			{
				iPixelSizeX = std::min(10, iPixelSizeX + 1);
				iPixelSizeY = std::min(10, iPixelSizeY + 1);
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
				RECT clientRect;
				GetClientRect(hwnd, &clientRect);
				int centerX = (clientRect.right - clientRect.left) / 2;
				int centerY = (clientRect.bottom - clientRect.top) / 2;

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

	while (g_bRunning)
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

	DWORD style = 0;
	DWORD exStyle = 0;
	int posX = CW_USEDEFAULT;
	int posY = CW_USEDEFAULT;
	int windowWidth = 0;
	int windowHeight = 0;

	if (bFullScreenBorderless)
	{
		RECT rect = { 0, 0, WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY };
		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;

		g_wpPrev.length = sizeof(WINDOWPLACEMENT);
		g_wpPrev.flags = 0;
		g_wpPrev.showCmd = SW_SHOWNORMAL;
		g_wpPrev.rcNormalPosition = { 100, 100, 100 + w, 100 + h };

		style = WS_POPUP | WS_VISIBLE;
		exStyle = WS_EX_APPWINDOW;
		posX = 0;
		posY = 0;
		windowWidth = GetSystemMetrics(SM_CXSCREEN);
		windowHeight = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		style = WS_OVERLAPPEDWINDOW;
		//style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		RECT rect = { 0, 0, WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY };
		AdjustWindowRect(&rect, style, FALSE);
		windowWidth = rect.right - rect.left;
		windowHeight = rect.bottom - rect.top;
	}

	HWND hwnd = CreateWindowEx(
		exStyle, wc.lpszClassName, L"SWRenderer", style,
		posX, posY, windowWidth, windowHeight,
		nullptr, nullptr, hInstance, nullptr
	);

	ShowWindow(hwnd, SW_SHOW);

	uint32_t* pFrameBuffer = Graphics_Init( hwnd );

	SFrameBuffer sFrameBuffer((BGRA8*)pFrameBuffer, WIDTH, HEIGHT);
	CEngine::CreateInstance();
	CEngine::GetInstance().Create(sFrameBuffer);

	std::thread audioThread(AudioThread);

#ifdef FRAME_CAP
	// improve Sleep resolution for more accurate sleep durations
	timeBeginPeriod(1);
#endif

	uint64_t iElapsedTimeNs = 0;

	while (g_bRunning)
	{
		// high-resolution frame start timestamp (used for frame-capping)
#ifdef FRAME_CAP
		auto frameStart = std::chrono::high_resolution_clock::now();
#endif

		CPerf cPerfFrame;
		cPerfFrame.BeginPerf();

		/*{
			wchar_t title[256];
			swprintf(title, 256, L" SWRenderer %dx%d %.2f ms (%.2f fps)", WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY, (double)iElapsedTimeNs/1000000.0, 1000000000.0/(double)iElapsedTimeNs);
			SetWindowText(hwnd, title);
		}*/

#ifdef VSYNC
		DwmFlush();
#endif
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		CPerf cPerfUpdate;
		cPerfUpdate.BeginPerf();		
		CEngine::GetInstance().Update();
		uint64_t iUpdateTimeNs = cPerfUpdate.EndPerfNs();

		CPerf cPerfRender;
		cPerfRender.BeginPerf();
		CEngine::GetInstance().Render();
		uint64_t iRenderTimeNs = cPerfRender.EndPerfNs();

		Graphics_Present(hwnd, iUpdateTimeNs, iRenderTimeNs);

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
		iElapsedTimeNs = cPerfFrame.EndPerfNs();
	}

	if (audioThread.joinable())
	{
		audioThread.join();
	}

	CEngine::Destroy();

	Graphics_Shotdown();

#ifdef FRAME_CAP
	timeEndPeriod(1);
#endif

	return 0;
}