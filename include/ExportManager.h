#ifndef EXPORTMANAGER_H
#define EXPORTMANAGER_H

#include <windows.h>
#include "Types.h"  // For Mesh and other core types
#include "GUIAppTypes.h"  // For AppData
#include <vector>
#include <string>

// Forward declarations
class FemSolver;
class EllipticApp;

// Export manager class to handle all export-related functionality
class ExportManager {
public:
    // Constructor
    ExportManager();
    
    // Destructor
    ~ExportManager();
    
    // Initialize with solver and app data
    void initialize(FemSolver* solver, AppData* appData);
    
    // Export solution data to a file
    bool exportSolutionToFile(const std::string& filename);
    
    // Export solution data with current parameters
    bool exportCurrentSolution();
    
    // Export solution with specific data
    bool exportSolution(const Mesh& mesh, const std::vector<double>& solution, 
                       int Nx, int Ny, double Lx, double Ly,
                       const std::string& a11, const std::string& a12, const std::string& a22,
                       const std::string& b1, const std::string& b2, const std::string& c, const std::string& f,
                       const std::map<std::string, std::string>& boundaryConditions,
                       const std::string& title = "FEM Solution Export");
    
    // Get export status
    std::string getExportStatus() const;
    
private:
    FemSolver* m_solver;      // Pointer to the solver instance
    AppData* m_appData;       // Pointer to the app data
    std::string m_status;     // Export status
};

#endif // EXPORTMANAGER_H