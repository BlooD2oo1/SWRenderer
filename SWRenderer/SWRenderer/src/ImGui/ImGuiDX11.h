#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <stdint.h>
#include <vector>

class ImGuiDX11
{
public:
    ImGuiDX11() = default;
    ~ImGuiDX11() { Shutdown(); }

    // Initialize DirectX 11 device, swap chain, and ImGui context
    bool Init(HWND hwnd);

    // Release all DirectX 11 and ImGui resources
    void Shutdown();

    // Resize DXGI SwapChain buffers when the window size changes
    void OnResize(UINT width, UINT height);

    // Forward Win32 window messages to ImGui (only when editor mode is active)
    LRESULT HandleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Begin a new ImGui frame
    void BeginFrame();

    // Render ImGui draw data and swap back/front buffers
    void EndFrame();

    // Copy the pixel contents from the GDI Present DC (hDCPresent) into a DirectX 11 texture
    ID3D11ShaderResourceView* UpdateFromPresentDC(HDC hdc, int width, int height);

private:
    bool CreateRenderTarget();
    void CleanupRenderTarget();
    void EnsureTextureSize(int width, int height);

    HWND m_hwnd = nullptr;
    ID3D11Device* m_pd3dDevice = nullptr;
    ID3D11DeviceContext* m_pd3dContext = nullptr;
    IDXGISwapChain* m_pSwapChain = nullptr;
    ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;

    // Texture and Shader Resource View for displaying the present buffer in ImGui
    ID3D11Texture2D* m_pTexture = nullptr;
    ID3D11ShaderResourceView* m_pSRV = nullptr;
    int m_texWidth = 0;
    int m_texHeight = 0;

    // Temporary CPU pixel buffer for GetDIBits extraction
    std::vector<uint32_t> m_pixelBuffer;
};