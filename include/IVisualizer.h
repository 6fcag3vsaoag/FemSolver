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
};

#endif // IVISUALIZER_H
