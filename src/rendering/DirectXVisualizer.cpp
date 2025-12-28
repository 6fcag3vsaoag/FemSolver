#define NOMINMAX
#include "../include/rendering/DirectXVisualizer.h"
#include <algorithm>
#include <cmath>

DirectXVisualizer::DirectXVisualizer()
    : hwndTarget_(nullptr), device_(nullptr), context_(nullptr), swapChain_(nullptr),
      renderTargetView_(nullptr), depthStencilView_(nullptr), depthStencilBuffer_(nullptr),
      rasterizerState_(nullptr), wireframeState_(nullptr), // Initialize states
      vertexShader_(nullptr), pixelShader_(nullptr), inputLayout_(nullptr), constantBuffer_(nullptr),
      vertexBuffer_(nullptr), indexBuffer_(nullptr), vertexCount_(0), indexCount_(0),
      currentNx_(0), currentNy_(0), hasSolution_(false),
      cameraRotationX_(-0.5f), cameraRotationY_(0.5f), cameraDistance_(5.0f),
      cameraTargetX_(0.0f), cameraTargetY_(0.0f), cameraTargetZ_(0.0f),
      domainLx_(1.0f), domainLy_(1.0f), domainLz_(1.0f) { // Initialize domain dimensions
    // Initialize matrices
    worldMatrix_ = DirectX::XMMatrixIdentity();
    updateCameraMatrices();
}

DirectXVisualizer::~DirectXVisualizer() {
    // Release DirectX resources
    if (rasterizerState_) rasterizerState_->Release();
    if (wireframeState_) wireframeState_->Release();
    if (vertexBuffer_) vertexBuffer_->Release();
    if (indexBuffer_) indexBuffer_->Release();
    if (constantBuffer_) constantBuffer_->Release();
    if (inputLayout_) inputLayout_->Release();
    if (pixelShader_) pixelShader_->Release();
    if (vertexShader_) vertexShader_->Release();
    if (depthStencilView_) depthStencilView_->Release();
    if (renderTargetView_) renderTargetView_->Release();
    if (depthStencilBuffer_) depthStencilBuffer_->Release();
    if (context_) {
        context_->ClearState();
        context_->Release();
    }
    if (swapChain_) swapChain_->Release();
    if (device_) device_->Release();
}

void DirectXVisualizer::setWindowHandle(HWND handle) {
    hwndTarget_ = handle;

    // Initialize DirectX if not already done and we have a valid window handle
    if (handle && !device_) {
        initialize();
    }
}

void DirectXVisualizer::render(const Mesh& mesh, const std::vector<double>& solution, int Nx, int Ny, const std::string& title) {
    if (!device_ || !context_ || !swapChain_) {
        if (!initialize()) {
            // If initialization failed, return early
            return;
        }
    }

    // Additional check for required resources
    if (!constantBuffer_ || !vertexShader_ || !pixelShader_ || !inputLayout_) {
        return;
    }

    // Store current data
    currentMesh_ = mesh;
    currentSolution_ = solution;
    currentNx_ = Nx;
    currentNy_ = Ny;
    currentTitle_ = title;
    hasSolution_ = true;

    // Create or update mesh buffers
    if (!createMeshBuffers(mesh, solution, Nx, Ny)) {
        return;
    }

    // Update matrices
    updateMatrices();

    // Clear the render target
    clearRenderTarget();

    // Set up the viewport
    RECT rect;
    GetClientRect(hwndTarget_, &rect);
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(rect.right - rect.left);
    viewport.Height = static_cast<float>(rect.bottom - rect.top);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    context_->RSSetViewports(1, &viewport);

    // Set constant buffer
    ConstantBuffer cb;
    cb.world = DirectX::XMMatrixTranspose(worldMatrix_);
    cb.view = DirectX::XMMatrixTranspose(viewMatrix_);
    cb.projection = DirectX::XMMatrixTranspose(projectionMatrix_);

    if (constantBuffer_) {
        context_->UpdateSubresource(constantBuffer_, 0, nullptr, &cb, 0, 0);
        context_->VSSetConstantBuffers(0, 1, &constantBuffer_);
    }

    // Set vertex buffer
    UINT stride = sizeof(VertexPosColor);
    UINT offset = 0;
    if (vertexBuffer_) {
        context_->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
    }

    // Set index buffer
    if (indexBuffer_) {
        context_->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R32_UINT, 0);
    }

    // Set primitive topology
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set shaders
    if (vertexShader_) {
        context_->VSSetShader(vertexShader_, nullptr, 0);
    }
    if (pixelShader_) {
        context_->PSSetShader(pixelShader_, nullptr, 0);
    }
    if (inputLayout_) {
        context_->IASetInputLayout(inputLayout_);
    }

    // Draw
    if (indexCount_ > 0) {
        // Draw the solid mesh
        context_->RSSetState(rasterizerState_);
        context_->DrawIndexed(indexCount_, 0, 0);

        // Draw the wireframe overlay
        context_->RSSetState(wireframeState_);
        context_->DrawIndexed(indexCount_, 0, 0);
    }

    // Reset to default state for other rendering
    context_->RSSetState(rasterizerState_);

    // Render coordinate axes
    renderAxes();

    // Render grid
    renderGrid();

    // Present the frame
    present();
}

