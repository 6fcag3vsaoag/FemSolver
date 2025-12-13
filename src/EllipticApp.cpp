#include "EllipticApp.h"
#include "GUIApp.h"
#include "MeshGenerator.h"
#include "FunctionParser.h"
#include "EllipticFEMSolver.h"
#include "Visualizer.h"
#include <iostream>
#include <memory>


EllipticApp::EllipticApp()
    : Lx_(1.0), Ly_(1.0), Nx_(10), Ny_(10),
      a11_func_(nullptr), a12_func_(nullptr), a22_func_(nullptr),
      b1_func_(nullptr), b2_func_(nullptr), c_func_(nullptr), f_func_(nullptr) {

    // Initialize components
    meshGenerator_ = std::make_unique<MeshGenerator>(Lx_, Ly_, Nx_, Ny_);
    femSolver_ = std::make_unique<EllipticFEMSolver>();
    visualizer_ = std::make_unique<Visualizer>();
    guiApp_ = std::make_unique<GUIApp>();

    std::cout << "EllipticApp initialized" << std::endl;
}

EllipticApp::~EllipticApp() = default;

void EllipticApp::run(bool useGUI) {
    if (useGUI) {
        try {
            // Initialize and run the GUI
            guiApp_->initialize();
            guiApp_->run();
        } catch (const std::exception& e) {
            std::cerr << "Error running GUI: " << e.what() << std::endl;
            std::cout << "Falling back to console mode..." << std::endl;
            runConsoleMode();
        }
    } else {
        runConsoleMode();
    }
}

void EllipticApp::runConsoleMode() {
    std::cout << "Starting Elliptic FEM Solver Application in console mode..." << std::endl;

    // Example: Set up a simple Poisson problem
    setupPoissonProblem();

    // Generate mesh
    generateMesh();

    // Solve the problem
    solveProblem();

    // Plot the solution
    plotSolution();

    std::cout << "Application completed successfully." << std::endl;
}

void EllipticApp::generateMesh() {
    try {
        std::cout << "Generating mesh with dimensions: " << Lx_ << " x " << Ly_
                  << " and " << Nx_ << " x " << Ny_ << " nodes" << std::endl;

        currentMesh_ = std::make_unique<Mesh>(meshGenerator_->generate());

        std::cout << "Mesh generated with " << currentMesh_->nodes.size()
                  << " nodes and " << currentMesh_->elements.size() << " elements" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error generating mesh: " << e.what() << std::endl;
        throw;
    }
}

void EllipticApp::solveProblem() {
    try {
        if (!currentMesh_) {
            throw std::runtime_error("No mesh available. Generate mesh first.");
        }

        std::cout << "Solving problem..." << std::endl;

        // Update the solver with current coefficient functions
        femSolver_ = std::make_unique<EllipticFEMSolver>(
            a11_func_, a12_func_, a22_func_,
            b1_func_, b2_func_, c_func_, f_func_
        );

        // Solve the problem
        currentSolution_ = femSolver_->solve(*currentMesh_, boundaryConditions_);

        std::cout << "Problem solved. Solution computed for "
                  << currentSolution_.size() << " nodes." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error solving problem: " << e.what() << std::endl;
        throw;
    }
}

void EllipticApp::plotSolution() {
    try {
        if (currentSolution_.empty()) {
            throw std::runtime_error("No solution available to plot.");
        }

        if (!currentMesh_) {
            throw std::runtime_error("No mesh available for plotting.");
        }

        std::cout << "Plotting solution..." << std::endl;

        // Plot the solution
        visualizer_->plotSolution(*currentMesh_, currentSolution_, "FEM Solution");

        // Export the plot data
        exportResults();
    } catch (const std::exception& e) {
        std::cerr << "Error plotting solution: " << e.what() << std::endl;
        throw;
    }
}

void EllipticApp::exportResults() {
    try {
        // Export the solution data
        visualizer_->exportPlot("solution_output.txt");

        // Generate detailed report
        std::map<std::string, std::string> coefficients;
        coefficients["a11"] = "1.0";  // This would be set from the UI
        coefficients["a12"] = "0.0";
        coefficients["a22"] = "1.0";
        coefficients["b1"] = "0.0";
        coefficients["b2"] = "0.0";
        coefficients["c"] = "0.0";
        coefficients["f"] = "1.0";

        if (currentMesh_) {
            visualizer_->generateReport(
                *currentMesh_,
                currentSolution_,
                coefficients,
                boundaryConditions_,
                "fem_detailed_report.txt"
            );
        }
    } catch (const std::exception& e) {
        std::cerr << "Error exporting results: " << e.what() << std::endl;
        throw;
    }
}

