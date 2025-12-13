#ifndef GUIAPP_H
#define GUIAPP_H

#include "Types.h"
#include <string>
#include <memory>

// Forward declaration of GUI framework classes
// For now, we'll use a placeholder approach
class GUIApp {
public:
    GUIApp();
    ~GUIApp();

    void initialize();
    void run();
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

    // Methods to update GUI from solver
    void setSolver(FemSolver* solver) { coreSolver = solver; }
    void updateGUIGridParams(double Lx, double Ly, int Nx, int Ny);
    void updateGUICoefficients(
        const std::string& a11, const std::string& a12, const std::string& a22,
        const std::string& b1, const std::string& b2, const std::string& c, const std::string& f);
    void updateGUIVisualization();
};

#endif // GUIAPP_H