bool DirectXVisualizer::initialize() {
    if (device_) {
        // Already initialized
        return true;
    }

    if (!hwndTarget_) {
        // No window handle yet, can't initialize
        return false;
    }

    bool success = initializeDirectX();
    if (!success) return false;

    success = createShaders();
    if (!success) return false;

    success = createConstantBuffer();
    if (!success) return false;

    return true;
}

void DirectXVisualizer::resize(int width, int height) {
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

    // Recreate render target view
    ID3D11Texture2D* backBuffer = nullptr;
    result = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (SUCCEEDED(result)) {
        device_->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView_);
        backBuffer->Release();
    }

    // Recreate depth stencil view
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

    result = device_->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer_);
    if (SUCCEEDED(result)) {
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format = depthStencilDesc.Format;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice = 0;

        result = device_->CreateDepthStencilView(depthStencilBuffer_, &depthStencilViewDesc, &depthStencilView_);
        if (SUCCEEDED(result)) {
            context_->OMSetRenderTargets(1, &renderTargetView_, depthStencilView_);
        }
    }
}

void DirectXVisualizer::render() {
    if (!hasSolution_ || !device_ || !context_ || !swapChain_) {
        return;
    }

    // Render with stored data
    render(currentMesh_, currentSolution_, currentNx_, currentNy_, currentTitle_);
}

bool DirectXVisualizer::initializeDirectX() {
    if (device_) return true; // Already initialized

    // Get window dimensions
    RECT rect;
    GetClientRect(hwndTarget_, &rect);
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

    // Create swap chain description
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwndTarget_;
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

    if (FAILED(result)) {
        return false;
    }

    // Create render target view
    ID3D11Texture2D* backBuffer = nullptr;
    result = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(result)) {
        return false;
    }

    result = device_->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView_);
    backBuffer->Release();
    if (FAILED(result)) {
        return false;
    }

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

    result = device_->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer_);
    if (FAILED(result)) {
        return false;
    }

    // Create depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format = depthStencilDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    result = device_->CreateDepthStencilView(depthStencilBuffer_, &depthStencilViewDesc, &depthStencilView_);
    if (FAILED(result)) {
        return false;
    }

    // Bind the render target view and depth stencil view to the output render pipeline
    context_->OMSetRenderTargets(1, &renderTargetView_, depthStencilView_);

    // Set up the viewport
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    context_->RSSetViewports(1, &viewport);

    // Create rasterizer states (solid and wireframe)
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE; // Disable culling to prevent holes
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.SlopeScaledDepthBias = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.ScissorEnable = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.AntialiasedLineEnable = false;
    
    result = device_->CreateRasterizerState(&rasterDesc, &rasterizerState_);
    if (FAILED(result)) return false;

    rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
    result = device_->CreateRasterizerState(&rasterDesc, &wireframeState_);
    if (FAILED(result)) return false;

    context_->RSSetState(rasterizerState_);

    return true;
}

