#include "EllipticFEMSolver.h"
#include <cmath>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <numeric>

EllipticFEMSolver::EllipticFEMSolver(
    CoefficientFunction a11_func,
    CoefficientFunction a12_func,
    CoefficientFunction a22_func,
    CoefficientFunction b1_func,
    CoefficientFunction b2_func,
    CoefficientFunction c_func,
    CoefficientFunction f_func
) : a11_func_(a11_func ? a11_func : [](double, double) -> double { return 0.0; }),
    a12_func_(a12_func ? a12_func : [](double, double) -> double { return 0.0; }),
    a22_func_(a22_func ? a22_func : [](double, double) -> double { return 0.0; }),
    b1_func_(b1_func ? b1_func : [](double, double) -> double { return 0.0; }),
    b2_func_(b2_func ? b2_func : [](double, double) -> double { return 0.0; }),
    c_func_(c_func ? c_func : [](double, double) -> double { return 0.0; }),
    f_func_(f_func ? f_func : [](double, double) -> double { return 0.0; })
{
}

std::vector<double> EllipticFEMSolver::solve(const Mesh& mesh, const std::map<std::string, BoundaryConditionData>& boundaryConditions) {
    // Assemble global matrix and vector
    auto [K_global, F_global] = assembleGlobalMatrix(mesh);
    
    // Apply boundary conditions
    applyBoundaryConditions(K_global, F_global, mesh, boundaryConditions);
    
    // Solve the linear system (using a simple Gaussian elimination for now)
    // In a real implementation, you'd want to use a proper linear solver
    std::vector<double> solution = solveLinearSystem(K_global, F_global);
    
    return solution;
}

std::pair<std::vector<std::vector<double>>, std::vector<double>> 
EllipticFEMSolver::assembleGlobalMatrix(const Mesh& mesh) {
    int nNodes = static_cast<int>(mesh.nodes.size());
    
    // Initialize global matrices
    std::vector<std::vector<double>> K_global(nNodes, std::vector<double>(nNodes, 0.0));
    std::vector<double> F_global(nNodes, 0.0);
    
    // Assemble by elements
    for (const auto& element : mesh.elements) {
        // Get coordinates of the element nodes
        std::vector<Node> coords = {
            mesh.nodes[element[0]],
            mesh.nodes[element[1]],
            mesh.nodes[element[2]]
        };
        
        // Compute local matrices
        auto Ee = localEllipticMatrix(coords);
        auto Ce = localConvectionMatrix(coords);
        auto Re = localReactionMatrix(coords);
        auto Fe = localLoadVector(coords);
        
        // Assemble into global matrix
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                K_global[element[i]][element[j]] += Ee[i][j] + Ce[i][j] + Re[i][j];
            }
            F_global[element[i]] += Fe[i];
        }
    }
    
    return std::make_pair(K_global, F_global);
}

void EllipticFEMSolver::applyBoundaryConditions(
    std::vector<std::vector<double>>& K_global,
    std::vector<double>& F_global,
    const Mesh& mesh,
    const std::map<std::string, BoundaryConditionData>& boundaryConditions
) {
    for (const auto& pair : boundaryConditions) {
        const std::string& boundaryName = pair.first;
        const BoundaryConditionData& bcData = pair.second;

        auto boundaryIt = mesh.boundaries.find(boundaryName);
        if (boundaryIt == mesh.boundaries.end()) continue;

        const std::vector<int>& boundaryNodes = boundaryIt->second;

        if (bcData.type == "dirichlet") {
            for (int nodeIdx : boundaryNodes) {
                double x = mesh.nodes[nodeIdx].first;
                double y = mesh.nodes[nodeIdx].second;
                double g_val = bcData.value_func ? bcData.value_func(x, y) : bcData.value;

                // Set row and column to zero, diagonal to 1
                for (size_t j = 0; j < K_global[nodeIdx].size(); ++j) {
                    K_global[nodeIdx][j] = 0.0;
                    if (j < K_global.size()) {
                        K_global[j][nodeIdx] = 0.0; // Also set column to zero
                    }
                }
                K_global[nodeIdx][nodeIdx] = 1.0;

                // Set right-hand side
                F_global[nodeIdx] = g_val;
            }
        } else if (bcData.type == "neumann") {
            // For Neumann boundary conditions, add to the right-hand side
            for (int nodeIdx : boundaryNodes) {
                double x = mesh.nodes[nodeIdx].first;
                double y = mesh.nodes[nodeIdx].second;
                double h_val = bcData.value_func ? bcData.value_func(x, y) : bcData.value;

                // Add contribution to the right-hand side
                F_global[nodeIdx] += h_val;
            }
        }
    }
}

