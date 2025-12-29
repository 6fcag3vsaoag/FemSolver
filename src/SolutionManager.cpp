#include "../include/SolutionManager.h"
#include "../include/EllipticApp.h"
#include "../include/FemSolver.h"  // Include FemSolver.h to define FemSolver class
#include <windows.h>

// Constructor
SolutionManager::SolutionManager() : m_solver(nullptr), m_appData(nullptr) {
}

// Destructor
SolutionManager::~SolutionManager() {
    // Clean up if needed
}

// Initialize with solver and app data
void SolutionManager::initialize(FemSolver* solver, AppData* appData) {
    m_solver = solver;
    m_appData = appData;
}

// Solve the equation with current parameters
bool SolutionManager::solveCurrentProblem() {
    if (!m_solver || !m_appData) {
        return false;
    }

    try {
        EllipticApp* ellipticApp = m_solver->getApp();
        if (ellipticApp) {
            // Get boundary condition types from combo boxes
            // Since we can't directly access HWND from here, we'll need to pass the values
            // This is a simplified approach - in practice, you'd need to get these values differently
            std::string westBC = "dirichlet"; // This would need to be determined differently
            std::string eastBC = "dirichlet";
            std::string southBC = "dirichlet";
            std::string northBC = "dirichlet";
            
            // For now, using default values - these would need to be retrieved from the UI
            double westVal = 0.0;
            double eastVal = 0.0;
            double southVal = 0.0;
            double northVal = 0.0;

            // Solve the equation with the parameters obtained from the app data
            ellipticApp->solveWithParameters(
                m_appData->Lx, m_appData->Ly, m_appData->Nx, m_appData->Ny,
                m_appData->a11Func, m_appData->a12Func, m_appData->a22Func,
                m_appData->b1Func, m_appData->b2Func, m_appData->cFunc, m_appData->fFunc,
                westBC, eastBC, southBC, northBC,
                westVal, eastVal, southVal, northVal
            );

            // Get the solution and mesh
            m_solution = ellipticApp->getSolution();
            m_mesh = ellipticApp->getMesh();
            
            m_status = "Solution computed successfully!";
            return true;
        } else {
            m_status = "Failed to access solver application.";
            return false;
        }
    } catch (const std::exception& e) {
        m_status = std::string("Error: ") + e.what();
        return false;
    }
}

// Solve with specific parameters
bool SolutionManager::solveWithParameters(
    double Lx, double Ly, int Nx, int Ny,
    const std::string& a11Func, const std::string& a12Func, const std::string& a22Func,
    const std::string& b1Func, const std::string& b2Func, const std::string& cFunc, const std::string& fFunc,
    const std::string& westBC, const std::string& eastBC, const std::string& southBC, const std::string& northBC,
    double westVal, double eastVal, double southVal, double northVal
) {
    if (!m_solver) {
        return false;
    }

    try {
        EllipticApp* ellipticApp = m_solver->getApp();
        if (ellipticApp) {
            // Solve the equation with the provided parameters
            ellipticApp->solveWithParameters(
                Lx, Ly, Nx, Ny,
                a11Func, a12Func, a22Func,
                b1Func, b2Func, cFunc, fFunc,
                westBC, eastBC, southBC, northBC,
                westVal, eastVal, southVal, northVal
            );

            // Get the solution and mesh
            m_solution = ellipticApp->getSolution();
            m_mesh = ellipticApp->getMesh();
            
            m_status = "Solution computed successfully!";
            return true;
        } else {
            m_status = "Failed to access solver application.";
            return false;
        }
    } catch (const std::exception& e) {
        m_status = std::string("Error: ") + e.what();
        return false;
    }
}

// Get the solution data
const std::vector<double>& SolutionManager::getSolution() const {
    return m_solution;
}

// Get the mesh data
const Mesh& SolutionManager::getMesh() const {
    return m_mesh;
}

// Update solution based on current AppData
bool SolutionManager::updateSolutionFromAppData() {
    if (!m_appData) {
        return false;
    }
    
    // Update internal parameters from AppData
    // This is a simplified approach - in practice, you'd map the UI values to the solution parameters
    return solveCurrentProblem();
}

// Get solution status
std::string SolutionManager::getSolutionStatus() const {
    return m_status;
}