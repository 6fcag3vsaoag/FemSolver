#ifndef ELLIPTICFEMSOLVER_H
#define ELLIPTICFEMSOLVER_H

#include "Types.h"
#include <vector>
#include <map>
#include <memory>

class EllipticFEMSolver {
public:
    EllipticFEMSolver(
        CoefficientFunction a11_func = nullptr,
        CoefficientFunction a12_func = nullptr,
        CoefficientFunction a22_func = nullptr,
        CoefficientFunction b1_func = nullptr,
        CoefficientFunction b2_func = nullptr,
        CoefficientFunction c_func = nullptr,
        CoefficientFunction f_func = nullptr
    );
    ~EllipticFEMSolver() = default;

    // Solve the elliptic equation with given mesh and boundary conditions
    std::vector<double> solve(const Mesh& mesh, const std::map<std::string, BoundaryConditionData>& boundaryConditions);

    // Assemble global matrix and vector
    std::pair<std::vector<std::vector<double>>, std::vector<double>>
    assembleGlobalMatrix(const Mesh& mesh);

    // Apply boundary conditions to the global system
    void applyBoundaryConditions(
        std::vector<std::vector<double>>& K_global,
        std::vector<double>& F_global,
        const Mesh& mesh,
        const std::map<std::string, BoundaryConditionData>& boundaryConditions
    );

private:
    // Local element matrices
    std::vector<std::vector<double>> localEllipticMatrix(const std::vector<Node>& coords);
    std::vector<std::vector<double>> localConvectionMatrix(const std::vector<Node>& coords);
    std::vector<std::vector<double>> localReactionMatrix(const std::vector<Node>& coords);
    std::vector<double> localLoadVector(const std::vector<Node>& coords);

    // Coefficient functions
    CoefficientFunction a11_func_, a12_func_, a22_func_;
    CoefficientFunction b1_func_, b2_func_;
    CoefficientFunction c_func_, f_func_;

    // Helper function for solving linear systems
    std::vector<double> solveLinearSystem(
        const std::vector<std::vector<double>>& A,
        const std::vector<double>& b
    );
};

#endif // ELLIPTICFEMSOLVER_H