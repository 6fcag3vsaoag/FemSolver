#ifndef GUIAPP_H
#define GUIAPP_H

#include "Types.h"
#include "GdiVisualizer.h" // Include the concrete GDI visualizer
#include "VisualizationManager.h" // Include the visualization manager
#include "SolutionManager.h" // Include the solution manager
#include "ExportManager.h" // Include the export manager
#include "ResetManager.h" // Include the reset manager
#include <string>
#include <memory>          // For std::unique_ptr

// Forward declaration of FemSolver (to avoid circular includes)
class FemSolver;

// Forward declaration of GUI framework classes
// For now, we'll use a placeholder approach
class GUIApp {
public:
    GUIApp();
    ~GUIApp();

    void initialize();
    int run();
    void createMainWindow();
    void setupEventHandlers();
    void updateUI();

private:
    // UI components would be stored here
    void* mainWindow;  // Placeholder for actual UI framework window
    void* meshParamsFrame;  // For mesh parameters
    void* equationCoeffsFrame;  // For equation coefficients
    void* boundaryConditionsFrame;  // For boundary conditions
    void* visualizationFrame;  // For visualization
    void* statusLabel;  // For status messages

    // Application state
    double Lx, Ly;
    int Nx, Ny;
    std::string a11Func, a12Func, a22Func, b1Func, b2Func, cFunc, fFunc;

    // For now, we'll store a reference to the core solver
    class FemSolver* coreSolver;

    // GDI Visualizer instance
    std::unique_ptr<GdiVisualizer> gdiVisualizer_;

    // Visualization manager instance
    std::unique_ptr<VisualizationManager> visualizationManager_;

    // Solution manager instance
    std::unique_ptr<SolutionManager> solutionManager_;

    // Export manager instance
    std::unique_ptr<ExportManager> exportManager_;

    // Reset manager instance
    std::unique_ptr<ResetManager> resetManager_;

    // Methods to update GUI from solver
    friend class EllipticApp;  // Allow EllipticApp to access these private methods
    void setSolver(FemSolver* solver);
    void updateGUIGridParams(double Lx, double Ly, int Nx, int Ny);
    void updateGUICoefficients(
        const std::string& a11, const std::string& a12, const std::string& a22,
        const std::string& b1, const std::string& b2, const std::string& c, const std::string& f);
    void updateGUIVisualization();

    // Run the GUI application
    void runWithSolver(FemSolver* solver);
};

#endif // GUIAPP_H