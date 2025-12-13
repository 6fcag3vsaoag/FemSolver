#include "GUIApp.h"
#include "FemSolver.h"
#include "EllipticApp.h"
#include <iostream>
#include <stdexcept>
#define NOMINMAX  // Prevent windows.h from defining min/max macros
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

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

        case WM_PAINT:
            {
                // Handle painting for the visualization frame
                HWND visualFrame = g_appData.hVisualFrame;
                if (visualFrame && IsWindow(hwnd)) {
                    // Check if this paint message is for the visualization frame
                    if (reinterpret_cast<HWND>(wParam) == visualFrame) {
                        PAINTSTRUCT ps;
                        HDC hdc = BeginPaint(visualFrame, &ps);

                        RECT rect;
                        GetClientRect(visualFrame, &rect);

                        // Fill background
                        HBRUSH bgBrush = CreateSolidBrush(RGB(240, 240, 240));
                        FillRect(hdc, &rect, bgBrush);
                        DeleteObject(bgBrush);

                        // If we have a solution, visualize it
                        if (g_appData.solver) {
                            EllipticApp* ellipticApp = g_appData.solver->getApp();
                            if (ellipticApp) {
                                const std::vector<double>& solution = ellipticApp->getSolution();
                                const Mesh& mesh = ellipticApp->getMesh();

                                if (!solution.empty()) {
                                    // Draw title
                                    SetTextColor(hdc, RGB(0, 0, 0));
                                    SetBkMode(hdc, TRANSPARENT);
                                    std::string title = "Solution Visualization";
                                    TextOutA(hdc, 10, 10, title.c_str(), static_cast<int>(title.length()));

                                    // Draw solution stats
                                    double min_val = *std::min_element(solution.begin(), solution.end());
                                    double max_val = *std::max_element(solution.begin(), solution.end());

                                    std::ostringstream stats;
                                    stats << "Nodes: " << solution.size()
                                          << ", Min: " << min_val
                                          << ", Max: " << max_val;
                                    TextOutA(hdc, 10, 30, stats.str().c_str(), static_cast<int>(stats.str().length()));

                                    // Draw a simplified visualization
                                    int visChartLeft = 10;
                                    int visChartTop = 60;
                                    int visChartWidth = rect.right - 20;
                                    int visChartHeight = rect.bottom - 80;

                                    if (visChartWidth > 20 && visChartHeight > 20) {
                                        // Draw bounding rectangle
                                        Rectangle(hdc, visChartLeft, visChartTop, visChartLeft + visChartWidth, visChartTop + visChartHeight);

                                        // Draw solution as a color gradient if we have data
                                        if (solution.size() >= 4) {
                                            double localMinVal = *std::min_element(solution.begin(), solution.end());
                                            double localMaxVal = *std::max_element(solution.begin(), solution.end());
                                            double range = (localMaxVal == localMinVal) ? 1.0 : (localMaxVal - localMinVal);

                                            // Use a simplified grid view based on the mesh
                                            int gridX = (g_appData.Nx < 40) ? g_appData.Nx : 40;  // Limit grid for display
                                            int gridY = (g_appData.Ny < 40) ? g_appData.Ny : 40;

                                            int cellWidth = visChartWidth / gridX;
                                            int cellHeight = visChartHeight / gridY;

                                            if (cellWidth >= 1 && cellHeight >= 1) {
                                                for (int y = 0; y < gridY && y < g_appData.Ny; y++) {
                                                    for (int x = 0; x < gridX && x < g_appData.Nx; x++) {
                                                        int idx = y * g_appData.Nx + x;
                                                        if (idx >= 0 && idx < static_cast<int>(solution.size())) {
                                                            double val = solution[idx];
                                                            double normVal = (val - localMinVal) / range;

                                                            // Create color based on solution value (blue=low, red=high)
                                                            int r = static_cast<int>(normVal * 255);
                                                            int b = static_cast<int>((1.0 - normVal) * 255);
                                                            int g = 50; // Keep some green

                                                            HBRUSH cellBrush = CreateSolidBrush(RGB(r, g, b));
                                                            RECT cellRect;
                                                            cellRect.left = visChartLeft + x * cellWidth;
                                                            cellRect.top = visChartTop + y * cellHeight;
                                                            cellRect.right = cellRect.left + cellWidth;
                                                            cellRect.bottom = cellRect.top + cellHeight;

                                                            FillRect(hdc, &cellRect, cellBrush);
                                                            FrameRect(hdc, &cellRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
                                                            DeleteObject(cellBrush);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                } else {
                                    // No solution yet - show placeholder
                                    SetTextColor(hdc, RGB(128, 128, 128));
                                    std::string placeholder = "No solution computed yet.\nClick 'Solve' to compute.";
                                    DrawTextA(hdc, placeholder.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                                }
                            } else {
                                // No elliptic app
                                SetTextColor(hdc, RGB(128, 128, 128));
                                std::string noAppText = "Solver not initialized.\nRun from main application.";
                                DrawTextA(hdc, noAppText.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                            }
                        } else {
                            // No solver - show instructions
                            SetTextColor(hdc, RGB(128, 128, 128));
                            std::string noSolverText = "Solver not available.\nNo solution to visualize.";
                            DrawTextA(hdc, noSolverText.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                        }

                        EndPaint(visualFrame, &ps);
                        break; // Handle internally, don't pass to DefWindowProc
                    }
                }
            }
            // Intentionally not calling DefWindowProc for visualization frame
            // Pass to default for main window painting
            return DefWindowProc(hwnd, msg, wParam, lParam);

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
            // Call the solver with the parameters from the GUI
            EllipticApp* ellipticApp = g_appData.solver->getApp();
            if (ellipticApp) {
                // Solve the equation with the parameters obtained from GUI
                ellipticApp->solveWithParameters(
                    g_appData.Lx, g_appData.Ly, g_appData.Nx, g_appData.Ny,
                    g_appData.a11Func, g_appData.a12Func, g_appData.a22Func,
                    g_appData.b1Func, g_appData.b2Func, g_appData.cFunc, g_appData.fFunc,
                    westBC, eastBC, southBC, northBC,
                    westVal, eastVal, southVal, northVal
                );

                // Update status to show solution is computed
                SetWindowText(g_appData.hStatus, "Solution computed successfully!");

                // Show solution information
                const std::vector<double>& solution = ellipticApp->getSolution();
                const Mesh& mesh = ellipticApp->getMesh();

                if (!solution.empty()) {
                    double min_val = *std::min_element(solution.begin(), solution.end());
                    double max_val = *std::max_element(solution.begin(), solution.end());

                    std::ostringstream oss;
                    oss << "Solution computed: " << solution.size() << " nodes, "
                        << "Range: [" << min_val << ", " << max_val << "]";
                    SetWindowText(g_appData.hStatus, oss.str().c_str());

                    // Update the visualization area with solution data
                    InvalidateRect(g_appData.hVisualFrame, NULL, TRUE);

                    // Update the visualization area to show solution data
                    HDC hdc = GetDC(g_appData.hVisualFrame);
                    if (hdc) {
                        RECT rect;
                        GetClientRect(g_appData.hVisualFrame, &rect);

                        // Fill background
                        HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
                        FillRect(hdc, &rect, hBrush);
                        DeleteObject(hBrush);

                        // Draw title
                        std::string title = "Solution Visualization\n";
                        SetTextColor(hdc, RGB(0, 0, 0));
                        SetBkMode(hdc, TRANSPARENT);
                        TextOutA(hdc, 10, 10, title.c_str(), static_cast<int>(title.length()));

                        // Draw solution statistics
                        std::string stats = "Nodes: " + std::to_string(solution.size()) +
                                           ", Elements: " + std::to_string(mesh.elements.size()) +
                                           "\nMin: " + std::to_string(min_val) +
                                           ", Max: " + std::to_string(max_val);
                        TextOutA(hdc, 10, 30, stats.c_str(), static_cast<int>(stats.length()));

                        // If there are enough nodes to visualize, draw a simple representation
                        if (solution.size() >= 4) {
                            // Draw a simplified representation of the solution
                            int chartLeft = 10;
                            int chartTop = 60;
                            int chartWidth = rect.right - 20;
                            int chartHeight = rect.bottom - 80;

                            if (chartWidth > 0 && chartHeight > 0) {
                                // Draw border for chart
                                Rectangle(hdc, chartLeft, chartTop, chartLeft + chartWidth, chartTop + chartHeight);

                                // Determine grid dimensions based on original mesh parameters
                                int gridX = (g_appData.Nx < 50) ? g_appData.Nx : 50;  // Limit for display
                                int gridY = (g_appData.Ny < 50) ? g_appData.Ny : 50;

                                // Draw a color gradient representation
                                int cellWidth = chartWidth / gridX;
                                int cellHeight = chartHeight / gridY;

                                if (cellWidth > 0 && cellHeight > 0) {
                                    // Scale factors for mapping solution values to colors
                                    double range = max_val - min_val;
                                    if (range == 0) range = 1; // Avoid division by zero

                                    // Draw grid cells with colors based on solution values
                                    for (int y = 0; y < gridY && y < g_appData.Ny; y++) {
                                        for (int x = 0; x < gridX && x < g_appData.Nx; x++) {
                                            int index = y * g_appData.Nx + x;
                                            if (index >= 0 && index < static_cast<int>(solution.size())) {
                                                double val = solution[index];
                                                double normalized = (val - min_val) / range;

                                                // Map to color: blue (low) to red (high)
                                                int r = static_cast<int>(normalized * 255);
                                                int b = static_cast<int>((1.0 - normalized) * 255);
                                                int g = 50; // Constant green component

                                                COLORREF color = RGB(r, g, b);
                                                HBRUSH brush = CreateSolidBrush(color);
                                                RECT cellRect;
                                                cellRect.left = chartLeft + x * cellWidth;
                                                cellRect.top = chartTop + y * cellHeight;
                                                cellRect.right = cellRect.left + cellWidth;
                                                cellRect.bottom = cellRect.top + cellHeight;
                                                FillRect(hdc, &cellRect, brush);
                                                FrameRect(hdc, &cellRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
                                                DeleteObject(brush);
                                            }
                                        }
                                    }
                                }

                                // Draw legend
                                int legendX = chartLeft + chartWidth + 5;
                                int legendY = chartTop;
                                int legendHeight = 100;
                                int legendWidth = 20;

                                if (legendY + legendHeight < rect.bottom && legendX + legendWidth < rect.right) {
                                    // Draw color legend
                                    for (int i = 0; i < legendHeight; i++) {
                                        double ratio = static_cast<double>(i) / legendHeight;
                                        int r = static_cast<int>(ratio * 255);
                                        int b = static_cast<int>((1.0 - ratio) * 255);
                                        int g = 50;

                                        HBRUSH legendBrush = CreateSolidBrush(RGB(r, g, b));
                                        RECT legendRect;
                                        legendRect.left = legendX;
                                        legendRect.top = legendY + i;
                                        legendRect.right = legendX + legendWidth;
                                        legendRect.bottom = legendY + i + 1;
                                        FillRect(hdc, &legendRect, legendBrush);
                                        DeleteObject(legendBrush);
                                    }

                                    // Draw legend labels
                                    std::string lowLabel = std::to_string(min_val);
                                    std::string highLabel = std::to_string(max_val);
                                    TextOutA(hdc, legendX + legendWidth + 5, legendY, lowLabel.c_str(), static_cast<int>(lowLabel.length()));
                                    TextOutA(hdc, legendX + legendWidth + 5, legendY + legendHeight - 15, highLabel.c_str(), static_cast<int>(highLabel.length()));
                                }
                            }
                        }

                        ReleaseDC(g_appData.hVisualFrame, hdc);
                    }
                }
            } else {
                MessageBox(hwnd, "Failed to access solver application.",
                          "Error", MB_OK | MB_ICONERROR);
            }
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

    if (g_appData.solver) {
        EllipticApp* ellipticApp = g_appData.solver->getApp();
        if (ellipticApp) {
            const std::vector<double>& solution = ellipticApp->getSolution();
            const Mesh& mesh = ellipticApp->getMesh();

            if (!solution.empty()) {
                // Export solution data to a file
                OPENFILENAME ofn;
                char szFile[260] = "";

                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

                if (GetSaveFileName(&ofn)) {
                    std::ofstream outFile(ofn.lpstrFile);
                    if (outFile.is_open()) {
                        // Write solution data to file
                        outFile << "# Finite Element Solution Data\n";
                        outFile << "# Generated by FEM Solver\n";
                        outFile << "# Domain: [0, " << g_appData.Lx << "] x [0, " << g_appData.Ly << "]\n";
                        outFile << "# Mesh: " << g_appData.Nx << " x " << g_appData.Ny << " nodes\n";
                        outFile << "# Coefficients:\n";
                        outFile << "#   a11(x,y) = " << g_appData.a11Func << "\n";
                        outFile << "#   a12(x,y) = " << g_appData.a12Func << "\n";
                        outFile << "#   a22(x,y) = " << g_appData.a22Func << "\n";
                        outFile << "#   b1(x,y) = " << g_appData.b1Func << "\n";
                        outFile << "#   b2(x,y) = " << g_appData.b2Func << "\n";
                        outFile << "#   c(x,y) = " << g_appData.cFunc << "\n";
                        outFile << "#   f(x,y) = " << g_appData.fFunc << "\n";
                        outFile << "\n";
                        outFile << "# Node_ID\tX_coord\tY_coord\tSolution_Value\n";

                        // Write node data with solution values
                        size_t min_size = (mesh.nodes.size() < solution.size()) ? mesh.nodes.size() : solution.size();
                        for (size_t i = 0; i < min_size; ++i) {
                            outFile << i << "\t"
                                   << mesh.nodes[i].first << "\t"
                                   << mesh.nodes[i].second << "\t"
                                   << solution[i] << "\n";
                        }

                        // Write element connectivity
                        outFile << "\n# Element Connectivity\n";
                        outFile << "# Element_ID\tNode1\tNode2\tNode3\n";
                        for (size_t i = 0; i < mesh.elements.size(); ++i) {
                            outFile << i << "\t"
                                   << mesh.elements[i][0] << "\t"
                                   << mesh.elements[i][1] << "\t"
                                   << mesh.elements[i][2] << "\n";
                        }

                        outFile.close();

                        SetWindowText(g_appData.hStatus, "Results successfully exported!");
                        MessageBox(hwnd, "Results exported successfully to:\n", "Export Complete", MB_OK | MB_ICONINFORMATION);
                    } else {
                        SetWindowText(g_appData.hStatus, "Error: Could not open file for export.");
                        MessageBox(hwnd, "Error opening file for export.", "Export Error", MB_OK | MB_ICONERROR);
                    }
                } else {
                    SetWindowText(g_appData.hStatus, "Export cancelled.");
                }
            } else {
                MessageBox(hwnd, "No solution data available to export. Please solve the problem first.",
                          "No Data", MB_OK | MB_ICONINFORMATION);
                SetWindowText(g_appData.hStatus, "No solution data to export. Solve first.");
            }
        } else {
            MessageBox(hwnd, "Cannot access solver application.", "Error", MB_OK | MB_ICONERROR);
            SetWindowText(g_appData.hStatus, "Cannot access solver application.");
        }
    } else {
        MessageBox(hwnd, "Solver not initialized.", "Error", MB_OK | MB_ICONERROR);
        SetWindowText(g_appData.hStatus, "Solver not initialized.");
    }
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