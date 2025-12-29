#include "../include/rendering/DirectXRenderer.h"
#include "../include/Types.h"
#include <stdexcept>
#include <cstring> // for strlen

DirectXRenderer::DirectXRenderer()
    : hwnd_(nullptr), device_(nullptr), context_(nullptr), swapChain_(nullptr),
      renderTargetView_(nullptr), depthStencilView_(nullptr), depthStencilBuffer_(nullptr),
      vertexShader_(nullptr), pixelShader_(nullptr), inputLayout_(nullptr),
      vertexBuffer_(nullptr), indexBuffer_(nullptr), constantBuffer_(nullptr) {
}

DirectXRenderer::~DirectXRenderer() {
    releaseResources();
}

bool DirectXRenderer::initialize(HWND hwnd) {
    hwnd_ = hwnd;

    if (!createDeviceAndSwapChain()) return false;
    if (!createRenderTargetView()) return false;
    if (!createDepthStencilView()) return false;
    if (!createShaders()) return false;
    if (!createInputLayout()) return false;
    if (!createVertexBuffer()) return false;
    if (!createIndexBuffer()) return false;
    if (!createConstantBuffer()) return false;

    return true;
}

void DirectXRenderer::resize(int width, int height) {
    if (renderTargetView_) {
        renderTargetView_->Release();
        renderTargetView_ = nullptr;
    }

    if (depthStencilView_) {
        depthStencilView_->Release();
        depthStencilView_ = nullptr;
    }

    if (depthStencilBuffer_) {
        depthStencilBuffer_->Release();
        depthStencilBuffer_ = nullptr;
    }

    HRESULT result = swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(result)) {
        return;
    }

    createRenderTargetView();
    createDepthStencilView();
}

void DirectXRenderer::clear(float r, float g, float b, float a) {
    float color[4] = { r, g, b, a };
    context_->ClearRenderTargetView(renderTargetView_, color);
    context_->ClearDepthStencilView(depthStencilView_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DirectXRenderer::renderMesh(
    const VertexPosColor* vertices,
    unsigned int vertexCount,
    const unsigned long* indices,
    unsigned int indexCount,
    const DirectX::XMMATRIX& world,
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& projection
) {
    // Update constant buffer
    ConstantBuffer cb;
    cb.world = DirectX::XMMatrixTranspose(world);
    cb.view = DirectX::XMMatrixTranspose(view);
    cb.projection = DirectX::XMMatrixTranspose(projection);

    context_->UpdateSubresource(constantBuffer_, 0, nullptr, &cb, 0, 0);
    context_->VSSetConstantBuffers(0, 1, &constantBuffer_);

    // Set vertex buffer
    UINT stride = sizeof(VertexPosColor);
    UINT offset = 0;
    context_->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);

    // Set index buffer
    context_->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R32_UINT, 0);

    // Set primitive topology
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set shaders
    context_->VSSetShader(vertexShader_, nullptr, 0);
    context_->PSSetShader(pixelShader_, nullptr, 0);
    context_->IASetInputLayout(inputLayout_);

    // Draw
    context_->DrawIndexed(indexCount, 0, 0);
}

void DirectXRenderer::present() {
    swapChain_->Present(0, 0);
}

bool DirectXRenderer::createDeviceAndSwapChain() {
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = 0; // Use window width
    swapChainDesc.BufferDesc.Height = 0; // Use window height
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd_;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr,                    // pAdapter
        D3D_DRIVER_TYPE_HARDWARE,   // DriverType
        nullptr,                    // Software
        createDeviceFlags,          // Flags
        &featureLevel,              // pFeatureLevels
        1,                          // FeatureLevels
        D3D11_SDK_VERSION,          // SDKVersion
        &swapChainDesc,             // pSwapChainDesc
        &swapChain_,                // ppSwapChain
        &device_,                   // ppDevice
        nullptr,                    // pFeatureLevel
        &context_                   // ppImmediateContext
    );

    return SUCCEEDED(result);
}

bool DirectXRenderer::createRenderTargetView() {
    ID3D11Texture2D* backBuffer = nullptr;
    HRESULT result = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(result)) return false;

    result = device_->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView_);
    backBuffer->Release();
    if (FAILED(result)) return false;

    return true;
}

bool DirectXRenderer::createDepthStencilView() {
    // Get window size
    RECT rect;
    GetClientRect(hwnd_, &rect);
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    HRESULT result = device_->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer_);
    if (FAILED(result)) return false;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format = depthStencilDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    result = device_->CreateDepthStencilView(depthStencilBuffer_, &depthStencilViewDesc, &depthStencilView_);
    if (FAILED(result)) return false;

    // Bind the render target view and depth stencil view to the output render pipeline
    context_->OMSetRenderTargets(1, &renderTargetView_, depthStencilView_);

    return true;
}

