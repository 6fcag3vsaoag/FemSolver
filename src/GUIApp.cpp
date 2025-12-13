#include "GUIApp.h"
#include "FemSolver.h"
#include <iostream>
#include <stdexcept>
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <sstream>

// Define window class name
const char g_szClassName[] = "FemSolverWindowClass";

// Structure to hold application data
struct AppData {
    double Lx = 1.0;
    double Ly = 1.0;
    int Nx = 10;
    int Ny = 10;
    std::string a11Func = "1.0";
    std::string a12Func = "0.0";
    std::string a22Func = "1.0";
    std::string b1Func = "0.0";
    std::string b2Func = "0.0";
    std::string cFunc = "0.0";
    std::string fFunc = "1.0";

    // Controls handles
    HWND hLxEdit, hLyEdit, hNxEdit, hNyEdit;
    HWND hA11Edit, hA12Edit, hA22Edit;
    HWND hB1Edit, hB2Edit, hCEdit, hFEdit;
    HWND hWestValue, hEastValue, hSouthValue, hNorthValue;
    HWND hVisualFrame;
    HWND hStatus;

    // Solver instance
    FemSolver* solver;
};

AppData g_appData;

// Function declarations
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateControls(HWND hwnd);
void OnSolveButtonClicked(HWND hwnd);
void OnResetButtonClicked(HWND hwnd);
void OnExportButtonClicked(HWND hwnd);
void OnPresetChanged(HWND hwnd, int presetIndex);
void LoadPreset(int presetIndex);

GUIApp::GUIApp() : mainWindow(nullptr), coreSolver(nullptr) {
    // Initialize common controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);
}

GUIApp::~GUIApp() {
    // Cleanup if needed
}

void GUIApp::initialize() {
    // Register window class
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        throw std::runtime_error("Window Registration Failed!");
    }
}

