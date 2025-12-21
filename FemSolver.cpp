// FemSolver.cpp: определяет точку входа для приложения.
//

#include "include/FemSolver.h"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

int main()
{
    try {
        FemSolver solver;
        // Run with GUI by default
        return solver.run(); // This will now use GUI by default
    } catch (const std::exception& e) {
        // In GUI mode, we might want to show a message box instead of console output
        #ifdef _WIN32
        // For Windows GUI application, show error in message box
        MessageBox(NULL, ("Fatal error: " + std::string(e.what())).c_str(), "Error", MB_OK | MB_ICONERROR);
        #else
        std::cerr << "Fatal error: " << e.what() << std::endl;
        #endif
        return -1;
    }
}
