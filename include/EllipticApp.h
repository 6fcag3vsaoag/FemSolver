#ifndef ELLIPTICAPP_H
#define ELLIPTICAPP_H

#include "Types.h"
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>

// Forward declarations
class MeshGenerator;
class EllipticFEMSolver;
class Visualizer;
class GUIApp;

class EllipticApp {
public:
    EllipticApp();
    ~EllipticApp();  // Defined in implementation to handle unique_ptr to forward-declared type

    // Run the application (with or without GUI)
    void run(bool useGUI = true);

    // Generate mesh
    void generateMesh();

    // Solve the problem
    void solveProblem();

    // Plot the solution
    void plotSolution();

    // Export results
    void exportResults();

    // Reset the problem
    void resetProblem();

    // Set problem parameters
    void setDimensions(double Lx, double Ly) { Lx_ = Lx; Ly_ = Ly; }
    void setNodeCount(int Nx, int Ny) { Nx_ = Nx; Ny_ = Ny; }
    void setCoefficientFunctions(
        CoefficientFunction a11, CoefficientFunction a12, CoefficientFunction a22,
        CoefficientFunction b1, CoefficientFunction b2, CoefficientFunction c, CoefficientFunction f
    );
    void setBoundaryConditions(const std::map<std::string, BoundaryConditionData>& bc) {
        boundaryConditions_ = bc;
    }

    // Get GUI app for setting solver
    GUIApp* getGUIApp() { return guiApp_.get(); }

    // Set solver for GUI app (to avoid exposing FemSolver in header)
    void setSolverForGUI(class FemSolver* solver);

    // GUI interface methods
    void solveWithParameters(
        double Lx, double Ly, int Nx, int Ny,
        const std::string& a11, const std::string& a12, const std::string& a22,
        const std::string& b1, const std::string& b2, const std::string& c, const std::string& f,
        const std::string& westBC, const std::string& eastBC, const std::string& southBC, const std::string& northBC,
        double westVal, double eastVal, double southVal, double northVal
    );

    // Get solution data for visualization
    const std::vector<double>& getSolution() const { return currentSolution_; }
    const Mesh& getMesh() const { return *currentMesh_; }

private:
    // Private members for the application
    std::unique_ptr<MeshGenerator> meshGenerator_;
    std::unique_ptr<EllipticFEMSolver> femSolver_;
    std::unique_ptr<Visualizer> visualizer_;
    std::unique_ptr<GUIApp> guiApp_; // Optional GUI

    // Current solution
    std::unique_ptr<Mesh> currentMesh_;
    std::vector<double> currentSolution_;

    // Problem parameters
    double Lx_, Ly_;
    int Nx_, Ny_;

    // Coefficient functions
    CoefficientFunction a11_func_, a12_func_, a22_func_;
    CoefficientFunction b1_func_, b2_func_, c_func_, f_func_;

    // Boundary conditions
    std::map<std::string, BoundaryConditionData> boundaryConditions_;

    // Setup functions for different problem types
    void setupLaplaceProblem();
    void setupPoissonProblem();
    void setupHelmholtzProblem();
    void setupConvectionDiffusionProblem();
    void setupReactionDiffusionProblem();

    // Console mode function
    void runConsoleMode();
};

#endif // ELLIPTICAPP_H