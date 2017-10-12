#include "gfx/gfx.h"

#include "os/os.h"

#include "gfx/shaders.h"

#include <windows.h>
#include <d3d11_1.h>
#include <directxmath.h>

// So wrong, but oh so right. The naughtiest.
#pragma comment (lib, "D3D11.lib")

static D3D_DRIVER_TYPE d3dDriverType_ = D3D_DRIVER_TYPE_NULL;
static D3D_FEATURE_LEVEL d3dFeatureLevel_ = D3D_FEATURE_LEVEL_11_0;
static ID3D11Device * d3dDevice_ = nullptr;
static ID3D11Device1 * d3dDevice1_ = nullptr;
static ID3D11DeviceContext * d3dContext_ = nullptr;
static ID3D11DeviceContext1 * d3dContext1_ = nullptr;
static IDXGISwapChain * d3dSwapChain_ = nullptr;
static IDXGISwapChain1 * d3dSwapChain1_ = nullptr;
static ID3D11RenderTargetView * d3dRenderTargetView_ = nullptr;
static ID3D11VertexShader * d3dVertexShader_ = nullptr;
static ID3D11PixelShader * d3dPixelShader_ = nullptr;
static ID3D11InputLayout * d3dVertexLayout_ = nullptr;
static ID3D11Buffer * d3dVertexBuffer_ = nullptr;

struct SimpleVertex
{
    DirectX::XMFLOAT3 Pos;
};

namespace gfx
{

bool startup()
{
    os::printf("gfx::startup\n");

    HWND hwnd = os::windowHandle();
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(hwnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
        d3dDriverType_ = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(nullptr, d3dDriverType_, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &d3dDevice_, &d3dFeatureLevel_, &d3dContext_);

        if (hr == E_INVALIDARG) {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice(nullptr, d3dDriverType_, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                D3D11_SDK_VERSION, &d3dDevice_, &d3dFeatureLevel_, &d3dContext_);
        }

        if (SUCCEEDED(hr) )
            break;
    }
    if (FAILED(hr) )
        return false;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1 * dxgiFactory = nullptr;
    {
        IDXGIDevice * dxgiDevice = nullptr;
        hr = d3dDevice_->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&dxgiDevice) );
        if (SUCCEEDED(hr)) {
            IDXGIAdapter * adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr)) {
                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(&dxgiFactory) );
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return hr;

    // Create swap chain
    IDXGIFactory2 * dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void **>(&dxgiFactory2) );
    if (dxgiFactory2) {
        // DirectX 11.1 or later
        hr = d3dDevice_->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void **>(&d3dDevice1_) );
        if (SUCCEEDED(hr)) {
            (void)d3dContext_->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void **>(&d3dContext1_) );
        }

        DXGI_SWAP_CHAIN_DESC1 sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd(d3dDevice_, hwnd, &sd, nullptr, nullptr, &d3dSwapChain1_);
        if (SUCCEEDED(hr)) {
            hr = d3dSwapChain1_->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void **>(&d3dSwapChain_) );
        }

        dxgiFactory2->Release();
    } else {
        // DirectX 11.0 systems
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain(d3dDevice_, &sd, &d3dSwapChain_);
    }

    // Block alt+enter for fullscreen
    dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    dxgiFactory->Release();

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D * pBackBuffer = nullptr;
    hr = d3dSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>( &pBackBuffer ) );
    if (FAILED(hr) )
        return hr;

    hr = d3dDevice_->CreateRenderTargetView(pBackBuffer, nullptr, &d3dRenderTargetView_);
    pBackBuffer->Release();
    if (FAILED(hr) )
        return hr;

    d3dContext_->OMSetRenderTargets(1, &d3dRenderTargetView_, nullptr);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    d3dContext_->RSSetViewports(1, &vp);

    // Create the vertex shader
    hr = d3dDevice_->CreateVertexShader(Shader_VSPos, sizeof(Shader_VSPos), nullptr, &d3dVertexShader_);
    if (FAILED(hr) ) {
        return false;
    }

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = d3dDevice_->CreateInputLayout(layout, numElements, Shader_VSPos, sizeof(Shader_VSPos), &d3dVertexLayout_);
    if (FAILED(hr) )
        return false;

    // Set the input layout
    d3dContext_->IASetInputLayout(d3dVertexLayout_);

    // Create the pixel shader
    hr = d3dDevice_->CreatePixelShader(Shader_PSYellow, sizeof(Shader_PSYellow), nullptr, &d3dPixelShader_);
    if (FAILED(hr) )
        return false;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        DirectX::XMFLOAT3(0.0f, 0.5f, 0.5f),
        DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f),
        DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f),
    };
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 3;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData) );
    InitData.pSysMem = vertices;
    hr = d3dDevice_->CreateBuffer(&bd, &InitData, &d3dVertexBuffer_);
    if (FAILED(hr) )
        return hr;

    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    d3dContext_->IASetVertexBuffers(0, 1, &d3dVertexBuffer_, &stride, &offset);

    // Set primitive topology
    d3dContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    return true;
}

void shutdown()
{
    if (d3dContext_)
        d3dContext_->ClearState();
    if (d3dVertexBuffer_)
        d3dVertexBuffer_->Release();
    if (d3dVertexLayout_)
        d3dVertexLayout_->Release();
    if (d3dVertexShader_)
        d3dVertexShader_->Release();
    if (d3dPixelShader_)
        d3dPixelShader_->Release();
    if (d3dRenderTargetView_)
        d3dRenderTargetView_->Release();
    if (d3dSwapChain1_)
        d3dSwapChain1_->Release();
    if (d3dSwapChain_)
        d3dSwapChain_->Release();
    if (d3dContext1_)
        d3dContext1_->Release();
    if (d3dContext_)
        d3dContext_->Release();
    if (d3dDevice1_)
        d3dDevice1_->Release();
    if (d3dDevice_)
        d3dDevice_->Release();
}

void begin()
{
    float clearColor[4] = { 0, 0, 0, 0 };
    d3dContext_->ClearRenderTargetView(d3dRenderTargetView_, clearColor);

    d3dContext_->VSSetShader(d3dVertexShader_, nullptr, 0);
    d3dContext_->PSSetShader(d3dPixelShader_, nullptr, 0);
    d3dContext_->Draw(3, 0);
}

void end()
{
    d3dSwapChain_->Present(0, 0);
}

}