void EllipticApp::resetProblem() {
    currentMesh_.reset();
    currentSolution_.clear();

    // Reset to default values
    Lx_ = 1.0; Ly_ = 1.0;
    Nx_ = 10; Ny_ = 10;

    // Reset coefficient functions to default
    a11_func_ = [](double, double) -> double { return 1.0; };
    a12_func_ = [](double, double) -> double { return 0.0; };
    a22_func_ = [](double, double) -> double { return 1.0; };
    b1_func_ = [](double, double) -> double { return 0.0; };
    b2_func_ = [](double, double) -> double { return 0.0; };
    c_func_ = [](double, double) -> double { return 0.0; };
    f_func_ = [](double, double) -> double { return 1.0; };

    // Reset boundary conditions
    boundaryConditions_.clear();

    std::cout << "Problem reset to default values." << std::endl;
}

void EllipticApp::setCoefficientFunctions(
    CoefficientFunction a11, CoefficientFunction a12, CoefficientFunction a22,
    CoefficientFunction b1, CoefficientFunction b2, CoefficientFunction c, CoefficientFunction f
) {
    a11_func_ = std::move(a11);
    a12_func_ = std::move(a12);
    a22_func_ = std::move(a22);
    b1_func_ = std::move(b1);
    b2_func_ = std::move(b2);
    c_func_ = std::move(c);
    f_func_ = std::move(f);
}

void EllipticApp::setupPoissonProblem() {
    // Set up a simple Poisson problem: -Laplacian(u) = f
    // u = 0 on boundary
    Lx_ = 1.0;
    Ly_ = 1.0;
    Nx_ = 20;
    Ny_ = 20;

    // Coefficients for -Laplacian u = -(d²u/dx² + d²u/dy²) = f
    // This corresponds to: a11=1, a22=1, a12=b1=b2=c=0, f=function
    a11_func_ = [](double, double) -> double { return 1.0; };
    a12_func_ = [](double, double) -> double { return 0.0; };
    a22_func_ = [](double, double) -> double { return 1.0; };
    b1_func_ = [](double, double) -> double { return 0.0; };
    b2_func_ = [](double, double) -> double { return 0.0; };
    c_func_ = [](double, double) -> double { return 0.0; };
    f_func_ = [](double x, double y) -> double {
        // Example: f(x,y) = 2*pi*pi*sin(pi*x)*sin(pi*y) with solution u = sin(pi*x)*sin(pi*y)
        return 2.0 * M_PI * M_PI * std::sin(M_PI * x) * std::sin(M_PI * y);
    };

    // Set up Dirichlet boundary conditions (u = 0 on all boundaries)
    BoundaryConditionData zeroBC;
    zeroBC.type = "dirichlet";
    zeroBC.value_func = [](double, double) -> double { return 0.0; };
    zeroBC.value = 0.0;

    boundaryConditions_["west"] = zeroBC;
    boundaryConditions_["east"] = zeroBC;
    boundaryConditions_["south"] = zeroBC;
    boundaryConditions_["north"] = zeroBC;

    // Update the mesh generator with new dimensions
    meshGenerator_ = std::make_unique<MeshGenerator>(Lx_, Ly_, Nx_, Ny_);
}

void EllipticApp::setupLaplaceProblem() {
    // Set up Laplace problem: d²u/dx² + d²u/dy² = 0
    Lx_ = 1.0;
    Ly_ = 1.0;
    Nx_ = 15;
    Ny_ = 15;

    a11_func_ = [](double, double) -> double { return 1.0; };
    a12_func_ = [](double, double) -> double { return 0.0; };
    a22_func_ = [](double, double) -> double { return 1.0; };
    b1_func_ = [](double, double) -> double { return 0.0; };
    b2_func_ = [](double, double) -> double { return 0.0; };
    c_func_ = [](double, double) -> double { return 0.0; };
    f_func_ = [](double, double) -> double { return 0.0; };

    // Example boundary conditions: u = x^2 + y^2 on all boundaries
    BoundaryConditionData bc;
    bc.type = "dirichlet";
    bc.value_func = [](double x, double y) -> double { return x*x + y*y; };
    bc.value = 0.0;

    boundaryConditions_["west"] = bc;
    boundaryConditions_["east"] = bc;
    boundaryConditions_["south"] = bc;
    boundaryConditions_["north"] = bc;

    meshGenerator_ = std::make_unique<MeshGenerator>(Lx_, Ly_, Nx_, Ny_);
}

