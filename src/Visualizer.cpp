#include "Visualizer.h"
#include "ReportGenerator.h"
#include <iostream>
#include <fstream>
#include <sstream>


Visualizer::Visualizer() {
    reportGenerator_ = std::make_unique<ReportGenerator>();
}

Visualizer::~Visualizer() = default;

void Visualizer::plotSolution(const Mesh& mesh, const std::vector<double>& solution, const std::string& title) {
    // Store the data temporarily
    mesh_ = mesh;
    solution_ = solution;
    title_ = title;

    std::cout << "Plotting solution with " << mesh.nodes.size() << " nodes and "
              << mesh.elements.size() << " elements" << std::endl;

    // In a real implementation, this would create a plot using a graphics library
    // For now, we'll just print some information
    std::cout << "Solution range: [" << *std::min_element(solution.begin(), solution.end())
              << ", " << *std::max_element(solution.begin(), solution.end()) << "]" << std::endl;
}

void Visualizer::exportPlot(const std::string& filename) {
    // Export solution data to a file that can be plotted externally
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }

    // Write header
    file << "# Solution data for: " << title_ << std::endl;
    file << "# Node_Id, X, Y, Solution_Value" << std::endl;

    // Write node data
    for (size_t i = 0; i < mesh_.nodes.size(); ++i) {
        file << i << ", "
             << mesh_.nodes[i].first << ", "
             << mesh_.nodes[i].second << ", "
             << solution_[i] << std::endl;
    }

    // Write element data
    file << std::endl << "# Elements (Node indices)" << std::endl;
    for (const auto& element : mesh_.elements) {
        file << element[0] << " " << element[1] << " " << element[2] << std::endl;
    }

    file.close();
    std::cout << "Plot data exported to: " << filename << std::endl;
}

void Visualizer::generateReport(
    const Mesh& mesh,
    const std::vector<double>& solution,
    const std::map<std::string, std::string>& equationCoefficients,
    const std::map<std::string, BoundaryConditionData>& boundaryConditions,
    const std::string& filename
) {
    if (reportGenerator_) {
        reportGenerator_->generateReport(
            mesh, solution, equationCoefficients, boundaryConditions, filename
        );
        std::cout << "Detailed report generated: " << filename << std::endl;
    } else {
        throw std::runtime_error("Report generator not initialized");
    }
}