void GUIApp::run() {
    // Create the main window with better size and positioning
    HWND hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "Finite Element Method Solver - Elliptic Equations",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1400, 900,  // Increased size for better layout
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (hwnd == NULL) {
        throw std::runtime_error("Window Creation Failed!");
    }

    // Center the window
    RECT rect;
    GetWindowRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            CreateControls(hwnd);
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case 1001: // Solve button
                    OnSolveButtonClicked(hwnd);
                    break;
                case 1002: // Reset button
                    OnResetButtonClicked(hwnd);
                    break;
                case 1003: // Export button
                    OnExportButtonClicked(hwnd);
                    break;
                case 1004: // Help button
                    MessageBox(hwnd,
                        "FEM Solver Help:\n\n"
                        "1. Select an equation preset from the dropdown\n"
                        "2. Adjust mesh parameters (Lx, Ly, Nx, Ny)\n"
                        "3. Modify equation coefficients if needed\n"
                        "4. Set boundary conditions for each side\n"
                        "5. Click 'Solve' to compute the solution\n"
                        "6. Use 'Export' to save results\n\n"
                        "Coefficients:\n"
                        "a11, a22: diffusion coefficients\n"
                        "a12: mixed derivative coefficient\n"
                        "b1, b2: convection coefficients\n"
                        "c: reaction coefficient\n"
                        "f: source term",
                        "Help", MB_OK | MB_ICONINFORMATION);
                    break;
                case 2000: // Preset combo box
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        int selection = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                        // Load preset based on selection
                        OnPresetChanged(hwnd, selection);
                    }
                    break;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void CreateControls(HWND hwnd) {
    // Create a combo box for equation presets
    CreateWindow("Static", "Equation Preset:", WS_VISIBLE | WS_CHILD, 20, 10, 100, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hPresetCombo = CreateWindow("ComboBox", "",
                                     WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                     130, 10, 300, 150, hwnd, (HMENU)2000, GetModuleHandle(NULL), NULL);

    // Add preset options
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"Select Preset...");
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"Laplace Equation");
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"Poisson Equation");
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"Helmholtz Equation");
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"Convection-Diffusion");
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"Reaction-Diffusion");
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"General Elliptic");
    SendMessage(hPresetCombo, CB_SETCURSEL, 0, 0); // Set default selection

    // Create group box for mesh parameters
    HWND hMeshGroup = CreateWindow("Button", "Mesh Parameters",
                                   WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                   20, 45, 200, 140, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "Lx:", WS_VISIBLE | WS_CHILD, 40, 70, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hLxEdit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 80, 68, 70, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "Ly:", WS_VISIBLE | WS_CHILD, 40, 100, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hLyEdit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 80, 98, 70, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "Nx:", WS_VISIBLE | WS_CHILD, 160, 70, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNxEdit = CreateWindow("Edit", "20", WS_VISIBLE | WS_CHILD | WS_BORDER, 185, 68, 70, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "Ny:", WS_VISIBLE | WS_CHILD, 160, 100, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNyEdit = CreateWindow("Edit", "20", WS_VISIBLE | WS_CHILD | WS_BORDER, 185, 98, 70, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Create group box for equation coefficients
    HWND hCoeffGroup = CreateWindow("Button", "Equation Coefficients",
                                    WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                    250, 45, 450, 200, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "a11(x,y):", WS_VISIBLE | WS_CHILD, 270, 70, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA11Edit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 340, 68, 120, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "a12(x,y):", WS_VISIBLE | WS_CHILD, 480, 70, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA12Edit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 550, 68, 120, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "a22(x,y):", WS_VISIBLE | WS_CHILD, 270, 100, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA22Edit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 340, 98, 120, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "b1(x,y):", WS_VISIBLE | WS_CHILD, 480, 100, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hB1Edit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 550, 98, 120, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "b2(x,y):", WS_VISIBLE | WS_CHILD, 270, 130, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hB2Edit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 340, 128, 120, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "c(x,y):", WS_VISIBLE | WS_CHILD, 480, 130, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hCEdit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 550, 128, 120, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "f(x,y):", WS_VISIBLE | WS_CHILD, 270, 160, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hFEdit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 340, 158, 330, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Create group box for boundary conditions
    HWND hBCGroup = CreateWindow("Button", "Boundary Conditions",
                                 WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                 20, 195, 680, 120, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "West:", WS_VISIBLE | WS_CHILD, 40, 220, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hWestBC = CreateWindow("ComboBox", "",
                                WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                80, 218, 100, 60, hwnd, (HMENU)2001, GetModuleHandle(NULL), NULL);
    SendMessage(hWestBC, CB_ADDSTRING, 0, (LPARAM)"Dirichlet");
    SendMessage(hWestBC, CB_ADDSTRING, 0, (LPARAM)"Neumann");
    SendMessage(hWestBC, CB_SETCURSEL, 0, 0);

    g_appData.hWestValue = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 190, 218, 80, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "East:", WS_VISIBLE | WS_CHILD, 290, 220, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hEastBC = CreateWindow("ComboBox", "",
                                WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                330, 218, 100, 60, hwnd, (HMENU)2002, GetModuleHandle(NULL), NULL);
    SendMessage(hEastBC, CB_ADDSTRING, 0, (LPARAM)"Dirichlet");
    SendMessage(hEastBC, CB_ADDSTRING, 0, (LPARAM)"Neumann");
    SendMessage(hEastBC, CB_SETCURSEL, 0, 0);

    g_appData.hEastValue = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 440, 218, 80, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "South:", WS_VISIBLE | WS_CHILD, 40, 255, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hSouthBC = CreateWindow("ComboBox", "",
                                 WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                 80, 253, 100, 60, hwnd, (HMENU)2003, GetModuleHandle(NULL), NULL);
    SendMessage(hSouthBC, CB_ADDSTRING, 0, (LPARAM)"Dirichlet");
    SendMessage(hSouthBC, CB_ADDSTRING, 0, (LPARAM)"Neumann");
    SendMessage(hSouthBC, CB_SETCURSEL, 0, 0);

    g_appData.hSouthValue = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 190, 253, 80, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "North:", WS_VISIBLE | WS_CHILD, 290, 255, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hNorthBC = CreateWindow("ComboBox", "",
                                 WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                 330, 253, 100, 60, hwnd, (HMENU)2004, GetModuleHandle(NULL), NULL);
    SendMessage(hNorthBC, CB_ADDSTRING, 0, (LPARAM)"Dirichlet");
    SendMessage(hNorthBC, CB_ADDSTRING, 0, (LPARAM)"Neumann");
    SendMessage(hNorthBC, CB_SETCURSEL, 0, 0);

    g_appData.hNorthValue = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 440, 253, 80, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Create buttons
    HWND hSolveBtn = CreateWindow("Button", "Solve", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                  20, 325, 100, 30, hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);
    HWND hResetBtn = CreateWindow("Button", "Reset", WS_VISIBLE | WS_CHILD,
                                  130, 325, 100, 30, hwnd, (HMENU)1002, GetModuleHandle(NULL), NULL);
    HWND hExportBtn = CreateWindow("Button", "Export", WS_VISIBLE | WS_CHILD,
                                   240, 325, 100, 30, hwnd, (HMENU)1003, GetModuleHandle(NULL), NULL);
    HWND hHelpBtn = CreateWindow("Button", "Help", WS_VISIBLE | WS_CHILD,
                                 350, 325, 100, 30, hwnd, (HMENU)1004, GetModuleHandle(NULL), NULL);

    // Create visualization area
    g_appData.hVisualFrame = CreateWindow("Static", "Solution Visualization",
                                         WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTERIMAGE | SS_CENTER,
                                         20, 370, 680, 450, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Create status bar
    g_appData.hStatus = CreateWindow("Static", "Ready - Select an equation preset to begin",
                                    WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTERIMAGE | SS_CENTER,
                                    20, 830, 1360, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
}

void OnSolveButtonClicked(HWND hwnd) {
    // Get values from the UI
    char buffer[256];

    GetWindowText(g_appData.hLxEdit, buffer, sizeof(buffer));
    g_appData.Lx = atof(buffer);

    GetWindowText(g_appData.hLyEdit, buffer, sizeof(buffer));
    g_appData.Ly = atof(buffer);

    GetWindowText(g_appData.hNxEdit, buffer, sizeof(buffer));
    g_appData.Nx = atoi(buffer);

    GetWindowText(g_appData.hNyEdit, buffer, sizeof(buffer));
    g_appData.Ny = atoi(buffer);

    // Get coefficient functions
    GetWindowText(g_appData.hA11Edit, buffer, sizeof(buffer));
    g_appData.a11Func = std::string(buffer);

    GetWindowText(g_appData.hA12Edit, buffer, sizeof(buffer));
    g_appData.a12Func = std::string(buffer);

    GetWindowText(g_appData.hA22Edit, buffer, sizeof(buffer));
    g_appData.a22Func = std::string(buffer);

    GetWindowText(g_appData.hB1Edit, buffer, sizeof(buffer));
    g_appData.b1Func = std::string(buffer);

    GetWindowText(g_appData.hB2Edit, buffer, sizeof(buffer));
    g_appData.b2Func = std::string(buffer);

    GetWindowText(g_appData.hCEdit, buffer, sizeof(buffer));
    g_appData.cFunc = std::string(buffer);

    GetWindowText(g_appData.hFEdit, buffer, sizeof(buffer));
    g_appData.fFunc = std::string(buffer);

    // Update status
    SetWindowText(g_appData.hStatus, "Solving...");

    try {
        // If we have a solver instance, use it
        if (g_appData.solver) {
            // This is where we would connect to the actual solver
            // For now, we'll create a temporary message
            std::ostringstream oss;
            oss << "Solved: Lx=" << g_appData.Lx << ", Ly=" << g_appData.Ly
                << ", Nx=" << g_appData.Nx << ", Ny=" << g_appData.Ny
                << ", Eq: -div(A*grad(u)) + b*grad(u) + c*u = f";

            SetWindowText(g_appData.hStatus, oss.str().c_str());

            // In a real implementation, we would call:
            // g_appData.solver->solveWithParameters(...);
        } else {
            MessageBox(hwnd, "Solver not initialized. Please run from main application.",
                      "Error", MB_OK | MB_ICONERROR);
            SetWindowText(g_appData.hStatus, "Solver not initialized");
        }
    } catch (const std::exception& e) {
        std::ostringstream oss;
        oss << "Error: " << e.what();
        SetWindowText(g_appData.hStatus, oss.str().c_str());
    }
}

void OnResetButtonClicked(HWND hwnd) {
    // Reset to default values
    SetWindowText(g_appData.hLxEdit, "1.0");
    SetWindowText(g_appData.hLyEdit, "1.0");
    SetWindowText(g_appData.hNxEdit, "10");
    SetWindowText(g_appData.hNyEdit, "10");
    SetWindowText(g_appData.hA11Edit, "1.0");
    SetWindowText(g_appData.hA12Edit, "0.0");
    SetWindowText(g_appData.hA22Edit, "1.0");
    SetWindowText(g_appData.hB1Edit, "0.0");
    SetWindowText(g_appData.hB2Edit, "0.0");
    SetWindowText(g_appData.hCEdit, "0.0");
    SetWindowText(g_appData.hFEdit, "1.0");
    
    SetWindowText(g_appData.hStatus, "Reset to defaults");
}

void OnExportButtonClicked(HWND hwnd) {
    SetWindowText(g_appData.hStatus, "Exporting results...");
    // In a real app, you'd export the results here
}

void OnPresetChanged(HWND hwnd, int presetIndex) {
    if (presetIndex > 0) { // Skip the "Select Preset..." option
        LoadPreset(presetIndex);
        SetWindowText(g_appData.hStatus, "Preset loaded. Ready to solve.");
    }
}

void LoadPreset(int presetIndex) {
    // Set default values based on the preset
    switch(presetIndex) {
        case 1: // Laplace Equation
            SetWindowText(g_appData.hLxEdit, "1.0");
            SetWindowText(g_appData.hLyEdit, "1.0");
            SetWindowText(g_appData.hNxEdit, "20");
            SetWindowText(g_appData.hNyEdit, "20");
            SetWindowText(g_appData.hA11Edit, "1.0");
            SetWindowText(g_appData.hA12Edit, "0.0");
            SetWindowText(g_appData.hA22Edit, "1.0");
            SetWindowText(g_appData.hB1Edit, "0.0");
            SetWindowText(g_appData.hB2Edit, "0.0");
            SetWindowText(g_appData.hCEdit, "0.0");
            SetWindowText(g_appData.hFEdit, "0.0");
            SetWindowText(g_appData.hWestValue, "x*x + y*y");
            SetWindowText(g_appData.hEastValue, "x*x + y*y");
            SetWindowText(g_appData.hSouthValue, "x*x + y*y");
            SetWindowText(g_appData.hNorthValue, "x*x + y*y");
            MessageBox(NULL,
                "LAPLACE EQUATION\n\n"
                "Solves: ∂²u/∂x² + ∂²u/∂y² = 0\n\n"
                "Description:\n"
                "Classic Laplace equation describing steady-state processes:\n"
                "- Heat distribution without sources\n"
                "- Electrostatic potential in vacuum\n"
                "- Potential flow of incompressible fluid\n\n"
                "Coefficients:\n"
                "- a11 = a22 = 1 (unit diffusion)\n"
                "- a12 = b1 = b2 = c = f = 0 (no mixed derivatives, convection, reaction or sources)\n\n"
                "Boundary Conditions:\n"
                "All sides: u = x² + y²",
                "Laplace Equation", MB_OK | MB_ICONINFORMATION);
            break;

        case 2: // Poisson Equation
            SetWindowText(g_appData.hLxEdit, "1.0");
            SetWindowText(g_appData.hLyEdit, "1.0");
            SetWindowText(g_appData.hNxEdit, "20");
            SetWindowText(g_appData.hNyEdit, "20");
            SetWindowText(g_appData.hA11Edit, "1.0");
            SetWindowText(g_appData.hA12Edit, "0.0");
            SetWindowText(g_appData.hA22Edit, "1.0");
            SetWindowText(g_appData.hB1Edit, "0.0");
            SetWindowText(g_appData.hB2Edit, "0.0");
            SetWindowText(g_appData.hCEdit, "0.0");
            SetWindowText(g_appData.hFEdit, "8*pi*pi*sin(2*pi*x)*sin(2*pi*y)");
            SetWindowText(g_appData.hWestValue, "0.0");
            SetWindowText(g_appData.hEastValue, "0.0");
            SetWindowText(g_appData.hSouthValue, "0.0");
            SetWindowText(g_appData.hNorthValue, "0.0");
            MessageBox(NULL,
                "POISSON EQUATION\n\n"
                "Solves: ∂²u/∂x² + ∂²u/∂y² = 8π²·sin(2πx)·sin(2πy)\n\n"
                "Description:\n"
                "Poisson equation with known analytical solution:\n"
                "- Heat distribution with sources\n"
                "- Electrostatic potential with charges\n"
                "- Stationary processes with sources/sinks\n\n"
                "Coefficients:\n"
                "- a11 = a22 = 1 (unit diffusion)\n"
                "- a12 = b1 = b2 = c = 0 (no mixed derivatives, convection, reaction)\n"
                "- f(x,y) = 8π²·sin(2πx)·sin(2πy) (harmonic source)\n\n"
                "Analytical Solution:\n"
                "u(x,y) = sin(2πx)·sin(2πy)\n\n"
                "Boundary Conditions:\n"
                "Zero Dirichlet conditions on all boundaries (u = 0)",
                "Poisson Equation", MB_OK | MB_ICONINFORMATION);
            break;

        case 3: // Helmholtz Equation
            SetWindowText(g_appData.hLxEdit, "3.0");
            SetWindowText(g_appData.hLyEdit, "1.0");
            SetWindowText(g_appData.hNxEdit, "30");
            SetWindowText(g_appData.hNyEdit, "10");
            SetWindowText(g_appData.hA11Edit, "1.0");
            SetWindowText(g_appData.hA12Edit, "0.0");
            SetWindowText(g_appData.hA22Edit, "1.0");
            SetWindowText(g_appData.hB1Edit, "0.0");
            SetWindowText(g_appData.hB2Edit, "0.0");
            SetWindowText(g_appData.hCEdit, "1.0");
            SetWindowText(g_appData.hFEdit, "cos(pi*x/3)*cos(pi*y)");
            SetWindowText(g_appData.hWestValue, "0.0"); // Neumann for west
            SetWindowText(g_appData.hEastValue, "cos(pi*3/3)*cos(pi*y)"); // cos(pi)*cos(pi*y) = -cos(pi*y)
            SetWindowText(g_appData.hSouthValue, "cos(pi*x/3)*cos(0)"); // cos(pi*x/3)
            SetWindowText(g_appData.hNorthValue, "cos(pi*x/3)*cos(pi*1)"); // cos(pi*x/3)*cos(pi) = -cos(pi*x/3)
            MessageBox(NULL,
                "HELMHOLTZ EQUATION\n\n"
                "Solves: ∂²u/∂x² + ∂²u/∂y² + u = cos(πx/3)·cos(πy)\n\n"
                "Description:\n"
                "Helmholtz equation - important elliptic equation:\n"
                "- Wave processes in frequency domain\n"
                "- Quantum mechanics (stationary Schrödinger equation)\n"
                "- Acoustics and electromagnetic waves\n\n"
                "Coefficients:\n"
                "- a11 = a22 = 1 (diffusion)\n"
                "- c = 1 (reaction term)\n"
                "- a12 = b1 = b2 = 0\n"
                "- f(x,y) = cos(πx/3)·cos(πy) (source)\n\n"
                "Boundary Conditions:\n"
                "- West: Neumann (∂u/∂n = 0) - symmetry\n"
                "- Others: Dirichlet with specified values",
                "Helmholtz Equation", MB_OK | MB_ICONINFORMATION);
            break;

        case 4: // Convection-Diffusion
            SetWindowText(g_appData.hLxEdit, "2.0");
            SetWindowText(g_appData.hLyEdit, "1.0");
            SetWindowText(g_appData.hNxEdit, "40");
            SetWindowText(g_appData.hNyEdit, "20");
            SetWindowText(g_appData.hA11Edit, "0.01 + 0.005*x");
            SetWindowText(g_appData.hA12Edit, "0.0");
            SetWindowText(g_appData.hA22Edit, "0.01 + 0.005*x");
            SetWindowText(g_appData.hB1Edit, "1.0");
            SetWindowText(g_appData.hB2Edit, "0.0");
            SetWindowText(g_appData.hCEdit, "0.0");
            SetWindowText(g_appData.hFEdit, "exp(-10*((x-2)^2 + (y-0.5)^2))");
            SetWindowText(g_appData.hWestValue, "1.0"); // Inlet
            SetWindowText(g_appData.hEastValue, "0.0"); // Outlet
            SetWindowText(g_appData.hSouthValue, "0.0"); // Symmetry
            SetWindowText(g_appData.hNorthValue, "0.0"); // Symmetry
            MessageBox(NULL,
                "CONVECTION-DIFFUSION\n\n"
                "Solves: (0.01+0.005x)∂²u/∂x² + (0.01+0.005x)∂²u/∂y² + ∂u/∂x = exp(-10·((x-2)²+(y-0.5)²))\n\n"
                "Description:\n"
                "Convection-diffusion equation describing transport:\n"
                "- Pollutant transport in atmosphere or water\n"
                "- Heat transfer with fluid motion\n"
                "- Diffusion in flowing media\n\n"
                "Coefficients:\n"
                "- a11 = a22 = 0.01 + 0.005x (variable diffusion)\n"
                "- b1 = 1 (convection in positive X direction)\n"
                "- b2 = 0 (no convection in Y)\n"
                "- a12 = c = 0\n"
                "- f(x,y) = exp(-10·((x-2)²+(y-0.5)²)) (local source)\n\n"
                "Boundary Conditions:\n"
                "- West: Dirichlet u = 1 (inlet concentration)\n"
                "- East: Dirichlet u = 0 (outlet concentration)\n"
                "- South/North: Neumann (symmetry)",
                "Convection-Diffusion", MB_OK | MB_ICONINFORMATION);
            break;

        case 5: // Reaction-Diffusion
            SetWindowText(g_appData.hLxEdit, "2.0");
            SetWindowText(g_appData.hLyEdit, "2.0");
            SetWindowText(g_appData.hNxEdit, "30");
            SetWindowText(g_appData.hNyEdit, "30");
            SetWindowText(g_appData.hA11Edit, "0.1 + 0.05*x*y");
            SetWindowText(g_appData.hA12Edit, "0.0");
            SetWindowText(g_appData.hA22Edit, "0.1 + 0.05*x*y");
            SetWindowText(g_appData.hB1Edit, "0.0");
            SetWindowText(g_appData.hB2Edit, "0.0");
            SetWindowText(g_appData.hCEdit, "1.0");
            SetWindowText(g_appData.hFEdit, "10*exp(-5*((x-1)^2 + (y-1)^2)) + 2*pi^2*cos(pi*x)*cos(pi*y)");
            SetWindowText(g_appData.hWestValue, "20.0");
            SetWindowText(g_appData.hEastValue, "10.0");
            SetWindowText(g_appData.hSouthValue, "15.0");
            SetWindowText(g_appData.hNorthValue, "5.0"); // Neumann
            MessageBox(NULL,
                "REACTION-DIFFUSION\n\n"
                "Solves: (0.1+0.05xy)∂²u/∂x² + (0.1+0.05xy)∂²u/∂y² + u = 10·exp(-5·((x-1)²+(y-1)²)) + 2π²·cos(πx)·cos(πy)\n\n"
                "Description:\n"
                "Reaction-diffusion equations modeling many processes:\n"
                "- Population dynamics of species\n"
                "- Chemical reactions with diffusion\n"
                "- Neural impulse propagation\n\n"
                "Coefficients:\n"
                "- a11 = a22 = 0.1 + 0.05xy (variable diffusion)\n"
                "- c = 1 (linear reaction)\n"
                "- a12 = b1 = b2 = 0\n"
                "- f(x,y) = complex source (exponential + harmonic)\n\n"
                "Boundary Conditions:\n"
                "Mixed conditions:\n"
                "- Some: Dirichlet with different values\n"
                "- North: Neumann ∂u/∂n = 5",
                "Reaction-Diffusion", MB_OK | MB_ICONINFORMATION);
            break;

        case 6: // General Elliptic
            SetWindowText(g_appData.hLxEdit, "2.0");
            SetWindowText(g_appData.hLyEdit, "1.0");
            SetWindowText(g_appData.hNxEdit, "50");
            SetWindowText(g_appData.hNyEdit, "25");
            SetWindowText(g_appData.hA11Edit, "1 + 0.5*sin(pi*x)*cos(pi*y)");
            SetWindowText(g_appData.hA12Edit, "0.2");
            SetWindowText(g_appData.hA22Edit, "0.8 + 0.3*cos(pi*x)");
            SetWindowText(g_appData.hB1Edit, "0.2*x");
            SetWindowText(g_appData.hB2Edit, "0.1*y");
            SetWindowText(g_appData.hCEdit, "0.1");
            SetWindowText(g_appData.hFEdit, "sin(pi*x/2)*sin(pi*y) + 0.5*pi^2*cos(pi*x/2)*cos(pi*y)");
            SetWindowText(g_appData.hWestValue, "0.0"); // Neumann
            SetWindowText(g_appData.hEastValue, "1.0");
            SetWindowText(g_appData.hSouthValue, "0.0");
            SetWindowText(g_appData.hNorthValue, "0.0");
            MessageBox(NULL,
                "GENERAL ELLIPTIC\n\n"
                "Solves: General elliptic equation with variable coefficients\n"
                "(1+0.5sin(πx)cos(πy))∂²u/∂x² + 2·0.2∂²u/∂x∂y + (0.8+0.3cos(πx))∂²u/∂y² + 0.2x∂u/∂x + 0.1y∂u/∂y + 0.1u = sin(πx/2)sin(πy) + 0.5π²cos(πx/2)cos(πy)\n\n"
                "Description:\n"
                "General elliptic equation encompassing all physical effects:\n"
                "- Variable diffusion in both directions\n"
                "- Mixed derivative term\n"
                "- Convection in both directions\n"
                "- Reaction term\n\n"
                "Coefficients:\n"
                "- a11(x,y) = 1 + 0.5sin(πx)cos(πy) (variable diffusion)\n"
                "- a12 = 0.2 (mixed derivative coefficient)\n"
                "- a22(x,y) = 0.8 + 0.3cos(πx) (variable diffusion)\n"
                "- b1(x,y) = 0.2x (convection in x)\n"
                "- b2(x,y) = 0.1y (convection in y)\n"
                "- c = 0.1 (reaction)\n"
                "- f(x,y) = source term\n\n"
                "Boundary Conditions:\n"
                "- West: Neumann\n"
                "- Others: Dirichlet",
                "General Elliptic", MB_OK | MB_ICONINFORMATION);
            break;
    }
}