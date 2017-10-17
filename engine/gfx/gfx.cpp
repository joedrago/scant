#include "gfx/gfx.h"

#include "os/os.h"
#include "gfx/shaders.h"
#include "gfx/font.h"

#include <windows.h>
#include <d3d11_1.h>
#include <directxmath.h>

#include "png.h"

#include <algorithm>
#include <vector>

// So wrong, but oh so right. The naughtiest.
#pragma comment (lib, "D3D11.lib")

// #define DEBUG_FONT_RENDERING

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
static ID3D11PixelShader * d3dColorPixelShader_ = nullptr;
static ID3D11PixelShader * d3dTextureColorPixelShader_ = nullptr;
static ID3D11InputLayout * d3dVertexLayout_ = nullptr;
static ID3D11Buffer * d3dVertexBuffer_ = nullptr;

struct TextureRecord
{
    int width;
    int height;
    ID3D11Texture2D * d3dTexture;
    ID3D11ShaderResourceView * d3dTextureView;
};
static std::vector<TextureRecord> textures_;
static std::vector<gfx::Font *> fonts_;

struct vec2
{
    float x;
    float y;
};

struct PosTexColorVertex
{
    float x;
    float y;
    float z;
    float u;
    float v;
    float r;
    float g;
    float b;
    float a;
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

        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return false;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1 * dxgiFactory = nullptr;
    {
        IDXGIDevice * dxgiDevice = nullptr;
        hr = d3dDevice_->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&dxgiDevice));
        if (SUCCEEDED(hr)) {
            IDXGIAdapter * adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr)) {
                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(&dxgiFactory));
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return false;

    // Create swap chain
    IDXGIFactory2 * dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void **>(&dxgiFactory2));
    if (dxgiFactory2) {
        // DirectX 11.1 or later
        hr = d3dDevice_->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void **>(&d3dDevice1_));
        if (SUCCEEDED(hr)) {
            (void)d3dContext_->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void **>(&d3dContext1_));
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
            hr = d3dSwapChain1_->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void **>(&d3dSwapChain_));
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
        return false;

    // Create a render target view
    ID3D11Texture2D * pBackBuffer = nullptr;
    hr = d3dSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>( &pBackBuffer ));
    if (FAILED(hr))
        return false;

    hr = d3dDevice_->CreateRenderTargetView(pBackBuffer, nullptr, &d3dRenderTargetView_);
    pBackBuffer->Release();
    if (FAILED(hr))
        return false;

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

    // Setup blend state
    D3D11_BLEND_DESC sourceOver;
    ZeroMemory(&sourceOver, sizeof(sourceOver));
    sourceOver.RenderTarget[0].BlendEnable = TRUE;
    sourceOver.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    sourceOver.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    sourceOver.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    sourceOver.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    sourceOver.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    sourceOver.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    sourceOver.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    ID3D11BlendState * blendState = nullptr;
    d3dDevice_->CreateBlendState(&sourceOver, &blendState);
    assert(blendState != nullptr);
    d3dContext_->OMSetBlendState(blendState, nullptr, 0xffffffff);
    blendState->Release();

    // Setup sampler state
    ID3D11SamplerState * samplerState = nullptr;
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    d3dDevice_->CreateSamplerState(&sampDesc, &samplerState);
    assert(samplerState != nullptr);
    d3dContext_->PSSetSamplers(0, 1, &samplerState);
    samplerState->Release();

    // Create the vertex shader
    hr = d3dDevice_->CreateVertexShader(Shader_VSPos, sizeof(Shader_VSPos), nullptr, &d3dVertexShader_);
    if (FAILED(hr)) {
        return false;
    }

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = d3dDevice_->CreateInputLayout(layout, numElements, Shader_VSPos, sizeof(Shader_VSPos), &d3dVertexLayout_);
    if (FAILED(hr))
        return false;

    // Set the input layout
    d3dContext_->IASetInputLayout(d3dVertexLayout_);

    // Create the pixel shaders
    hr = d3dDevice_->CreatePixelShader(Shader_PSColor, sizeof(Shader_PSColor), nullptr, &d3dColorPixelShader_);
    if (FAILED(hr))
        return false;
    hr = d3dDevice_->CreatePixelShader(Shader_PSTextureColor, sizeof(Shader_PSTextureColor), nullptr, &d3dTextureColorPixelShader_);
    if (FAILED(hr))
        return false;

    // Create vertex buffer
    PosTexColorVertex vertices[] = {
        { 0.0f, 0.0f, 0.5f, 0, 0, 1, 0, 0, 1 }, // bottom left
        { 0.0f, 0.5f, 0.5f, 0, 0, 1, 0, 0, 1 }, // top left
        { 0.5f, 0.0f, 0.5f, 0, 0, 1, 0, 0, 1 }, // bottom right
        { 0.5f, 0.5f, 0.5f, 0, 0, 1, 0, 0, 1 } // top right
    };

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    hr = d3dDevice_->CreateBuffer(&bd, &InitData, &d3dVertexBuffer_);
    if (FAILED(hr))
        return false;

    // Set vertex buffer
    UINT stride = sizeof( PosTexColorVertex );
    UINT offset = 0;
    d3dContext_->IASetVertexBuffers(0, 1, &d3dVertexBuffer_, &stride, &offset);

    // Set primitive topology
    d3dContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    return true;
}

