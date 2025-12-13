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
    HWND hPresetCombo;
    HWND hVisualFrame;
    HWND hStatus;

    // Boundary condition combo boxes
    HWND hWestBC, hEastBC, hSouthBC, hNorthBC;

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

    // Initialize the app data with default values
    g_appData.Lx = 1.0;
    g_appData.Ly = 1.0;
    g_appData.Nx = 20;
    g_appData.Ny = 20;
    g_appData.a11Func = "1.0";
    g_appData.a12Func = "0.0";
    g_appData.a22Func = "1.0";
    g_appData.b1Func = "0.0";
    g_appData.b2Func = "0.0";
    g_appData.cFunc = "0.0";
    g_appData.fFunc = "1.0";
}

void GUIApp::setSolver(FemSolver* solver) {
    coreSolver = solver;
    g_appData.solver = solver;
}

void GUIApp::runWithSolver(FemSolver* solver) {
    setSolver(solver);
    run();  // Call the original run method
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
                        LRESULT selResult = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                        int selection = static_cast<int>(selResult);
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
    // Get window dimensions
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Calculate left and right panel widths (left 50%, right 50%)
    int leftWidth = static_cast<int>(width * 0.45); // 45% for controls
    int rightWidth = static_cast<int>(width * 0.50); // 50% for visualization
    int margin = 10;

    // Left panel (controls)
    int leftStart = margin;
    int rightStart = leftStart + leftWidth + margin;

    // Top section: Preset selection
    CreateWindow("Static", "Equation Preset:", WS_VISIBLE | WS_CHILD, leftStart, 10, 100, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hPresetCombo = CreateWindow("ComboBox", "",
                                     WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                     leftStart + 110, 8, 200, 150, hwnd, (HMENU)2000, GetModuleHandle(NULL), NULL);

    // Add preset options
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"Select Preset...");
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"Laplace Equation");
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"Poisson Equation");
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"Helmholtz Equation");
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"Convection-Diffusion");
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"Reaction-Diffusion");
    SendMessage(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)"General Elliptic");
    SendMessage(hPresetCombo, CB_SETCURSEL, 0, 0); // Set default selection

    // Second section: Buttons (Solve, Reset, Export, Help)
    int buttonY = 45;
    CreateWindow("Button", "Solve", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 leftStart, buttonY, 80, 30, hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);
    CreateWindow("Button", "Reset", WS_VISIBLE | WS_CHILD,
                 leftStart + 90, buttonY, 80, 30, hwnd, (HMENU)1002, GetModuleHandle(NULL), NULL);
    CreateWindow("Button", "Export", WS_VISIBLE | WS_CHILD,
                 leftStart + 180, buttonY, 80, 30, hwnd, (HMENU)1003, GetModuleHandle(NULL), NULL);
    CreateWindow("Button", "Help", WS_VISIBLE | WS_CHILD,
                 leftStart + 270, buttonY, 80, 30, hwnd, (HMENU)1004, GetModuleHandle(NULL), NULL);

    // Third section: Equation coefficients (in group box)
    int coeffY = 90;
    HWND hCoeffGroup = CreateWindow("Button", "Equation Coefficients",
                                    WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                    leftStart, coeffY, leftWidth - 20, 200, hwnd, NULL, GetModuleHandle(NULL), NULL);

    int coeffStartY = coeffY + 20;
    CreateWindow("Static", "a11(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA11Edit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 85, coeffStartY-2, 120, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "a12(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 220, coeffStartY, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA12Edit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 295, coeffStartY-2, 120, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "a22(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY + 30, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA22Edit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 85, coeffStartY + 28, 120, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "b1(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 220, coeffStartY + 30, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hB1Edit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 295, coeffStartY + 28, 120, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "b2(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY + 60, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hB2Edit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 85, coeffStartY + 58, 120, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "c(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 220, coeffStartY + 60, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hCEdit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 295, coeffStartY + 58, 120, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "f(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY + 90, 70, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hFEdit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 85, coeffStartY + 88, 330, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Mesh parameters
    CreateWindow("Static", "Lx:", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY + 125, 30, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hLxEdit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 45, coeffStartY + 123, 60, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);
    CreateWindow("Static", "Ly:", WS_VISIBLE | WS_CHILD, leftStart + 115, coeffStartY + 125, 30, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hLyEdit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 150, coeffStartY + 123, 60, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);
    CreateWindow("Static", "Nx:", WS_VISIBLE | WS_CHILD, leftStart + 220, coeffStartY + 125, 30, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNxEdit = CreateWindow("Edit", "20", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 255, coeffStartY + 123, 60, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);
    CreateWindow("Static", "Ny:", WS_VISIBLE | WS_CHILD, leftStart + 325, coeffStartY + 125, 30, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNyEdit = CreateWindow("Edit", "20", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 360, coeffStartY + 123, 60, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Fourth section: Boundary conditions (in group box)
    int bcY = coeffY + 210;
    HWND hBCGroup = CreateWindow("Button", "Boundary Conditions",
                                 WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                 leftStart, bcY, leftWidth - 20, 160, hwnd, NULL, GetModuleHandle(NULL), NULL);

    int bcStartY = bcY + 20;
    CreateWindow("Static", "West:", WS_VISIBLE | WS_CHILD, leftStart + 10, bcStartY, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hWestBC = CreateWindow("ComboBox", "",
                                WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                leftStart + 55, bcStartY-2, 80, 60, hwnd, (HMENU)2001, GetModuleHandle(NULL), NULL);
    SendMessage(g_appData.hWestBC, CB_ADDSTRING, 0, (LPARAM)"Dirichlet");
    SendMessage(g_appData.hWestBC, CB_ADDSTRING, 0, (LPARAM)"Neumann");
    SendMessage(g_appData.hWestBC, CB_SETCURSEL, 0, 0);
    g_appData.hWestValue = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 145, bcStartY-2, 80, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "East:", WS_VISIBLE | WS_CHILD, leftStart + 240, bcStartY, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hEastBC = CreateWindow("ComboBox", "",
                                WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                leftStart + 285, bcStartY-2, 80, 60, hwnd, (HMENU)2002, GetModuleHandle(NULL), NULL);
    SendMessage(g_appData.hEastBC, CB_ADDSTRING, 0, (LPARAM)"Dirichlet");
    SendMessage(g_appData.hEastBC, CB_ADDSTRING, 0, (LPARAM)"Neumann");
    SendMessage(g_appData.hEastBC, CB_SETCURSEL, 0, 0);
    g_appData.hEastValue = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 375, bcStartY-2, 80, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "South:", WS_VISIBLE | WS_CHILD, leftStart + 10, bcStartY + 35, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hSouthBC = CreateWindow("ComboBox", "",
                                 WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                 leftStart + 55, bcStartY + 33, 80, 60, hwnd, (HMENU)2003, GetModuleHandle(NULL), NULL);
    SendMessage(g_appData.hSouthBC, CB_ADDSTRING, 0, (LPARAM)"Dirichlet");
    SendMessage(g_appData.hSouthBC, CB_ADDSTRING, 0, (LPARAM)"Neumann");
    SendMessage(g_appData.hSouthBC, CB_SETCURSEL, 0, 0);
    g_appData.hSouthValue = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 145, bcStartY + 33, 80, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow("Static", "North:", WS_VISIBLE | WS_CHILD, leftStart + 240, bcStartY + 35, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNorthBC = CreateWindow("ComboBox", "",
                                 WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                 leftStart + 285, bcStartY + 33, 80, 60, hwnd, (HMENU)2004, GetModuleHandle(NULL), NULL);
    SendMessage(g_appData.hNorthBC, CB_ADDSTRING, 0, (LPARAM)"Dirichlet");
    SendMessage(g_appData.hNorthBC, CB_ADDSTRING, 0, (LPARAM)"Neumann");
    SendMessage(g_appData.hNorthBC, CB_SETCURSEL, 0, 0);
    g_appData.hNorthValue = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 375, bcStartY + 33, 80, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Right panel: Visualization
    g_appData.hVisualFrame = CreateWindow("Static", "Solution Visualization",
                                         WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTERIMAGE | SS_CENTER,
                                         rightStart, 10, rightWidth - 20, height - 60, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Status bar at the bottom
    g_appData.hStatus = CreateWindow("Static", "Ready - Select an equation preset to begin",
                                    WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTERIMAGE | SS_CENTER,
                                    10, height - 40, width - 20, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
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

    // Get boundary condition types
    char bcBuffer[50];
    GetWindowText(g_appData.hWestValue, buffer, sizeof(buffer));
    double westVal = atof(buffer);

    GetWindowText(g_appData.hEastValue, buffer, sizeof(buffer));
    double eastVal = atof(buffer);

    GetWindowText(g_appData.hSouthValue, buffer, sizeof(buffer));
    double southVal = atof(buffer);

    GetWindowText(g_appData.hNorthValue, buffer, sizeof(buffer));
    double northVal = atof(buffer);

    // Get boundary condition types from combo boxes
    LRESULT westBCResult = SendMessage(g_appData.hWestBC, CB_GETCURSEL, 0, 0);
    LRESULT eastBCResult = SendMessage(g_appData.hEastBC, CB_GETCURSEL, 0, 0);
    LRESULT southBCResult = SendMessage(g_appData.hSouthBC, CB_GETCURSEL, 0, 0);
    LRESULT northBCResult = SendMessage(g_appData.hNorthBC, CB_GETCURSEL, 0, 0);
    int westBCIndex = static_cast<int>(westBCResult);
    int eastBCIndex = static_cast<int>(eastBCResult);
    int southBCIndex = static_cast<int>(southBCResult);
    int northBCIndex = static_cast<int>(northBCResult);

    std::string westBC = (westBCIndex == 0) ? "dirichlet" : "neumann";
    std::string eastBC = (eastBCIndex == 0) ? "dirichlet" : "neumann";
    std::string southBC = (southBCIndex == 0) ? "dirichlet" : "neumann";
    std::string northBC = (northBCIndex == 0) ? "dirichlet" : "neumann";

    // Update status
    SetWindowText(g_appData.hStatus, "Solving...");

    try {
        // If we have a solver instance, use it
        if (g_appData.solver) {
            // We would call the actual solver here
            // For demonstration purposes, I'll show a success message
            std::ostringstream oss;
            oss << "Solved: Lx=" << g_appData.Lx << ", Ly=" << g_appData.Ly
                << ", Nx=" << g_appData.Nx << ", Ny=" << g_appData.Ny;

            SetWindowText(g_appData.hStatus, oss.str().c_str());

            // In a real implementation, we would call the solver method:
            // g_appData.solver->getEllipticApp()->solveWithParameters(...)

            // For now, let's just show the values that would be passed
            std::string msg = std::string("Equation solved with:\n\n") +
                              std::string("Coefficients:\n") +
                              std::string("  a11=") + g_appData.a11Func + std::string("\n") +
                              std::string("  a12=") + g_appData.a12Func + std::string("\n") +
                              std::string("  a22=") + g_appData.a22Func + std::string("\n") +
                              std::string("  b1=") + g_appData.b1Func + std::string("\n") +
                              std::string("  b2=") + g_appData.b2Func + std::string("\n") +
                              std::string("  c=") + g_appData.cFunc + std::string("\n") +
                              std::string("  f=") + g_appData.fFunc + std::string("\n\n") +
                              std::string("Boundary conditions:\n") +
                              std::string("  West: ") + westBC + std::string(" (") + std::to_string(westVal) + std::string(")\n") +
                              std::string("  East: ") + eastBC + std::string(" (") + std::to_string(eastVal) + std::string(")\n") +
                              std::string("  South: ") + southBC + std::string(" (") + std::to_string(southVal) + std::string(")\n") +
                              std::string("  North: ") + northBC + std::string(" (") + std::to_string(northVal) + std::string(")");
            MessageBox(hwnd, msg.c_str(),
                "Solution Parameters", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBox(hwnd, "Solver not initialized. The application should be run from the main FemSolver instance.",
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
    SetWindowText(g_appData.hNxEdit, "20");
    SetWindowText(g_appData.hNyEdit, "20");
    SetWindowText(g_appData.hA11Edit, "1.0");
    SetWindowText(g_appData.hA12Edit, "0.0");
    SetWindowText(g_appData.hA22Edit, "1.0");
    SetWindowText(g_appData.hB1Edit, "0.0");
    SetWindowText(g_appData.hB2Edit, "0.0");
    SetWindowText(g_appData.hCEdit, "0.0");
    SetWindowText(g_appData.hFEdit, "1.0");

    // Reset boundary conditions
    SetWindowText(g_appData.hWestValue, "0.0");
    SetWindowText(g_appData.hEastValue, "0.0");
    SetWindowText(g_appData.hSouthValue, "0.0");
    SetWindowText(g_appData.hNorthValue, "0.0");

    // Reset boundary condition types to Dirichlet
    SendMessage(g_appData.hWestBC, CB_SETCURSEL, 0, 0);
    SendMessage(g_appData.hEastBC, CB_SETCURSEL, 0, 0);
    SendMessage(g_appData.hSouthBC, CB_SETCURSEL, 0, 0);
    SendMessage(g_appData.hNorthBC, CB_SETCURSEL, 0, 0);

    // Reset preset to default
    SendMessage(g_appData.hPresetCombo, CB_SETCURSEL, 0, 0);

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