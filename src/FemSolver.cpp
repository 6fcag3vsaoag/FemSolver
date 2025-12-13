#include "FemSolver.h"
#include "EllipticApp.h"
#include <iostream>
#include <stdexcept>

// Implementation of the FemSolver class

FemSolver::FemSolver() {
    try {
        // Initialize the main application
        app = std::make_unique<EllipticApp>();
    } catch (const std::exception& e) {
        std::cerr << "Error initializing FemSolver: " << e.what() << std::endl;
        throw;
    }
}

FemSolver::~FemSolver() {
    // Cleanup is handled by smart pointers
}

int FemSolver::run() {
    try {
        std::cout << "Starting FemSolver application..." << std::endl;

        // Run the main application with GUI
        if (app) {
            app->run(true);  // Use GUI by default
            return 0;
        } else {
            std::cerr << "Error: Application not initialized" << std::endl;
            return -1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error running FemSolver: " << e.what() << std::endl;
        return -1;
    }
}