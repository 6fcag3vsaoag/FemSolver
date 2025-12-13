// FemSolver.cpp: определяет точку входа для приложения.
//

#include "include/FemSolver.h"

int main()
{
    try {
        FemSolver solver;
        // Run with GUI by default
        return solver.run(); // This will now use GUI by default
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    }
}
