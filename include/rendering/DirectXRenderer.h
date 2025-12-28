#ifndef DIRECTXRENDERER_H
#define DIRECTXRENDERER_H

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <string>

// Vertex structure for our 3D mesh
struct VertexPosColor {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

// Constant buffer for shader
struct ConstantBuffer {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
};

/**
 * @brief Helper class to manage DirectX 11 rendering operations.
 *
 * This class encapsulates the core DirectX 11 rendering functionality
 * and can be used by the DirectXVisualizer.
 */
class DirectXRenderer {
public:
    DirectXRenderer();
    ~DirectXRenderer();

    /**
     * @brief Initializes DirectX 11 device and context.
     * @param hwnd Handle to the window for rendering.
     * @return True if initialization was successful, false otherwise.
     */
    bool initialize(HWND hwnd);

    /**
     * @brief Resizes the swap chain and related resources.
     * @param width New width of the render target.
     * @param height New height of the render target.
     */
    void resize(int width, int height);

    /**
     * @brief Clears the render target with a specified color.
     * @param r Red component (0.0f-1.0f).
     * @param g Green component (0.0f-1.0f).
     * @param b Blue component (0.0f-1.0f).
     * @param a Alpha component (0.0f-1.0f).
     */
    void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);

    /**
     * @brief Renders a 3D mesh with solution values as color mapping.
     * @param vertices Array of vertices with position and color.
     * @param vertexCount Number of vertices.
     * @param indices Array of indices defining triangles.
     * @param indexCount Number of indices.
     * @param world World transformation matrix.
     * @param view View transformation matrix.
     * @param projection Projection transformation matrix.
     */
    void renderMesh(
        const VertexPosColor* vertices,
        unsigned int vertexCount,
        const unsigned long* indices,
        unsigned int indexCount,
        const DirectX::XMMATRIX& world,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& projection
    );

    /**
     * @brief Presents the rendered frame to the screen.
     */
    void present();

    /**
     * @brief Gets the DirectX device.
     */
    ID3D11Device* getDevice() const { return device_; }

    /**
     * @brief Gets the DirectX device context.
     */
    ID3D11DeviceContext* getContext() const { return context_; }

private:
    HWND hwnd_;
    ID3D11Device* device_;
    ID3D11DeviceContext* context_;
    IDXGISwapChain* swapChain_;
    ID3D11RenderTargetView* renderTargetView_;
    ID3D11DepthStencilView* depthStencilView_;
    ID3D11Texture2D* depthStencilBuffer_;
    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;
    ID3D11InputLayout* inputLayout_;
    ID3D11Buffer* vertexBuffer_;
    ID3D11Buffer* indexBuffer_;
    ID3D11Buffer* constantBuffer_;

    // Initialize DirectX resources
    bool createDeviceAndSwapChain();
    bool createRenderTargetView();
    bool createDepthStencilView();
    bool createShaders();
    bool createInputLayout();
    bool createVertexBuffer();
    bool createIndexBuffer();
    bool createConstantBuffer();

    // Release DirectX resources
    void releaseResources();
};

#endif // DIRECTXRENDERER_H