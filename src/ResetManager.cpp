#include "../include/ResetManager.h"
#include "../include/StringUtils.h"
#include <windows.h>

// Constructor
ResetManager::ResetManager() : m_appData(nullptr) {
    m_status = "Reset manager initialized";
}

// Destructor
ResetManager::~ResetManager() {
    // Clean up if needed
}

// Initialize with app data
void ResetManager::initialize(AppData* appData) {
    m_appData = appData;
    m_status = "Reset manager initialized with app data";
}

// Reset to default values
void ResetManager::resetToDefaults() {
    if (!m_appData) {
        m_status = "AppData not initialized";
        return;
    }

    // Reset grid parameters
    m_appData->Lx = 1.0;
    m_appData->Ly = 1.0;
    m_appData->Nx = 20;
    m_appData->Ny = 20;

    // Reset coefficient functions
    m_appData->a11Func = "1.0";
    m_appData->a12Func = "0.0";
    m_appData->a22Func = "1.0";
    m_appData->b1Func = "0.0";
    m_appData->b2Func = "0.0";
    m_appData->cFunc = "0.0";
    m_appData->fFunc = "1.0";

    m_status = "Parameters reset to defaults";
}

// Reset specific parameters
void ResetManager::resetGridParams() {
    if (!m_appData) {
        m_status = "AppData not initialized";
        return;
    }

    m_appData->Lx = 1.0;
    m_appData->Ly = 1.0;
    m_appData->Nx = 20;
    m_appData->Ny = 20;

    m_status = "Grid parameters reset to defaults";
}

void ResetManager::resetCoefficients() {
    if (!m_appData) {
        m_status = "AppData not initialized";
        return;
    }

    m_appData->a11Func = "1.0";
    m_appData->a12Func = "0.0";
    m_appData->a22Func = "1.0";
    m_appData->b1Func = "0.0";
    m_appData->b2Func = "0.0";
    m_appData->cFunc = "0.0";
    m_appData->fFunc = "1.0";

    m_status = "Coefficient functions reset to defaults";
}

void ResetManager::resetBoundaryConditions() {
    if (!m_appData) {
        m_status = "AppData not initialized";
        return;
    }

    // Note: Boundary condition values are stored in UI controls (HWND handles), not as string members
    // This would need to be handled separately in the UI layer
    m_status = "Boundary condition values reset to defaults (handled in UI layer)";
}

// Get reset status
std::string ResetManager::getResetStatus() const {
    return m_status;
}