std::vector<std::vector<double>> EllipticFEMSolver::localEllipticMatrix(const std::vector<Node>& coords) {
    // Calculate element area
    double x1 = coords[0].first, y1 = coords[0].second;
    double x2 = coords[1].first, y2 = coords[1].second;
    double x3 = coords[2].first, y3 = coords[2].second;
    
    double area = 0.5 * std::abs((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1));
    
    // Calculate centroid for coefficient evaluation
    double x_c = (x1 + x2 + x3) / 3.0;
    double y_c = (y1 + y2 + y3) / 3.0;
    
    double a11_val = a11_func_(x_c, y_c);
    double a12_val = a12_func_(x_c, y_c) * 2.0; // Factor of 2 for the mixed term
    double a22_val = a22_func_(x_c, y_c);
    
    // Gradients of shape functions
    double detJ = 2.0 * area;
    double b1 = y2 - y3;
    double c1 = x3 - x2;
    double b2 = y3 - y1;
    double c2 = x1 - x3;
    double b3 = y1 - y2;
    double c3 = x2 - x1;
    
    std::vector<std::vector<double>> Be(3, std::vector<double>(3, 0.0));
    
    for (int i = 0; i < 3; ++i) {
        double dN_dx_i = (i == 0) ? b1 : (i == 1) ? b2 : b3;
        double dN_dy_i = (i == 0) ? c1 : (i == 1) ? c2 : c3;
        dN_dx_i /= detJ;
        dN_dy_i /= detJ;
        
        for (int j = 0; j < 3; ++j) {
            double dN_dx_j = (j == 0) ? b1 : (j == 1) ? b2 : b3;
            double dN_dy_j = (j == 0) ? c1 : (j == 1) ? c2 : c3;
            dN_dx_j /= detJ;
            dN_dy_j /= detJ;
            
            // Element stiffness matrix: integral of grad(Ni)^T * A * grad(Nj) * area
            Be[i][j] = area * (a11_val * dN_dx_i * dN_dx_j + 
                               a12_val * dN_dx_i * dN_dy_j + 
                               a12_val * dN_dy_i * dN_dx_j + 
                               a22_val * dN_dy_i * dN_dy_j);
        }
    }
    
    return Be;
}

std::vector<std::vector<double>> EllipticFEMSolver::localConvectionMatrix(const std::vector<Node>& coords) {
    // Calculate element area
    double x1 = coords[0].first, y1 = coords[0].second;
    double x2 = coords[1].first, y2 = coords[1].second;
    double x3 = coords[2].first, y3 = coords[2].second;
    
    double area = 0.5 * std::abs((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1));
    
    // Calculate centroid for coefficient evaluation
    double x_c = (x1 + x2 + x3) / 3.0;
    double y_c = (y1 + y2 + y3) / 3.0;
    
    double b1_val = b1_func_(x_c, y_c);
    double b2_val = b2_func_(x_c, y_c);
    
    // Gradients of shape functions
    double detJ = 2.0 * area;
    double b1 = y2 - y3;
    double c1 = x3 - x2;
    double b2 = y3 - y1;
    double c2 = x1 - x3;
    double b3 = y1 - y2;
    double c3 = x2 - x1;
    
    std::vector<std::vector<double>> Ce(3, std::vector<double>(3, 0.0));
    
    for (int i = 0; i < 3; ++i) {
        double dN_dx_i = (i == 0) ? b1 : (i == 1) ? b2 : b3;
        double dN_dy_i = (i == 0) ? c1 : (i == 1) ? c2 : c3;
        dN_dx_i /= detJ;
        dN_dy_i /= detJ;
        
        for (int j = 0; j < 3; ++j) {
            // Convection matrix: integral of (b1*dN_i/dx + b2*dN_i/dy) * N_j * area
            // For linear elements: N_j integrated over element = area/3 at each node
            double N_j_avg = 1.0/3.0; // Average value of N_j over the element
            Ce[i][j] = (b1_val * dN_dx_i + b2_val * dN_dy_i) * N_j_avg * area / 6.0;
        }
    }
    
    return Ce;
}

