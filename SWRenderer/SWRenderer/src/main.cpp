#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "Engine/Common/Globals.h"
#include "Engine/Common/Perf.h"
#include "Engine/Engine.h"

// ============================================================
// CONFIG
// ============================================================

constexpr int WIDTH  = 320;
constexpr int HEIGHT = 200;

constexpr int iPixelSizeX = 3;
constexpr int iPixelSizeY = 3;

// ============================================================
// GLOBALS
// ============================================================

bool gRunning = true;

double fRenderTime = 0.0;

HBITMAP gBitmapFrameBuffer = nullptr;
HDC     gDCFrameBuffer = nullptr;
uint32_t* gFramebuffer = nullptr;

HBITMAP gBitmapPresent = nullptr;
HDC     gDCPresent = nullptr;



// ============================================================
// DPI AWARENESS
// ============================================================

void EnableDPIAwareness()
{
    HMODULE user32 =
        LoadLibraryA("user32.dll");

    if (user32)
    {
        using SetDpiAwarenessContext_t =
            DPI_AWARENESS_CONTEXT(WINAPI*)(DPI_AWARENESS_CONTEXT);

        auto SetThreadDpiAwarenessContext =
            (SetDpiAwarenessContext_t)
            GetProcAddress(
                user32,
                "SetThreadDpiAwarenessContext");

        if (SetThreadDpiAwarenessContext)
        {
            SetThreadDpiAwarenessContext(
                DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }

        FreeLibrary(user32);
    }
}

// ============================================================
// WINDOW PROC
// ============================================================

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
    case WM_DESTROY:
    {
        gRunning = false;
        PostQuitMessage(0);
        return 0;
    }

    case WM_SETCURSOR:
    {
        if (LOWORD(lParam) == HTCLIENT)
        {
            SetCursor(nullptr);
            return TRUE;
        }
    }

    case WM_KEYDOWN:
    {
		CEngine::GetInstance().On_KeyDown( (uint32_t)wParam );
        return 0;
    }

    case WM_KEYUP:
    {
		CEngine::GetInstance().On_KeyUp( (uint32_t)wParam );
        return 0;
    }

    case WM_MOUSEMOVE:
    {
        int iMouseX = GET_X_LPARAM(lParam)/iPixelSizeX;
        int iMouseY = GET_Y_LPARAM(lParam)/iPixelSizeY;
		CEngine::GetInstance().On_MouseMove( iMouseX, iMouseY );
        return 0;
    }

    case WM_LBUTTONDOWN:
	CEngine::GetInstance().On_MouseButtonDown( 0 );
    return 0;

    case WM_LBUTTONUP:
	CEngine::GetInstance().On_MouseButtonUp( 0 );
    return 0;

	case WM_MBUTTONDOWN:
	CEngine::GetInstance().On_MouseButtonDown( 2 );
	return 0;

	case WM_MBUTTONUP:
	CEngine::GetInstance().On_MouseButtonUp( 2 );
	return 0;

    case WM_RBUTTONDOWN:
	CEngine::GetInstance().On_MouseButtonDown( 1 );
    return 0;

    case WM_RBUTTONUP:
	CEngine::GetInstance().On_MouseButtonUp( 1 );
    return 0;
    }

    return DefWindowProc(
        hwnd,
        msg,
        wParam,
        lParam);
}

// ============================================================
// FRAMEBUFFER
// ============================================================

void CreateFramebuffer(HDC windowDC)
{
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WIDTH;
    bmi.bmiHeader.biHeight = -HEIGHT; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    gDCFrameBuffer = CreateCompatibleDC(windowDC);

    gBitmapFrameBuffer = CreateDIBSection(
        windowDC,
        &bmi,
        DIB_RGB_COLORS,
        (void**)&gFramebuffer,
        nullptr,
        0);

    SelectObject(gDCFrameBuffer, gBitmapFrameBuffer);
}

void CreatePresentBuffer(HDC windowDC)
{
    gDCPresent = CreateCompatibleDC(windowDC);

    gBitmapPresent =
        CreateCompatibleBitmap(
            windowDC,
            WIDTH * iPixelSizeX,
            HEIGHT * iPixelSizeY);

    SelectObject(
        gDCPresent,
        gBitmapPresent);
}

// ============================================================
// PRESENT
// ============================================================

void Present(HWND hwnd)
{
    {
        SetStretchBltMode( gDCPresent, COLORONCOLOR ); // nearest neighbour
        StretchBlt( gDCPresent, 0, 0, WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY, gDCFrameBuffer, 0, 0, WIDTH, HEIGHT, SRCCOPY );
    }

    {
        wchar_t msg[256];
        swprintf( msg, 256, L" %dx%d - %.2f ms (%.2f fps)", WIDTH, HEIGHT, fRenderTime * 1000, 1.0 / fRenderTime );
        SetBkMode(gDCPresent, TRANSPARENT);
        SetTextColor(gDCPresent, RGB(255, 255, 255));
        TextOut(gDCPresent, 0, 0, msg, (int)wcslen(msg));
    }

    {
        HDC windowDC = GetDC(hwnd);
        BitBlt( windowDC, 0, 0, WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY, gDCPresent, 0, 0, SRCCOPY );
        ReleaseDC(hwnd, windowDC);
    }
}

// ============================================================
// MAIN
// ============================================================

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int)
{
#ifdef _DEBUG
    _CrtSetDbgFlag(
        _CRTDBG_ALLOC_MEM_DF |
        _CRTDBG_LEAK_CHECK_DF);
#endif

    EnableDPIAwareness();

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MinimalWindow";

    RegisterClass(&wc);

    DWORD style =
        WS_CAPTION |
        WS_SYSMENU |
        WS_MINIMIZEBOX;

    RECT rect =
    {
        0,
        0,
        WIDTH * iPixelSizeX,
        HEIGHT * iPixelSizeY
    };

    AdjustWindowRect(
        &rect,
        style,
        FALSE);

    HWND hwnd =
        CreateWindowEx(
            0,
            wc.lpszClassName,
            L"SWRenderer",
            style,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,
            nullptr,
            nullptr,
            hInstance,
            nullptr);

    ShowWindow(hwnd, SW_SHOW);

    HDC dc = GetDC(hwnd);
    CreateFramebuffer(dc);
    CreatePresentBuffer(dc);
    ReleaseDC(hwnd, dc);

    MSG msg = {};

    SFrameBuffer sFrameBuffer(
        gFramebuffer,
        WIDTH,
		HEIGHT );
    CEngine::CreateInstance();
	CEngine::GetInstance().Create( sFrameBuffer );

    while (gRunning)
    {
        CPerf cPerfFrame;
        
		cPerfFrame.BeginPerf();

        while (PeekMessage(
            &msg,
            nullptr,
            0,
            0,
            PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        CPerf cPerfRender;
        cPerfRender.BeginPerf();
		CEngine::GetInstance().Update();
        fRenderTime = cPerfRender.EndPerf();

        Present(hwnd);

		double frameTime = cPerfFrame.EndPerf();

        {
            wchar_t title[256];
            swprintf( title, 256, L" SWRenderer - %dx%d - %.2f ms (%.2f fps)", WIDTH * iPixelSizeX, HEIGHT * iPixelSizeY, frameTime * 1000, 1.0 / frameTime );
            SetWindowText( hwnd, title );
        }
    }

    CEngine::Destroy();

    DeleteObject(gBitmapFrameBuffer);
    DeleteDC(gDCFrameBuffer);

    return 0;
}