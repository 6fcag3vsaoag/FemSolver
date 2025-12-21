#include "FemSolver.h"
#include "EllipticApp.h"
#include "GUIApp.h"
#include <iostream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

// Implementation of the FemSolver class

FemSolver::FemSolver() {
    try {
        // Initialize the main application
        app = std::make_unique<EllipticApp>();
    } catch (const std::exception& e) {
        #ifdef _DEBUG
        std::cerr << "Error initializing FemSolver: " << e.what() << std::endl;
        #else
        #ifdef _WIN32
        MessageBox(NULL, ("Error initializing FemSolver: " + std::string(e.what())).c_str(), "Error", MB_OK | MB_ICONERROR);
        #endif
        #endif
        throw;
    }
}

FemSolver::~FemSolver() {
    // Cleanup is handled by smart pointers
}

int FemSolver::run() {
    try {
        #ifdef _DEBUG
        // Only output to console in debug mode
        std::cout << "Starting FemSolver application..." << std::endl;
        #endif

        // Run the main application with GUI
        if (app) {
            // Set the solver in the GUI before running it
            app->setSolverForGUI(this);
            app->run(true);  // Use GUI by default
            return 0;
        } else {
            #ifdef _DEBUG
            std::cerr << "Error: Application not initialized" << std::endl;
            #endif
            return -1;
        }
    } catch (const std::exception& e) {
        #ifdef _DEBUG
        std::cerr << "Error running FemSolver: " << e.what() << std::endl;
        #else
        #ifdef _WIN32
        // For Windows GUI application, show error in message box
        MessageBox(NULL, ("Error running FemSolver: " + std::string(e.what())).c_str(), "Error", MB_OK | MB_ICONERROR);
        #endif
        #endif
        return -1;
    }
}