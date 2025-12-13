#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <array>

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Type definitions
using Node = std::pair<double, double>;
using Element = std::array<int, 3>; // Triangle element with 3 nodes
using CoefficientFunction = std::function<double(double, double)>;

// Boundary condition structure
struct BoundaryConditionData {
    std::string type; // "dirichlet" or "neumann"
    CoefficientFunction value_func;
    double value;
};

// Mesh structure
struct Mesh {
    std::vector<Node> nodes;
    std::vector<Element> elements;
    std::map<std::string, std::vector<int>> boundaries; // west, east, south, north
};

#endif // TYPES_H