bool DirectXVisualizer::createShaders() {
    // Simple vertex shader
    const char* vertexShaderCode =
        "cbuffer ConstantBuffer : register(b0) {"
        "    matrix world;"
        "    matrix view;"
        "    matrix projection;"
        "}"
        "struct VS_INPUT {"
        "    float3 pos : POSITION;"
        "    float4 color : COLOR0;"
        "};"
        "struct VS_OUTPUT {"
        "    float4 pos : SV_POSITION;"
        "    float4 color : COLOR0;"
        "};"
        "VS_OUTPUT main(VS_INPUT input) {"
        "    VS_OUTPUT output;"
        "    float4 pos = float4(input.pos, 1.0f);"
        "    pos = mul(pos, world);"
        "    pos = mul(pos, view);"
        "    pos = mul(pos, projection);"
        "    output.pos = pos;"
        "    output.color = input.color;"
        "    return output;"
        "}";

    // Simple pixel shader
    const char* pixelShaderCode =
        "struct PS_INPUT {"
        "    float4 pos : SV_POSITION;"
        "    float4 color : COLOR0;"
        "};"
        "float4 main(PS_INPUT input) : SV_TARGET {"
        "    return input.color;"
        "}";

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

    if (FAILED(result)) {
        vertexShaderBlob->Release();
        pixelShaderBlob->Release();
        return false;
    }

    // Create input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    UINT numElements = ARRAYSIZE(layout);

    result = device_->CreateInputLayout(
        layout,
        numElements,
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        &inputLayout_
    );

    vertexShaderBlob->Release();
    pixelShaderBlob->Release();

    return SUCCEEDED(result);
}

bool DirectXVisualizer::createConstantBuffer() {
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(ConstantBuffer);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    HRESULT result = device_->CreateBuffer(&bufferDesc, nullptr, &constantBuffer_);
    return SUCCEEDED(result);
}

bool DirectXVisualizer::createMeshBuffers(const Mesh& mesh, const std::vector<double>& solution, int Nx, int Ny) {
    if (solution.empty() || mesh.nodes.empty() || mesh.elements.empty()) {
        return false;
    }

    // Calculate the range of solution values for color mapping and Z-axis
    double minVal = *std::min_element(solution.begin(), solution.end());
    double maxVal = *std::max_element(solution.begin(), solution.end());
    domainLz_ = static_cast<float>(maxVal);
    double range = maxVal - minVal;
    if (range == 0.0) range = 1.0; // Avoid division by zero

    // Create vertices from mesh nodes with color based on solution values
    std::vector<VertexPosColor> vertices;
    vertices.reserve(mesh.nodes.size());

    // Calculate domain center and dimensions for proper positioning
    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (const auto& node : mesh.nodes) {
        minX = std::min(minX, node.first);
        maxX = std::max(maxX, node.first);
        minY = std::min(minY, node.second);
        maxY = std::max(maxY, node.second);
    }

    domainLx_ = static_cast<float>(maxX - minX);
    domainLy_ = static_cast<float>(maxY - minY);

    double centerX = (minX + maxX) / 2.0;
    double centerY = (minY + maxY) / 2.0;

    // Update camera target to the center of the now-centered mesh
    cameraTargetX_ = 0.0f;
    cameraTargetZ_ = 0.0f;
    cameraTargetY_ = static_cast<float>((minVal + maxVal) / 2.0);

    for (size_t i = 0; i < mesh.nodes.size(); ++i) {
        VertexPosColor vertex;
        vertex.position = DirectX::XMFLOAT3(
            static_cast<float>(mesh.nodes[i].first) - static_cast<float>(centerX),  // x coordinate is first in pair, centered
            static_cast<float>(solution[i]), // Use solution value as Y coordinate for 3D visualization
            static_cast<float>(mesh.nodes[i].second) - static_cast<float>(centerY)  // y coordinate is second in pair, centered
        );

        // Map solution value to color (blue to red gradient)
        float normalizedValue = static_cast<float>((solution[i] - minVal) / range);
        vertex.color = DirectX::XMFLOAT4(
            normalizedValue,    // Red component
            0.0f,               // Green component
            1.0f - normalizedValue, // Blue component
            1.0f                // Alpha
        );

        vertices.push_back(vertex);
    }

    // Create indices from mesh elements (triangles)
    std::vector<unsigned long> indices;
    for (const auto& element : mesh.elements) {
        // Each element is a triangle with 3 nodes (std::array<int, 3>)
        for (int j = 0; j < 3; ++j) {
            indices.push_back(element[j]);  // Access array element directly
        }
    }

    // Release existing buffers if they exist
    if (vertexBuffer_) {
        vertexBuffer_->Release();
        vertexBuffer_ = nullptr;
    }
    if (indexBuffer_) {
        indexBuffer_->Release();
        indexBuffer_ = nullptr;
    }

    // Create new vertex buffer
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT; // Changed from DYNAMIC for simplicity
    vertexBufferDesc.ByteWidth = sizeof(VertexPosColor) * static_cast<UINT>(vertices.size());
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices.data();

    HRESULT result = device_->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer_);
    if (FAILED(result)) {
        return false;
    }

    // Create new index buffer
    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT; // Changed from DYNAMIC for simplicity
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * static_cast<UINT>(indices.size());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices.data();

    result = device_->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer_);
    if (FAILED(result)) {
        // Clean up vertex buffer if index buffer creation failed
        if (vertexBuffer_) {
            vertexBuffer_->Release();
            vertexBuffer_ = nullptr;
        }
        return false;
    }

    vertexCount_ = static_cast<UINT>(vertices.size());
    indexCount_ = static_cast<UINT>(indices.size());

    return true;
}

