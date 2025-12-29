#ifndef VISUALIZATIONMANAGER_H
#define VISUALIZATIONMANAGER_H

#include <windows.h>
#include "Types.h"  // For Mesh and other core types
#include "GUIAppTypes.h"  // For AppData if needed
#include "IVisualizer.h"  // For IVisualizer interface
#include <vector>
#include <string>

// Visualization manager class to handle all visualization-related functionality
class VisualizationManager {
public:
    // Constructor
    VisualizationManager();

    // Destructor
    ~VisualizationManager();

    // Initialize visualization with an IVisualizer instance
    void initialize(IVisualizer* visualizer);

    // Render solution data
    void renderSolution(const Mesh& mesh, const std::vector<double>& solution, int Nx, int Ny, const std::string& title);

    // Update visualization based on current AppData
    void updateVisualization(AppData& appData, const Mesh& mesh, const std::vector<double>& solution);

    // Export visualization to file
    bool exportVisualization(const std::string& filename);

    // Get the current visualizer instance
    IVisualizer* getVisualizer() const;

    // Set a new visualizer instance
    void setVisualizer(IVisualizer* visualizer);

private:
    IVisualizer* m_visualizer;  // Pointer to the visualizer instance
};

#endif // VISUALIZATIONMANAGER_H