void shutdown()
{
    // TODO: release textures_ and textures_.clear()

    if (d3dContext_)
        d3dContext_->ClearState();
    if (d3dVertexBuffer_)
        d3dVertexBuffer_->Release();
    if (d3dVertexLayout_)
        d3dVertexLayout_->Release();
    if (d3dVertexShader_)
        d3dVertexShader_->Release();
    if (d3dColorPixelShader_)
        d3dColorPixelShader_->Release();
    if (d3dTextureColorPixelShader_)
        d3dTextureColorPixelShader_->Release();
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
}

void end()
{
    d3dSwapChain_->Present(1, 0);
}

int createTexture(int width, int height)
{
    HRESULT hr;

    TextureRecord tr;
    tr.width = width;
    tr.height = height;

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    hr = d3dDevice_->CreateTexture2D(&desc, NULL, &tr.d3dTexture);
    assert((hr == S_OK) && (tr.d3dTexture != nullptr));

    D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
    srDesc.Format = desc.Format;
    srDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    srDesc.Texture2D.MostDetailedMip = 0;
    srDesc.Texture2D.MipLevels = 1;
    hr = d3dDevice_->CreateShaderResourceView(tr.d3dTexture, &srDesc, &tr.d3dTextureView);
    assert((hr == S_OK) && (tr.d3dTextureView != nullptr));

    int id = (int)textures_.size();
    textures_.push_back(tr);
    return id;
}

