#include "ReportGenerator.h"
#include <algorithm>
#include <numeric>
#include <iomanip>

void ReportGenerator::generateReport(
    const Mesh& mesh,
    const std::vector<double>& solution,
    const std::map<std::string, std::string>& equationCoefficients,
    const std::map<std::string, BoundaryConditionData>& boundaryConditions,
    const std::string& filename
) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }

    // Write header
    file << "FINITE ELEMENT METHOD SOLVER - DETAILED REPORT\n";
    file << "================================================\n\n";

    // Write process steps
    file << "SOLUTION PROCESS:\n";
    file << "------------------\n";
    file << generateProcessSteps() << "\n";

    // Write mesh information
    file << "MESH INFORMATION:\n";
    file << "-----------------\n";
    file << generateMeshInformation(mesh) << "\n";

    // Write solution statistics
    file << "SOLUTION STATISTICS:\n";
    file << "--------------------\n";
    file << generateSolutionStatistics(solution) << "\n";

    // Write equation details
    file << "EQUATION COEFFICIENTS:\n";
    file << "----------------------\n";
    file << generateEquationDetails(equationCoefficients) << "\n";

    // Write boundary conditions
    file << "BOUNDARY CONDITIONS:\n";
    file << "--------------------\n";
    file << generateBoundaryConditionsDetails(boundaryConditions) << "\n";

    // Write solution data
    file << "SOLUTION DATA:\n";
    file << "--------------\n";
    file << "Node\tX\t\tY\t\tValue\n";
    file << "----\t-\t\t-\t\t-----\n";
    
    for (size_t i = 0; i < std::min(mesh.nodes.size(), solution.size()); ++i) {
        file << i << "\t"
             << std::fixed << std::setprecision(6) << mesh.nodes[i].first << "\t"
             << std::fixed << std::setprecision(6) << mesh.nodes[i].second << "\t"
             << std::fixed << std::setprecision(8) << solution[i] << "\n";
    }

    file.close();
}

void ReportGenerator::exportSolutionData(
    const Mesh& mesh,
    const std::vector<double>& solution,
    const std::string& filename
) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }

    // Write header
    file << "# Solution data\n";
    file << "# Format: Node_Index X_Coord Y_Coord Solution_Value\n";

    // Write solution data
    for (size_t i = 0; i < std::min(mesh.nodes.size(), solution.size()); ++i) {
        file << i << " " 
             << mesh.nodes[i].first << " " 
             << mesh.nodes[i].second << " " 
             << solution[i] << "\n";
    }

    file.close();
}

void ReportGenerator::exportMeshData(
    const Mesh& mesh,
    const std::string& filename
) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }

    // Write header
    file << "# Mesh data\n";
    file << "# Nodes:\n";

    // Write nodes
    for (size_t i = 0; i < mesh.nodes.size(); ++i) {
        file << "N " << i << " " << mesh.nodes[i].first << " " << mesh.nodes[i].second << "\n";
    }

    file << "# Elements:\n";

    // Write elements
    for (size_t i = 0; i < mesh.elements.size(); ++i) {
        file << "E " << i << " " 
             << mesh.elements[i][0] << " " 
             << mesh.elements[i][1] << " " 
             << mesh.elements[i][2] << "\n";
    }

    file << "# Boundaries:\n";

    // Write boundary information
    for (const auto& boundary : mesh.boundaries) {
        file << "# Boundary: " << boundary.first << "\n";
        for (int nodeIdx : boundary.second) {
            file << "B " << boundary.first << " " << nodeIdx << "\n";
        }
    }

    file.close();
}

std::string ReportGenerator::generateSolutionStatistics(const std::vector<double>& solution) {
    if (solution.empty()) {
        return "No solution data available.\n";
    }

    double min_val = *std::min_element(solution.begin(), solution.end());
    double max_val = *std::max_element(solution.begin(), solution.end());
    double sum_val = std::accumulate(solution.begin(), solution.end(), 0.0);
    double mean_val = sum_val / solution.size();
    
    // Calculate standard deviation
    double sq_sum = std::inner_product(solution.begin(), solution.end(), solution.begin(), 0.0);
    double variance = sq_sum / solution.size() - mean_val * mean_val;
    double stddev_val = std::sqrt(variance);

    std::ostringstream oss;
    oss << "Number of nodes: " << solution.size() << "\n";
    oss << "Minimum value: " << std::fixed << std::setprecision(8) << min_val << "\n";
    oss << "Maximum value: " << std::fixed << std::setprecision(8) << max_val << "\n";
    oss << "Mean value: " << std::fixed << std::setprecision(8) << mean_val << "\n";
    oss << "Standard deviation: " << std::fixed << std::setprecision(8) << stddev_val << "\n";
    oss << "Range: " << std::fixed << std::setprecision(8) << (max_val - min_val) << "\n";

    return oss.str();
}

std::string ReportGenerator::generateMeshInformation(const Mesh& mesh) {
    std::ostringstream oss;
    oss << "Number of nodes: " << mesh.nodes.size() << "\n";
    oss << "Number of elements: " << mesh.elements.size() << "\n";
    oss << "Domain dimensions: X=[" << mesh.nodes.front().first << ", " 
        << mesh.nodes.back().first << "], Y=[" << mesh.nodes.front().second 
        << ", " << mesh.nodes.back().second << "]\n";

    // Boundary information
    oss << "Boundary nodes:\n";
    for (const auto& boundary : mesh.boundaries) {
        oss << "  " << boundary.first << ": " << boundary.second.size() << " nodes\n";
    }

    return oss.str();
}

std::string ReportGenerator::generateEquationDetails(const std::map<std::string, std::string>& coefficients) {
    std::ostringstream oss;
    oss << "General form: a11(x,y) * ∂²u/∂x² + 2*a12(x,y) * ∂²u/∂x∂y + a22(x,y) * ∂²u/∂y²\n";
    oss << "              + b1(x,y) * ∂u/∂x + b2(x,y) * ∂u/∂y + c(x,y) * u = f(x,y)\n\n";

    for (const auto& coeff : coefficients) {
        oss << coeff.first << "(x,y) = " << coeff.second << "\n";
    }

    return oss.str();
}

std::string ReportGenerator::generateBoundaryConditionsDetails(
    const std::map<std::string, BoundaryConditionData>& boundaryConditions
) {
    std::ostringstream oss;
    
    for (const auto& bc : boundaryConditions) {
        oss << bc.first << " boundary: ";
        if (bc.second.type == "dirichlet") {
            oss << "Dirichlet (u = g(x,y))";
        } else if (bc.second.type == "neumann") {
            oss << "Neumann (∂u/∂n = h(x,y))";
        } else {
            oss << "Unknown type";
        }
        
        oss << "\n";
    }

    return oss.str();
}

std::string ReportGenerator::generateProcessSteps() {
    return "1. Creation of finite element mesh\n"
           "2. Assembly of element stiffness matrices\n"
           "3. Assembly of global system matrix\n"
           "4. Application of boundary conditions\n"
           "5. Solution of linear system of equations\n"
           "6. Post-processing and visualization\n";
}