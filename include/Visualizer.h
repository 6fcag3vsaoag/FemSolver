#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "Types.h"
#include "ReportGenerator.h"  // Include full header since we use unique_ptr
#include <string>
#include <memory>
#include <vector>
#include <map>

class Visualizer {
public:
    Visualizer();
    ~Visualizer();  // Defined in implementation to handle unique_ptr to forward-declared type

    // Plot the solution
    void plotSolution(const Mesh& mesh, const std::vector<double>& solution, const std::string& title = "Solution");

    // Export visualization to file
    void exportPlot(const std::string& filename);

    // Generate detailed report
    void generateReport(
        const Mesh& mesh,
        const std::vector<double>& solution,
        const std::map<std::string, std::string>& equationCoefficients,
        const std::map<std::string, BoundaryConditionData>& boundaryConditions,
        const std::string& filename = "fem_report.txt"
    );

private:
    // Internal data for visualization
    Mesh mesh_;
    std::vector<double> solution_;
    std::string title_;

    // Report generator
    std::unique_ptr<ReportGenerator> reportGenerator_;
};

#endif // VISUALIZER_H