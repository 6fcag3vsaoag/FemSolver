#include "../include/VisualizationManager.h"
#include "../include/GdiVisualizer.h"
#include <windows.h>

// Constructor
VisualizationManager::VisualizationManager() : m_visualizer(nullptr) {
}

// Destructor
VisualizationManager::~VisualizationManager() {
    // Clean up if needed
}

// Initialize visualization with a GdiVisualizer instance
void VisualizationManager::initialize(GdiVisualizer* visualizer) {
    m_visualizer = visualizer;
}

// Render solution data
void VisualizationManager::renderSolution(const Mesh& mesh, const std::vector<double>& solution, int Nx, int Ny, const std::string& title) {
    if (m_visualizer) {
        m_visualizer->render(mesh, solution, Nx, Ny, title);
    }
}

// Update visualization based on current AppData
void VisualizationManager::updateVisualization(AppData& appData, const Mesh& mesh, const std::vector<double>& solution) {
    if (m_visualizer) {
        // Update the visualizer with the new data
        m_visualizer->render(mesh, solution, appData.Nx, appData.Ny, "Solution Visualization");
        
        // Optionally update the visualization frame window
        if (appData.hVisualFrame) {
            InvalidateRect(appData.hVisualFrame, NULL, TRUE);
            UpdateWindow(appData.hVisualFrame);
        }
    }
}

// Export visualization to file
bool VisualizationManager::exportVisualization(const std::string& filename) {
    // Implementation would depend on the specific export requirements
    // For now, this is a placeholder
    return false;
}

// Get the current visualizer instance
GdiVisualizer* VisualizationManager::getVisualizer() const {
    return m_visualizer;
}