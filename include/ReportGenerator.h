#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include "Types.h"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

class ReportGenerator {
public:
    ReportGenerator() = default;
    ~ReportGenerator() = default;

    // Generate a detailed report of the solution
    void generateReport(
        const Mesh& mesh,
        const std::vector<double>& solution,
        const std::map<std::string, std::string>& equationCoefficients,
        const std::map<std::string, BoundaryConditionData>& boundaryConditions,
        const std::string& filename = "fem_report.txt"
    );

    // Export raw solution data
    void exportSolutionData(
        const Mesh& mesh,
        const std::vector<double>& solution,
        const std::string& filename = "solution_data.txt"
    );

    // Export mesh data
    void exportMeshData(
        const Mesh& mesh,
        const std::string& filename = "mesh_data.txt"
    );

private:
    // Helper functions to format report sections
    std::string generateSolutionStatistics(const std::vector<double>& solution);
    std::string generateMeshInformation(const Mesh& mesh);
    std::string generateEquationDetails(const std::map<std::string, std::string>& coefficients);
    std::string generateBoundaryConditionsDetails(const std::map<std::string, BoundaryConditionData>& boundaryConditions);
    std::string generateProcessSteps();
};

#endif // REPORTGENERATOR_H