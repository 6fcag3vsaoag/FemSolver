#ifndef VISUALIZATIONMANAGER_H
#define VISUALIZATIONMANAGER_H

#include <windows.h>
#include "Types.h"  // For Mesh and other core types
#include "GUIAppTypes.h"  // For AppData if needed
#include <vector>
#include <string>

// Forward declaration for GdiVisualizer
class GdiVisualizer;

// Visualization manager class to handle all visualization-related functionality
class VisualizationManager {
public:
    // Constructor
    VisualizationManager();
    
    // Destructor
    ~VisualizationManager();
    
    // Initialize visualization with a GdiVisualizer instance
    void initialize(GdiVisualizer* visualizer);
    
    // Render solution data
    void renderSolution(const Mesh& mesh, const std::vector<double>& solution, int Nx, int Ny, const std::string& title);
    
    // Update visualization based on current AppData
    void updateVisualization(AppData& appData, const Mesh& mesh, const std::vector<double>& solution);
    
    // Export visualization to file
    bool exportVisualization(const std::string& filename);
    
    // Get the current visualizer instance
    GdiVisualizer* getVisualizer() const;
    
private:
    GdiVisualizer* m_visualizer;  // Pointer to the visualizer instance
};

#endif // VISUALIZATIONMANAGER_H