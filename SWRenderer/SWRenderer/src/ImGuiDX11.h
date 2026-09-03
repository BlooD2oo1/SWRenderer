#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <stdint.h>

class ImGuiDX11
{
public:
    ImGuiDX11() = default;
    ~ImGuiDX11() { Shutdown(); }

    bool Init(HWND hwnd);
    void Shutdown();

    void OnResize(UINT width, UINT height);
    LRESULT HandleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void BeginFrame();
    void EndFrame();

    // A szoftveres framebuffer feltöltése DX11 textúrába
    ID3D11ShaderResourceView* UpdateFramebufferTexture(const uint32_t* pFramebuffer, int width, int height);

    bool bEditorMode = false;

private:
    bool CreateRenderTarget();
    void CleanupRenderTarget();

    HWND m_hwnd = nullptr;
    ID3D11Device* m_pd3dDevice = nullptr;
    ID3D11DeviceContext* m_pd3dContext = nullptr;
    IDXGISwapChain* m_pSwapChain = nullptr;
    ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;

    ID3D11Texture2D* m_pFBTexture = nullptr;
    ID3D11ShaderResourceView* m_pFBSRV = nullptr;
    int m_fbWidth = 0;
    int m_fbHeight = 0;
};