bool DirectXRenderer::createShaders() {
    // Simple vertex shader
    const char* vertexShaderCode =
        "cbuffer ConstantBuffer : register(b0) \
        { \
            matrix world : packoffset(c0); \
            matrix view : packoffset(c4); \
            matrix projection : packoffset(c8); \
        } \
        struct VS_INPUT \
        { \
            float3 pos : POSITION; \
            float4 color : COLOR0; \
        }; \
        struct VS_OUTPUT \
        { \
            float4 pos : SV_POSITION; \
            float4 color : COLOR0; \
        }; \
        VS_OUTPUT main(VS_INPUT input) \
        { \
            VS_OUTPUT output; \
            float4 pos = float4(input.pos, 1.0f); \
            pos = mul(pos, world); \
            pos = mul(pos, view); \
            pos = mul(pos, projection); \
            output.pos = pos; \
            output.color = input.color; \
            return output; \
        }";

    // Simple pixel shader
    const char* pixelShaderCode =
        "struct PS_INPUT \
        { \
            float4 pos : SV_POSITION; \
            float4 color : COLOR0; \
        }; \
        float4 main(PS_INPUT input) : SV_TARGET \
        { \
            return input.color; \
        }";

    ID3DBlob* vertexShaderBlob = nullptr;
    ID3DBlob* pixelShaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT result = D3DCompile(
        vertexShaderCode,
        strlen(vertexShaderCode),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_4_0",
        0,
        0,
        &vertexShaderBlob,
        &errorBlob
    );

    if (FAILED(result)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        return false;
    }

    result = device_->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        nullptr,
        &vertexShader_
    );

    if (FAILED(result)) {
        vertexShaderBlob->Release();
        return false;
    }

    result = D3DCompile(
        pixelShaderCode,
        strlen(pixelShaderCode),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "ps_4_0",
        0,
        0,
        &pixelShaderBlob,
        &errorBlob
    );

    if (FAILED(result)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        vertexShaderBlob->Release();
        return false;
    }

    result = device_->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr,
        &pixelShader_
    );

    vertexShaderBlob->Release();
    pixelShaderBlob->Release();

    if (FAILED(result)) {
        return false;
    }

    return true;
}

bool DirectXRenderer::createInputLayout() {
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    UINT numElements = ARRAYSIZE(layout);

    // We need to compile the shader first to get the bytecode for input layout
    ID3DBlob* vertexShaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    const char* vertexShaderCode =
        "cbuffer ConstantBuffer : register(b0) \
        { \
            matrix world : packoffset(c0); \
            matrix view : packoffset(c4); \
            matrix projection : packoffset(c8); \
        } \
        struct VS_INPUT \
        { \
            float3 pos : POSITION; \
            float4 color : COLOR0; \
        }; \
        struct VS_OUTPUT \
        { \
            float4 pos : SV_POSITION; \
            float4 color : COLOR0; \
        }; \
        VS_OUTPUT main(VS_INPUT input) \
        { \
            VS_OUTPUT output; \
            float4 pos = float4(input.pos, 1.0f); \
            pos = mul(pos, world); \
            pos = mul(pos, view); \
            pos = mul(pos, projection); \
            output.pos = pos; \
            output.color = input.color; \
            return output; \
        }";

    HRESULT result = D3DCompile(
        vertexShaderCode,
        strlen(vertexShaderCode),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_4_0",
        0,
        0,
        &vertexShaderBlob,
        &errorBlob
    );

    if (SUCCEEDED(result)) {
        result = device_->CreateInputLayout(
            layout,
            numElements,
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            &inputLayout_
        );
        vertexShaderBlob->Release();
    }

    if (errorBlob) {
        errorBlob->Release();
    }

    return SUCCEEDED(result);
}

bool DirectXRenderer::createVertexBuffer() {
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT; // Changed from DYNAMIC for simplicity
    bufferDesc.ByteWidth = sizeof(VertexPosColor) * 3; // Initial size for 3 vertices
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0; // Changed from D3D11_CPU_ACCESS_WRITE

    HRESULT result = device_->CreateBuffer(&bufferDesc, nullptr, &vertexBuffer_);
    return SUCCEEDED(result);
}

bool DirectXRenderer::createIndexBuffer() {
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT; // Changed from DYNAMIC for simplicity
    bufferDesc.ByteWidth = sizeof(unsigned long) * 3; // Initial size for 3 indices
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0; // Changed from D3D11_CPU_ACCESS_WRITE

    HRESULT result = device_->CreateBuffer(&bufferDesc, nullptr, &indexBuffer_);
    return SUCCEEDED(result);
}

bool DirectXRenderer::createConstantBuffer() {
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(ConstantBuffer);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    HRESULT result = device_->CreateBuffer(&bufferDesc, nullptr, &constantBuffer_);
    return SUCCEEDED(result);
}

void DirectXRenderer::releaseResources() {
    if (constantBuffer_) { constantBuffer_->Release(); constantBuffer_ = nullptr; }
    if (indexBuffer_) { indexBuffer_->Release(); indexBuffer_ = nullptr; }
    if (vertexBuffer_) { vertexBuffer_->Release(); vertexBuffer_ = nullptr; }
    if (inputLayout_) { inputLayout_->Release(); inputLayout_ = nullptr; }
    if (pixelShader_) { pixelShader_->Release(); pixelShader_ = nullptr; }
    if (vertexShader_) { vertexShader_->Release(); vertexShader_ = nullptr; }
    if (depthStencilView_) { depthStencilView_->Release(); depthStencilView_ = nullptr; }
    if (renderTargetView_) { renderTargetView_->Release(); renderTargetView_ = nullptr; }
    if (depthStencilBuffer_) { depthStencilBuffer_->Release(); depthStencilBuffer_ = nullptr; }
    if (context_) { context_->ClearState(); context_->Release(); context_ = nullptr; }
    if (swapChain_) { swapChain_->Release(); swapChain_ = nullptr; }
    if (device_) { device_->Release(); device_ = nullptr; }
}