void DirectXVisualizer::updateMatrices() {
    // Get window dimensions for aspect ratio
    RECT rect;
    GetClientRect(hwndTarget_, &rect);
    float aspectRatio = static_cast<float>(rect.right - rect.left) / static_cast<float>(rect.bottom - rect.top);

    // Update projection matrix
    projectionMatrix_ = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4,  // 45 degree field of view
        aspectRatio,         // Aspect ratio
        0.1f,               // Near plane
        100.0f              // Far plane
    );
}

void DirectXVisualizer::updateCameraMatrices() {
    // Calculate camera position based on rotation and distance
    float cameraX = cameraTargetX_ + cameraDistance_ * sinf(cameraRotationY_) * cosf(cameraRotationX_);
    float cameraY = cameraTargetY_ + cameraDistance_ * sinf(cameraRotationX_);
    float cameraZ = cameraTargetZ_ + cameraDistance_ * cosf(cameraRotationY_) * cosf(cameraRotationX_);

    // Calculate look-at point (always looking at the center)
    DirectX::XMVECTOR cameraPos = DirectX::XMVectorSet(cameraX, cameraY, cameraZ, 0.0f);
    DirectX::XMVECTOR lookAt = DirectX::XMVectorSet(cameraTargetX_, cameraTargetY_, cameraTargetZ_, 0.0f);

    // Use a fixed up vector (world up) for consistent camera behavior
    DirectX::XMVECTOR upVec = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // Create view matrix
    viewMatrix_ = DirectX::XMMatrixLookAtLH(cameraPos, lookAt, upVec);
}

void DirectXVisualizer::handleMouseInput(int mouseX, int mouseY, bool leftButton, bool rightButton) {
    static bool firstMouse = true;
    static int lastMouseX = mouseX;
    static int lastMouseY = mouseY;

    if (firstMouse) {
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        firstMouse = false;
    }

    int deltaX = mouseX - lastMouseX;
    int deltaY = mouseY - lastMouseY;

    lastMouseX = mouseX;
    lastMouseY = mouseY;

    if (leftButton) {
        // Rotate camera
        cameraRotationY_ += deltaX * 0.01f;
        cameraRotationX_ += deltaY * 0.01f;

        // Limit vertical rotation to avoid flipping
        cameraRotationX_ = std::max(-DirectX::XM_PI / 2.0f + 0.1f, std::min(DirectX::XM_PI / 2.0f - 0.1f, cameraRotationX_));
    }

    if (rightButton) {
        // Pan camera (simplified - just move the target)
        // Convert mouse movement to world space movement
        float panSpeed = 0.01f * cameraDistance_;
        cameraTargetX_ -= deltaX * panSpeed;
        cameraTargetZ_ += deltaY * panSpeed;  // Note: Z is inverted for screen coordinates
    }

    // Update camera matrices
    updateCameraMatrices();

    // Force immediate rendering to reflect the camera changes
    render();
}

