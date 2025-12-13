#ifndef FEMSOLVER_H
#define FEMSOLVER_H

#include "Types.h"
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <numeric>

// Forward declarations
class EllipticApp;

// Main FemSolver class
class FemSolver {
public:
    FemSolver();
    ~FemSolver();  // Defined in implementation to handle unique_ptr to forward-declared type

    // Entry point for the application
    int run();

    // Public interface for GUI
    class EllipticApp* getApp() { return app.get(); }

private:
    std::unique_ptr<EllipticApp> app;
};

#endif // FEMSOLVER_H