std::vector<std::vector<double>> EllipticFEMSolver::localReactionMatrix(const std::vector<Node>& coords) {
    // Calculate element area
    double x1 = coords[0].first, y1 = coords[0].second;
    double x2 = coords[1].first, y2 = coords[1].second;
    double x3 = coords[2].first, y3 = coords[2].second;
    
    double area = 0.5 * std::abs((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1));
    
    // Calculate centroid for coefficient evaluation
    double x_c = (x1 + x2 + x3) / 3.0;
    double y_c = (y1 + y2 + y3) / 3.0;
    
    double c_val = c_func_(x_c, y_c);
    
    std::vector<std::vector<double>> Re(3, std::vector<double>(3, 0.0));
    
    if (c_val != 0.0) {
        // Reaction matrix for linear elements
        // Integral of c*N_i*N_j over element = c*area/12*[2,1,1; 1,2,1; 1,1,2]
        double factor = c_val * area / 12.0;
        Re[0][0] = Re[1][1] = Re[2][2] = 2.0 * factor;
        Re[0][1] = Re[0][2] = Re[1][0] = Re[1][2] = Re[2][0] = Re[2][1] = factor;
    }
    
    return Re;
}

std::vector<double> EllipticFEMSolver::localLoadVector(const std::vector<Node>& coords) {
    // Calculate element area
    double x1 = coords[0].first, y1 = coords[0].second;
    double x2 = coords[1].first, y2 = coords[1].second;
    double x3 = coords[2].first, y3 = coords[2].second;
    
    double area = 0.5 * std::abs((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1));
    
    // Calculate centroid for coefficient evaluation
    double x_c = (x1 + x2 + x3) / 3.0;
    double y_c = (y1 + y2 + y3) / 3.0;
    
    double f_val = f_func_(x_c, y_c);
    
    // For linear elements: integral of f*N_i over element = f*area/3 for each i
    std::vector<double> Fe(3, f_val * area / 3.0);
    
    return Fe;
}

std::vector<double> EllipticFEMSolver::solveLinearSystem(
    const std::vector<std::vector<double>>& A, 
    const std::vector<double>& b
) {
    int n = static_cast<int>(b.size());
    std::vector<std::vector<double>> aug(A);
    
    // Augment the matrix with the right-hand side
    for (int i = 0; i < n; ++i) {
        aug[i].push_back(b[i]);
    }
    
    // Gaussian elimination with partial pivoting
    for (int i = 0; i < n; ++i) {
        // Find pivot
        int max_row = i;
        for (int k = i + 1; k < n; ++k) {
            if (std::abs(aug[k][i]) > std::abs(aug[max_row][i])) {
                max_row = k;
            }
        }
        
        // Swap rows
        std::swap(aug[i], aug[max_row]);
        
        // Check for singular matrix
        if (std::abs(aug[i][i]) < 1e-15) {
            throw std::runtime_error("Singular matrix in linear system solver");
        }
        
        // Eliminate column
        for (int k = i + 1; k < n; ++k) {
            double factor = aug[k][i] / aug[i][i];
            for (int j = i; j <= n; ++j) {
                aug[k][j] -= factor * aug[i][j];
            }
        }
    }
    
    // Back substitution
    std::vector<double> x(n);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = aug[i][n];
        for (int j = i + 1; j < n; ++j) {
            x[i] -= aug[i][j] * x[j];
        }
        x[i] /= aug[i][i];
    }
    
    return x;
}