void EllipticApp::setupHelmholtzProblem() {
    // Set up Helmholtz problem: d²u/dx² + d²u/dy² + u = f
    Lx_ = 3.0;
    Ly_ = 1.0;
    Nx_ = 30;
    Ny_ = 10;

    a11_func_ = [](double, double) -> double { return 1.0; };
    a12_func_ = [](double, double) -> double { return 0.0; };
    a22_func_ = [](double, double) -> double { return 1.0; };
    b1_func_ = [](double, double) -> double { return 0.0; };
    b2_func_ = [](double, double) -> double { return 0.0; };
    c_func_ = [](double, double) -> double { return 1.0; };  // Coefficient for u term
    f_func_ = [](double x, double y) -> double {
        return std::cos(M_PI*x/3.0) * std::cos(M_PI*y);
    };

    // Mixed boundary conditions
    BoundaryConditionData neumannBC, dirichletBC;

    // West: Neumann boundary condition
    neumannBC.type = "neumann";
    neumannBC.value_func = [](double, double) -> double { return 0.0; };
    neumannBC.value = 0.0;
    boundaryConditions_["west"] = neumannBC;

    // Others: Dirichlet boundary conditions
    dirichletBC.type = "dirichlet";
    dirichletBC.value_func = [](double x, double y) -> double {
        return std::cos(M_PI*x/3.0) * std::cos(M_PI*y);
    };
    dirichletBC.value = 0.0;
    boundaryConditions_["east"] = dirichletBC;
    boundaryConditions_["south"] = dirichletBC;
    boundaryConditions_["north"] = dirichletBC;

    meshGenerator_ = std::make_unique<MeshGenerator>(Lx_, Ly_, Nx_, Ny_);
}

void EllipticApp::setupConvectionDiffusionProblem() {
    // Set up convection-diffusion problem
    Lx_ = 2.0;
    Ly_ = 1.0;
    Nx_ = 40;
    Ny_ = 20;

    // Variable diffusion coefficients
    a11_func_ = [](double x, double) -> double { return 0.01 + 0.005*x; };
    a12_func_ = [](double, double) -> double { return 0.0; };
    a22_func_ = [](double x, double) -> double { return 0.01 + 0.005*x; };
    b1_func_ = [](double, double) -> double { return 1.0; };  // Convection in x direction
    b2_func_ = [](double, double) -> double { return 0.0; };
    c_func_ = [](double, double) -> double { return 0.0; };
    f_func_ = [](double x, double y) -> double {
        return std::exp(-10.0 * ((x-2.0)*(x-2.0) + (y-0.5)*(y-0.5)));
    };

    // Boundary conditions
    BoundaryConditionData dirichletBC, neumannBC;

    // West: Dirichlet u = 1 (inlet)
    dirichletBC.type = "dirichlet";
    dirichletBC.value = 1.0;
    dirichletBC.value_func = nullptr;
    boundaryConditions_["west"] = dirichletBC;

    // East: Dirichlet u = 0 (outlet)
    dirichletBC.value = 0.0;
    boundaryConditions_["east"] = dirichletBC;

    // South and North: Neumann (symmetry)
    neumannBC.type = "neumann";
    neumannBC.value = 0.0;
    neumannBC.value_func = nullptr;
    boundaryConditions_["south"] = neumannBC;
    boundaryConditions_["north"] = neumannBC;

    meshGenerator_ = std::make_unique<MeshGenerator>(Lx_, Ly_, Nx_, Ny_);
}

void EllipticApp::setupReactionDiffusionProblem() {
    // Set up reaction-diffusion problem
    Lx_ = 2.0;
    Ly_ = 2.0;
    Nx_ = 30;
    Ny_ = 30;

    // Variable diffusion coefficients
    a11_func_ = [](double x, double y) -> double { return 0.1 + 0.05*x*y; };
    a12_func_ = [](double, double) -> double { return 0.0; };
    a22_func_ = [](double x, double y) -> double { return 0.1 + 0.05*x*y; };
    b1_func_ = [](double, double) -> double { return 0.0; };
    b2_func_ = [](double, double) -> double { return 0.0; };
    c_func_ = [](double, double) -> double { return 1.0; };  // Reaction term
    f_func_ = [](double x, double y) -> double {
        return 10.0 * std::exp(-5.0 * ((x-1.0)*(x-1.0) + (y-1.0)*(y-1.0))) +
               2.0 * M_PI * M_PI * std::cos(M_PI*x) * std::cos(M_PI*y);
    };

    // Mixed boundary conditions
    BoundaryConditionData dirichletBC, neumannBC;

    dirichletBC.type = "dirichlet";
    neumannBC.type = "neumann";

    dirichletBC.value_func = [](double, double) -> double { return 20.0; };
    dirichletBC.value = 20.0;
    boundaryConditions_["west"] = dirichletBC;

    dirichletBC.value_func = [](double, double) -> double { return 10.0; };
    dirichletBC.value = 10.0;
    boundaryConditions_["east"] = dirichletBC;

    dirichletBC.value_func = [](double, double) -> double { return 15.0; };
    dirichletBC.value = 15.0;
    boundaryConditions_["south"] = dirichletBC;

    neumannBC.value_func = [](double, double) -> double { return 5.0; };
    neumannBC.value = 5.0;
    boundaryConditions_["north"] = neumannBC;

    meshGenerator_ = std::make_unique<MeshGenerator>(Lx_, Ly_, Nx_, Ny_);
}