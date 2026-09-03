#include "ImGuiDX11.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#pragma comment(lib, "d3d11.lib")

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool ImGuiDX11::Init(HWND hwnd)
{
    m_hwnd = hwnd;

    // Configure DXGI swap chain structure
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = 0; // Removed DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH to prevent exclusive fullscreen
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    // Create Direct3D 11 device and swap chain
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
        &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pd3dContext
    );

    if (FAILED(hr))
        return false;

    // Prevent DXGI from hijacking Alt+Enter
    IDXGIFactory* pFactory = nullptr;
    if (SUCCEEDED(m_pSwapChain->GetParent(IID_PPV_ARGS(&pFactory))))
    {
        pFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
        pFactory->Release();
    }

    CreateRenderTarget();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Apply default dark style
    ImGui::StyleColorsDark();

    // Initialize platform and renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(m_pd3dDevice, m_pd3dContext);

    return true;
}

bool ImGuiDX11::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    if (FAILED(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
        return false;

    m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
    pBackBuffer->Release();
    return true;
}

void ImGuiDX11::CleanupRenderTarget()
{
    if (m_mainRenderTargetView)
    {
        m_mainRenderTargetView->Release();
        m_mainRenderTargetView = nullptr;
    }
}

void ImGuiDX11::OnResize(UINT width, UINT height)
{
    if (!m_pd3dDevice || width == 0 || height == 0)
        return;

    CleanupRenderTarget();
    m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTarget();
}

LRESULT ImGuiDX11::HandleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Always pass Win32 messages to ImGui if context exists
    if (ImGui::GetCurrentContext() != nullptr)
    {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
            return true;
    }
    return 0;
}

void ImGuiDX11::EnsureTextureSize(int width, int height)
{
    // Recreate the texture if dimensions changed or if it doesn't exist yet
    if (!m_pTexture || m_texWidth != width || m_texHeight != height)
    {
        if (m_pSRV) { m_pSRV->Release(); m_pSRV = nullptr; }
        if (m_pTexture) { m_pTexture->Release(); m_pTexture = nullptr; }

        m_texWidth = width;
        m_texHeight = height;

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Match GDI 32-bit BGRA pixel layout
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        if (FAILED(m_pd3dDevice->CreateTexture2D(&desc, nullptr, &m_pTexture)))
            return;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        m_pd3dDevice->CreateShaderResourceView(m_pTexture, &srvDesc, &m_pSRV);
    }
}

ID3D11ShaderResourceView* ImGuiDX11::UpdateFromPresentDC(HDC hdc, int width, int height)
{
    if (!hdc || width <= 0 || height <= 0)
        return nullptr;

    EnsureTextureSize(width, height);
    if (!m_pTexture || !m_pSRV)
        return nullptr;

    // Retrieve the active bitmap selected into the GDI Device Context
    HBITMAP hBitmap = (HBITMAP)GetCurrentObject(hdc, OBJ_BITMAP);
    if (!hBitmap)
        return nullptr;

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down coordinate orientation
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    m_pixelBuffer.resize(width * height);

    // Extract raw ARGB pixel bits from the GDI bitmap
    if (GetDIBits(hdc, hBitmap, 0, height, m_pixelBuffer.data(), &bmi, DIB_RGB_COLORS))
    {
        // Copy pixel data into DirectX 11 dynamic texture memory
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(m_pd3dContext->Map(m_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            const uint8_t* pSrc = (const uint8_t*)m_pixelBuffer.data();
            uint8_t* pDst = (uint8_t*)mapped.pData;
            int rowBytes = width * 4;

            for (int y = 0; y < height; ++y)
            {
                memcpy(pDst + y * mapped.RowPitch, pSrc + y * rowBytes, rowBytes);
            }
            m_pd3dContext->Unmap(m_pTexture, 0);
        }
    }

    return m_pSRV;
}

void ImGuiDX11::BeginFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiDX11::EndFrame()
{
    ImGui::Render();

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_pd3dContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
    m_pd3dContext->ClearRenderTargetView(m_mainRenderTargetView, clearColor);

    // Render ImGui draw lists
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers (VSync enabled)
    m_pSwapChain->Present(1, 0);
}

void ImGuiDX11::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (m_pSRV) { m_pSRV->Release(); m_pSRV = nullptr; }
    if (m_pTexture) { m_pTexture->Release(); m_pTexture = nullptr; }
    CleanupRenderTarget();
    if (m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = nullptr; }
    if (m_pd3dContext) { m_pd3dContext->Release(); m_pd3dContext = nullptr; }
    if (m_pd3dDevice) { m_pd3dDevice->Release(); m_pd3dDevice = nullptr; }
}