void DirectXVisualizer::handleMouseWheel(int delta) {
    // Zoom in/out
    if (delta > 0) {
        cameraDistance_ *= 0.9f;
    } else {
        cameraDistance_ *= 1.1f;
    }

    // Limit zoom range
    cameraDistance_ = std::max(1.0f, std::min(50.0f, cameraDistance_));

    // Update camera matrices
    updateCameraMatrices();

    // Force immediate rendering to reflect the camera changes
    render();
}

void DirectXVisualizer::clearRenderTarget() {
    float clearColor[4] = { 0.0f, 0.0f, 0.2f, 1.0f }; // Dark blue background
    context_->ClearRenderTargetView(renderTargetView_, clearColor);
    context_->ClearDepthStencilView(depthStencilView_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DirectXVisualizer::present() {
    swapChain_->Present(0, 0);
}

void DirectXVisualizer::renderAxes() {
    if (!device_ || !context_) return;

    // Define axis vertices based on domain size with a 20% margin
    float margin = 1.2f;
    float x_len = (domainLx_ / 2.0f) * margin;
    float y_len = domainLz_ * margin; // Use max solution value for Y-axis length
    float z_len = (domainLy_ / 2.0f) * margin;

    std::vector<VertexPosColor> axisVertices = {
        // X-axis (red)
        {{-x_len, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{x_len, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},

        // Y-axis (green)
        {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.0f, y_len, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},

        // Z-axis (blue)
        {{0.0f, 0.0f, -z_len}, {0.0f, 0.0f, 1.0f, 1.0f}},
        {{0.0f, 0.0f, z_len}, {0.0f, 0.0f, 1.0f, 1.0f}}
    };

    std::vector<unsigned long> axisIndices = { 0, 1, 2, 3, 4, 5 };

    // Create temporary buffers for axes
    ID3D11Buffer* tempVertexBuffer = nullptr;
    ID3D11Buffer* tempIndexBuffer = nullptr;

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.ByteWidth = sizeof(VertexPosColor) * static_cast<UINT>(axisVertices.size());
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = axisVertices.data();

    HRESULT result = device_->CreateBuffer(&vertexBufferDesc, &vertexData, &tempVertexBuffer);
    if (FAILED(result)) return;

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * static_cast<UINT>(axisIndices.size());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = axisIndices.data();

    result = device_->CreateBuffer(&indexBufferDesc, &indexData, &tempIndexBuffer);
    if (FAILED(result)) {
        tempVertexBuffer->Release();
        return;
    }

    // Save current buffers and topology
    ID3D11Buffer* oldVertexBuffer = nullptr;
    ID3D11Buffer* oldIndexBuffer = nullptr;
    UINT oldStride, oldOffset;
    D3D11_PRIMITIVE_TOPOLOGY oldTopology;
    context_->IAGetVertexBuffers(0, 1, &oldVertexBuffer, &oldStride, &oldOffset);
    context_->IAGetIndexBuffer(&oldIndexBuffer, nullptr, nullptr);
    context_->IAGetPrimitiveTopology(&oldTopology);

    // Set axis buffers
    UINT stride = sizeof(VertexPosColor);
    UINT offset = 0;
    context_->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
    context_->IASetIndexBuffer(tempIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // Set constant buffer for axes
    ConstantBuffer cb;
    cb.world = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
    cb.view = DirectX::XMMatrixTranspose(viewMatrix_);
    cb.projection = DirectX::XMMatrixTranspose(projectionMatrix_);
    context_->UpdateSubresource(constantBuffer_, 0, nullptr, &cb, 0, 0);

    // Draw axes
    context_->DrawIndexed(static_cast<UINT>(axisIndices.size()), 0, 0);

    // Restore original buffers and topology
    context_->IASetVertexBuffers(0, 1, &oldVertexBuffer, &oldStride, &oldOffset);
    if(oldIndexBuffer) {
        context_->IASetIndexBuffer(oldIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        oldIndexBuffer->Release();
    }
    if(oldVertexBuffer) oldVertexBuffer->Release();
    context_->IASetPrimitiveTopology(oldTopology);

    // Release temporary buffers
    tempVertexBuffer->Release();
    tempIndexBuffer->Release();
}

void DirectXVisualizer::renderGrid() {
    if (!device_ || !context_ || !hasSolution_) return;

    std::vector<VertexPosColor> gridVertices;
    
    float halfLx = domainLx_ / 2.0f;
    float halfLy = domainLy_ / 2.0f;

    // Use currentNx_ and currentNy_ for grid lines
    int nx = currentNx_ > 1 ? currentNx_ : 2;
    int ny = currentNy_ > 1 ? currentNy_ : 2;

    // Vertical lines (parallel to Z-axis)
    for (int i = 0; i < nx; ++i) {
        float x = -halfLx + (static_cast<float>(i) / (nx - 1)) * domainLx_;
        gridVertices.push_back({{x, 0.0f, -halfLy}, {0.4f, 0.4f, 0.4f, 0.6f}});
        gridVertices.push_back({{x, 0.0f, halfLy}, {0.4f, 0.4f, 0.4f, 0.6f}});
    }

    // Horizontal lines (parallel to X-axis)
    for (int i = 0; i < ny; ++i) {
        float z = -halfLy + (static_cast<float>(i) / (ny - 1)) * domainLy_;
        gridVertices.push_back({{-halfLx, 0.0f, z}, {0.4f, 0.4f, 0.4f, 0.6f}});
        gridVertices.push_back({{halfLx, 0.0f, z}, {0.4f, 0.4f, 0.4f, 0.6f}});
    }

    if (gridVertices.empty()) return;

    // Create temporary vertex buffer
    ID3D11Buffer* tempVertexBuffer = nullptr;
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.ByteWidth = sizeof(VertexPosColor) * static_cast<UINT>(gridVertices.size());
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    
    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = gridVertices.data();

    HRESULT result = device_->CreateBuffer(&vertexBufferDesc, &vertexData, &tempVertexBuffer);
    if (FAILED(result)) return;
    
    // Save current state
    ID3D11Buffer* oldVertexBuffer = nullptr;
    UINT oldStride, oldOffset;
    D3D11_PRIMITIVE_TOPOLOGY oldTopology;
    context_->IAGetVertexBuffers(0, 1, &oldVertexBuffer, &oldStride, &oldOffset);
    context_->IAGetPrimitiveTopology(&oldTopology);

    // Set grid buffer and topology
    UINT stride = sizeof(VertexPosColor);
    UINT offset = 0;
    context_->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // Set constant buffer
    ConstantBuffer cb;
    cb.world = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
    cb.view = DirectX::XMMatrixTranspose(viewMatrix_);
    cb.projection = DirectX::XMMatrixTranspose(projectionMatrix_);
    context_->UpdateSubresource(constantBuffer_, 0, nullptr, &cb, 0, 0);

    // Draw grid
    context_->Draw(static_cast<UINT>(gridVertices.size()), 0);

    // Restore state
    context_->IASetVertexBuffers(0, 1, &oldVertexBuffer, &oldStride, &oldOffset);
    if(oldVertexBuffer) oldVertexBuffer->Release();
    context_->IASetPrimitiveTopology(oldTopology);

    tempVertexBuffer->Release();
}

void DirectXVisualizer::renderLegend() {
    // For now, this is a placeholder - in a full implementation, we would render
    // a 2D overlay showing the color-to-value mapping
    // This would typically require a separate 2D rendering pipeline
}