unsigned char * lockTexture(int id, TextureMetrics * outMetrics)
{
    assert((id >= 0) && (id < textures_.size()));

    TextureRecord & tr = textures_[id];
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    memset(&mappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
    d3dContext_->Map(tr.d3dTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    outMetrics->width = tr.width;
    outMetrics->height = tr.height;
    outMetrics->pitch = mappedResource.RowPitch;
    return (unsigned char *)mappedResource.pData;
}

void unlockTexture(int id)
{
    assert((id >= 0) && (id < textures_.size()));

    TextureRecord & tr = textures_[id];
    d3dContext_->Unmap(tr.d3dTexture, 0);
}

struct gfx_read_png_info
{
    unsigned char * curr;
    png_size_t remaining;
};
static void gfx_read_png(png_structp read, png_bytep data, png_size_t length)
{
    gfx_read_png_info * info = (gfx_read_png_info *)png_get_io_ptr(read);
    assert(info->remaining >= length);
    memcpy(data, info->curr, length);
    info->curr += length;
    info->remaining -= length;
}

int loadPNG(const char * path, TextureMetrics * outMetrics)
{
    std::vector<unsigned char> pngFileData;
    if (!os::readFile(path, pngFileData)) {
        return -1;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png == NULL) {
        return -1;
    }

    png_infop info = png_create_info_struct(png);
    if (info == NULL) {
        png_destroy_read_struct(&png, NULL, NULL);
        return -1;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        return -1;
    }

    gfx_read_png_info readInfo;
    readInfo.curr = &pngFileData[0];
    readInfo.remaining = pngFileData.size();
    png_set_read_fn(png, &readInfo, gfx_read_png);
    png_read_info(png, info);

    int width      = png_get_image_width(png, info);
    int height     = png_get_image_height(png, info);
    int color_type = png_get_color_type(png, info);
    int bit_depth  = png_get_bit_depth(png, info);

    // Read any color_type into 8bit depth, RGBA format.
    // See http://www.libpng.org/pub/png/libpng-manual.txt

    if (bit_depth == 16)
        png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if (( color_type == PNG_COLOR_TYPE_GRAY) && ( bit_depth < 8))
        png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    // These color_type don't have an alpha channel then fill it with 0xff.
    if (( color_type == PNG_COLOR_TYPE_RGB) ||
        ( color_type == PNG_COLOR_TYPE_GRAY) ||
        ( color_type == PNG_COLOR_TYPE_PALETTE))
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (( color_type == PNG_COLOR_TYPE_GRAY) ||
        ( color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    png_bytep * row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    int rowBytes = (int)png_get_rowbytes(png, info);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte *)malloc(rowBytes);
    }

    png_read_image(png, row_pointers);

    int id = gfx::createTexture(width, height);
    unsigned char * pixels = gfx::lockTexture(id, outMetrics);
    assert(rowBytes <= outMetrics->pitch);
    for (int y = 0; y < height; y++) {
        memcpy(pixels + (y * outMetrics->pitch), row_pointers[y], rowBytes);
    }
    gfx::unlockTexture(id);

    png_destroy_read_struct(&png, &info, NULL);
    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);
    return id;
}

DrawSource loadPNG(const char * path)
{
    TextureMetrics metrics;
    DrawSource src;
    src.textureId = loadPNG(path, &metrics);
    src.x = 0;
    src.y = 0;
    src.w = metrics.width;
    src.h = metrics.height;
    return src;
}

int loadFont(const char * name)
{
    gfx::Font * font = new gfx::Font;
    if (!font->load(name)) {
        delete font;
        return -1;
    }
    int id = (int)fonts_.size();
    fonts_.push_back(font);
    return id;
}

void draw(float pixelX, float pixelY, float pixelW, float pixelH, DrawSource * source, Color * color, float anchorX, float anchorY, float r)
{
    float windowW = os::winWf();
    float windowH = os::winHf();

    float anchorPixelX = -1 * anchorX * pixelW;
    float anchorPixelY = -1 * anchorY * pixelH;

    float leftPixelPos = anchorPixelX + pixelX;
    float topPixelPos = anchorPixelY + pixelY;
    float rightPixelPos = anchorPixelX + pixelX + pixelW;
    float bottomPixelPos = anchorPixelY + pixelY + pixelH;

    vec2 pos[4];
    pos[0].x = leftPixelPos;
    pos[0].y = bottomPixelPos;
    pos[1].x = leftPixelPos;
    pos[1].y = topPixelPos;
    pos[2].x = rightPixelPos;
    pos[2].y = bottomPixelPos;
    pos[3].x = rightPixelPos;
    pos[3].y = topPixelPos;

    if (r != 0.0f) {
        float anchorCoordX = leftPixelPos + (anchorX * (rightPixelPos - leftPixelPos));
        float anchorCoordY = topPixelPos + (anchorY * (bottomPixelPos - topPixelPos));

        float s = sin(r);
        float c = cos(r);

        for (int i = 0; i < 4; ++i) {
            vec2 & v = pos[i];
            v.x -= anchorCoordX;
            v.y -= anchorCoordY;

            float xnew = v.x * c - v.y * s;
            float ynew = v.x * s + v.y * c;
            v.x = xnew + anchorCoordX;
            v.y = ynew + anchorCoordY;
        }
    }

    float red, green, blue, alpha;
    if (color) {
        red   = (float)color->r / 255.0f;
        green = (float)color->g / 255.0f;
        blue  = (float)color->b / 255.0f;
        alpha = (float)color->a / 255.0f;
    } else {
        red = 1.0f;
        green = 1.0f;
        blue = 1.0f;
        alpha = 1.0f;
    }

    d3dContext_->VSSetShader(d3dVertexShader_, nullptr, 0);

    float leftUV, topUV, rightUV, bottomUV;
    if (source) {
        d3dContext_->PSSetShader(d3dTextureColorPixelShader_, nullptr, 0);

        assert((source->textureId >= 0) && (source->textureId < textures_.size()));
        TextureRecord & tr = textures_[source->textureId];
        d3dContext_->PSSetShaderResources(0, 1, &tr.d3dTextureView);

        int sourceW = source->w;
        int sourceH = source->h;
        if (sourceW == 0) {
            sourceW = tr.width;
        }
        if (sourceH == 0) {
            sourceH = tr.height;
        }

        leftUV   =  source->x / (float)tr.width;
        topUV    =  source->y / (float)tr.height;
        rightUV  = (source->x + (float)sourceW) / (float)tr.width;
        bottomUV = (source->y + (float)sourceH) / (float)tr.height;
    } else {
        d3dContext_->PSSetShader(d3dColorPixelShader_, nullptr, 0);

        ID3D11ShaderResourceView * unbindView = nullptr;
        d3dContext_->PSSetShaderResources(0, 1, &unbindView);

        leftUV   = 0.0f;
        topUV    = 0.0f;
        rightUV  = 0.0f;
        bottomUV = 0.0f;
    }

// *INDENT-OFF*
    PosTexColorVertex vertices[] = {
        { (2.0f * (pos[0].x / windowW)) - 1.0f, (2.0f * (1.0f - (pos[0].y / windowH))) - 1.0f, 0.5f,   leftUV,  bottomUV,   red, green, blue, alpha }, // bottom left
        { (2.0f * (pos[1].x / windowW)) - 1.0f, (2.0f * (1.0f - (pos[1].y / windowH))) - 1.0f, 0.5f,   leftUV,  topUV,      red, green, blue, alpha }, // top left
        { (2.0f * (pos[2].x / windowW)) - 1.0f, (2.0f * (1.0f - (pos[2].y / windowH))) - 1.0f, 0.5f,   rightUV, bottomUV,   red, green, blue, alpha }, // bottom right
        { (2.0f * (pos[3].x / windowW)) - 1.0f, (2.0f * (1.0f - (pos[3].y / windowH))) - 1.0f, 0.5f,   rightUV, topUV,      red, green, blue, alpha }  // top right
    };
// *INDENT-ON*

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    memset(&mappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
    d3dContext_->Map(d3dVertexBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, vertices, sizeof(vertices));
    d3dContext_->Unmap(d3dVertexBuffer_, 0);

    d3dContext_->Draw(4, 0);
}

void drawText(float pixelX, float pixelY, const char * text, int fontId, float fontHeight, Color * color, float anchorX, float anchorY, float r)
{
    if ((fontId < 0) || (fontId >= fonts_.size())) {
        return;
    }
    Font * font = fonts_[fontId];
    assert(font);

    // TODO: rotation

    float scale = fontHeight / font->maxHeight();
    float left = os::windowHeight() + 1000.0f;
    float top = os::windowWidth() + 1000.0f;
    float right = -1000.0f;
    float bottom = -1000.0f;

    {
        // Get some sensible baselines for ascent / descent
        char interestingChars[] = { 'M', 'd', '\'', 'g', 'p' }; // TODO: figure out tall and short letters by inspecting font.txt file
        for (int index = 0; index < sizeof(interestingChars); ++index) {
            int id = (int)interestingChars[index];
            Font::Glyph * glyph = font->findGlyph(id);
            if (!glyph)
                continue;

            float glyphY = glyph->yoffset * scale;
            float glyphH = glyph->src.h * scale;

            top = min(top, glyphY);
            bottom = max(bottom, glyphY + glyphH);
        }
    }

    float x = 0;
    float y = 0;
    for (const char * c = text; *c; ++c) {
        int id = *c;
        if (id == '`')
            continue;

        Font::Glyph * glyph = font->findGlyph(id);
        if (!glyph)
            continue;

        float glyphX = x + (glyph->xoffset * scale);
        float glyphY = y + (glyph->yoffset * scale);
        float glyphW = glyph->src.w * scale;
        float glyphH = glyph->src.h * scale;

        left = min(left, glyphX);
        top = min(top, glyphY);
        right = max(right, glyphX + glyphW);
        bottom = max(bottom, glyphY + glyphH);

        x += glyph->xadvance * scale;
    }

    float totalWidth = right - left;
    float totalHeight = bottom - top;
    float anchorOffsetX = -1 * anchorX * totalWidth;
    float anchorOffsetY = -1 * anchorY * totalHeight;

#if defined(DEBUG_FONT_RENDERING)
    // debug
    gfx::Color cyan = { 0, 255, 255, 64 };
    draw(anchorOffsetX + pixelX, anchorOffsetY + pixelY, totalWidth, totalHeight, nullptr, &cyan);
#endif

    x = pixelX;
    y = pixelY;
    for (const char * c = text; *c; ++c) {
        int id = *c;
        if (id == '`')
            continue;

        Font::Glyph * glyph = font->findGlyph(id);
        if (!glyph)
            continue;

        if ((glyph->src.w > 0) && (glyph->src.h > 0)) {
            float glyphX = anchorOffsetX + x + (glyph->xoffset * scale) - left;
            float glyphY = anchorOffsetY + y + (glyph->yoffset * scale) - top;
            float glyphW = glyph->src.w * scale;
            float glyphH = glyph->src.h * scale;
            draw(glyphX, glyphY, glyphW, glyphH, &glyph->src, color);

#if defined(DEBUG_FONT_RENDERING)
            // debug
            gfx::Color orange = { 255, 128, 0, 64 };
            draw(glyphX, glyphY, glyphW, glyphH, nullptr, &orange);
#endif
        }
        x += glyph->xadvance * scale;
    }
}

}
