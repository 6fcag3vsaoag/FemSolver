#ifndef DIRECTXVISUALIZER_H
#define DIRECTXVISUALIZER_H

#include "../IVisualizer.h"
#include "Types.h"
#include <vector>
#include <string>
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

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
 * @brief Concrete implementation of IVisualizer using DirectX 11 for 3D rendering.
 *
 * This class implements 3D visualization using DirectX 11, replacing the previous GDI-based 2D visualization.
 */
class DirectXVisualizer : public IVisualizer {
public:
    DirectXVisualizer();
    ~DirectXVisualizer() override;

    /**
     * @brief Sets the handle to the window where the visualization should be drawn.
     * @param handle The HWND of the target window.
     */
    void setWindowHandle(HWND handle) override;

    /**
     * @brief Renders the solution on the assigned window handle using DirectX 11.
     * @param mesh The mesh data to visualize.
     * @param solution The solution values corresponding to the mesh nodes.
     * @param Nx Number of nodes in X direction for structured grids.
     * @param Ny Number of nodes in Y direction for structured grids.
     * @param title An optional title for the visualization.
     */
    void render(const Mesh& mesh, const std::vector<double>& solution, int Nx, int Ny, const std::string& title) override;

    /**
     * @brief Initializes DirectX resources for the visualizer.
     * @return True if initialization was successful, false otherwise.
     */
    bool initialize();

    /**
     * @brief Triggers a manual render update (useful for continuous rendering).
     */
    void render() override;

    /**
     * @brief Resizes the visualization when the window is resized.
     * @param width New width of the window.
     * @param height New height of the window.
     */
        void resize(int width, int height) override;
    
        // Handle mouse input for camera control
        void handleMouseInput(int mouseX, int mouseY, bool leftButton, bool rightButton);
    
        // Handle mouse wheel for zoom
        void handleMouseWheel(int delta);
    
    private:
        HWND hwndTarget_; ///< Handle to the window where rendering occurs.
    
        // DirectX 11 resources
        ID3D11Device* device_;
        ID3D11DeviceContext* context_;
        IDXGISwapChain* swapChain_;
        ID3D11RenderTargetView* renderTargetView_;
            ID3D11DepthStencilView* depthStencilView_;
            ID3D11Texture2D* depthStencilBuffer_;
            ID3D11RasterizerState* rasterizerState_;
            ID3D11RasterizerState* wireframeState_;
        
            // Shaders
            ID3D11VertexShader* vertexShader_;
            ID3D11PixelShader* pixelShader_;        ID3D11InputLayout* inputLayout_;
        ID3D11Buffer* constantBuffer_;
    
        // Mesh data
        ID3D11Buffer* vertexBuffer_;
        ID3D11Buffer* indexBuffer_;
        unsigned int vertexCount_;
        unsigned int indexCount_;
    
        // Current visualization data
        Mesh currentMesh_;
        std::vector<double> currentSolution_;
        int currentNx_;
        int currentNy_;
        std::string currentTitle_;
        bool hasSolution_;
    
        // View/projection matrices
        DirectX::XMMATRIX worldMatrix_;
        DirectX::XMMATRIX viewMatrix_;
        DirectX::XMMATRIX projectionMatrix_;
    
            // Camera parameters
            float cameraRotationX_;
            float cameraRotationY_;
            float cameraDistance_;
            float cameraTargetX_;
            float cameraTargetY_;
            float cameraTargetZ_;
        
            // Domain dimensions
            float domainLx_;
            float domainLy_;
            float domainLz_; // To store max solution value for Y-axis scaling
        
            // Initialize DirectX 11
            bool initializeDirectX();    
        // Create shaders
        bool createShaders();
    
        // Create constant buffer
        bool createConstantBuffer();
    
        // Create vertex/index buffers from mesh data
        bool createMeshBuffers(const Mesh& mesh, const std::vector<double>& solution, int Nx, int Ny);
    
        // Update view/projection matrices
        void updateMatrices();
    
        // Update camera matrices
        void updateCameraMatrices();
    
        // Clear the render target
        void clearRenderTarget();
    
        // Present the rendered frame
        void present();
    
        // Render coordinate axes
        void renderAxes();
    
        // Render grid
        void renderGrid();
    
        // Render legend for solution values
        void renderLegend();
    };
    
    #endif // DIRECTXVISUALIZER_H
    