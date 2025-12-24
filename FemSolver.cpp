// FemSolver.cpp: определяет точку входа для приложения.
//

#include "include/FemSolver.h"
#include <iostream>
#include <string>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

// Для Windows GUI приложений используем wWinMain как точку входа
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR lpCmdLine,
                     _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    try {
        FemSolver solver;
        // Run with GUI by default
        return solver.run(); // This will now use GUI by default
    } catch (const std::exception& e) {
        // For Windows GUI application, show error in message box
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, NULL, 0);
        std::wstring wideError(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, &wideError[0], size_needed);
        std::wstring wideMessage = L"Fatal error: " + wideError;
        MessageBoxW(NULL, wideMessage.c_str(), L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}
