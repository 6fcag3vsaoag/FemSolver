#ifndef RESETMANAGER_H
#define RESETMANAGER_H

#include <windows.h>
#include "Types.h"  // For Mesh and other core types
#include "GUIAppTypes.h"  // For AppData
#include <string>

// Forward declarations
class FemSolver;

// Reset manager class to handle all reset-related functionality
class ResetManager {
public:
    // Constructor
    ResetManager();
    
    // Destructor
    ~ResetManager();
    
    // Initialize with app data
    void initialize(AppData* appData);
    
    // Reset to default values
    void resetToDefaults();
    
    // Reset specific parameters
    void resetGridParams();
    void resetCoefficients();
    void resetBoundaryConditions();
    
    // Get reset status
    std::string getResetStatus() const;
    
private:
    AppData* m_appData;       // Pointer to the app data
    std::string m_status;     // Reset status
};

#endif // RESETMANAGER_H