#ifndef GDIVISUALIZER_H
#define GDIVISUALIZER_H

#include "IVisualizer.h"
#include "Types.h"
#include <vector>
#include <string>
#include <windows.h> // For HWND and GDI types

/**
 * @brief Concrete implementation of IVisualizer using Windows GDI for 2D rendering.
 *
 * This class encapsulates the existing 2D GDI drawing logic that was previously
 * embedded in GUIApp.cpp's WM_PAINT handler.
 */
class GdiVisualizer : public IVisualizer {
public:
    GdiVisualizer();
    ~GdiVisualizer() override = default;

    /**
     * @brief Sets the handle to the window where the visualization should be drawn.
     * @param handle The HWND of the target window.
     */
    void setWindowHandle(HWND handle) override;

    /**
     * @brief Renders the solution on the assigned window handle using GDI.
     * @param mesh The mesh data to visualize.
     * @param solution The solution values corresponding to the mesh nodes.
     * @param title An optional title for the visualization (currently not used in GDI implementation).
     */
    void render(const Mesh& mesh, const std::vector<double>& solution, int Nx, int Ny, const std::string& title) override;

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

    /**
     * @brief Handles mouse input for camera control.
     * @param mouseX X coordinate of the mouse.
     * @param mouseY Y coordinate of the mouse.
     * @param leftButton Whether the left mouse button is pressed.
     * @param rightButton Whether the right mouse button is pressed.
     */
    void handleMouseInput(int mouseX, int mouseY, bool leftButton, bool rightButton) override;

    /**
     * @brief Handles mouse wheel input for zoom control.
     * @param delta Mouse wheel delta.
     */
    void handleMouseWheel(int delta) override;

    /**
     * @brief Renders coordinate axes.
     */
    void renderAxes() override {}

    /**
     * @brief Renders grid.
     */
    void renderGrid() override {}

    /**
     * @brief Renders legend for solution values.
     */
    void renderLegend() override {}

    // This method is called by the WNDPROC to perform the actual drawing
    void drawInternal(HDC hdc, const RECT& rect);

private:
    HWND hwndTarget_; ///< Handle to the window where rendering occurs.
    Mesh mesh_; ///< Last mesh data rendered.
    std::vector<double> solution_; ///< Last solution data rendered.
    int Nx_; ///< Number of nodes in X direction
    int Ny_; ///< Number of nodes in Y direction
    std::string title_; ///< Title for the visualization
    bool hasSolution_ = false; ///< Flag indicating if there's solution data to render.
};

#endif // GDIVISUALIZER_H
