#ifndef IVISUALIZER_H
#define IVISUALIZER_H

#include "Types.h" // For Mesh structure
#include <vector>    // For std::vector<double> solution
#include <string>    // For std::string title
#include <windows.h> // For HWND

/**
 * @brief Abstract interface for a visualization component.
 *
 * This interface defines the contract for any visualization module
 * that can display the results of the FEM solver.
 * Concrete implementations will provide specific rendering capabilities (e.g., 2D GDI, 3D OpenGL).
 */
class IVisualizer {
public:
    virtual ~IVisualizer() = default;

    /**
     * @brief Sets the handle to the window where the visualization should be drawn.
     * @param handle The HWND of the target window.
     */
    virtual void setWindowHandle(HWND handle) = 0;

    /**
     * @brief Renders the solution on the assigned window handle.
     * @param mesh The mesh data to visualize.
     * @param solution The solution values corresponding to the mesh nodes.
     * @param Nx Number of nodes in X direction for structured grids.
     * @param Ny Number of nodes in Y direction for structured grids.
     * @param title An optional title for the visualization.
     */
    virtual void render(const Mesh& mesh, const std::vector<double>& solution, int Nx, int Ny, const std::string& title) = 0;

    /**
     * @brief Triggers a manual render update (useful for continuous rendering).
     */
    virtual void render() = 0;

    /**
     * @brief Resizes the visualization when the window is resized.
     * @param width New width of the window.
     * @param height New height of the window.
     */
    virtual void resize(int width, int height) = 0;

    /**
     * @brief Handles mouse input for camera control.
     * @param mouseX X coordinate of the mouse.
     * @param mouseY Y coordinate of the mouse.
     * @param leftButton Whether the left mouse button is pressed.
     * @param rightButton Whether the right mouse button is pressed.
     */
    virtual void handleMouseInput(int mouseX, int mouseY, bool leftButton, bool rightButton) = 0;

    /**
     * @brief Handles mouse wheel input for zoom control.
     * @param delta Mouse wheel delta.
     */
    virtual void handleMouseWheel(int delta) = 0;

    /**
     * @brief Renders coordinate axes.
     */
    virtual void renderAxes() = 0;

    /**
     * @brief Renders grid.
     */
    virtual void renderGrid() = 0;

    /**
     * @brief Renders legend for solution values.
     */
    virtual void renderLegend() = 0;
};

#endif // IVISUALIZER_H
