#ifndef SOLUTIONMANAGER_H
#define SOLUTIONMANAGER_H

#include <windows.h>
#include "Types.h"  // For Mesh and other core types
#include "GUIAppTypes.h"  // For AppData
#include <vector>
#include <string>

// Forward declarations
class FemSolver;
class EllipticApp;

// Solution manager class to handle all solution-related functionality
class SolutionManager {
public:
    // Constructor
    SolutionManager();
    
    // Destructor
    ~SolutionManager();
    
    // Initialize with solver and app data
    void initialize(FemSolver* solver, AppData* appData);
    
    // Solve the equation with current parameters
    bool solveCurrentProblem();
    
    // Solve with specific parameters
    bool solveWithParameters(
        double Lx, double Ly, int Nx, int Ny,
        const std::string& a11Func, const std::string& a12Func, const std::string& a22Func,
        const std::string& b1Func, const std::string& b2Func, const std::string& cFunc, const std::string& fFunc,
        const std::string& westBC, const std::string& eastBC, const std::string& southBC, const std::string& northBC,
        double westVal, double eastVal, double southVal, double northVal
    );
    
    // Get the solution data
    const std::vector<double>& getSolution() const;
    
    // Get the mesh data
    const Mesh& getMesh() const;
    
    // Update solution based on current AppData
    bool updateSolutionFromAppData();
    
    // Get solution status
    std::string getSolutionStatus() const;
    
private:
    FemSolver* m_solver;      // Pointer to the solver instance
    AppData* m_appData;       // Pointer to the app data
    std::vector<double> m_solution;  // Solution data
    Mesh m_mesh;              // Mesh data
    std::string m_status;     // Solution status
};

#endif // SOLUTIONMANAGER_H