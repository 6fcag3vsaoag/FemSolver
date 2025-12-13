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
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        throw std::runtime_error("Window Registration Failed!");
    }
}

void GUIApp::run() {
    // Create the main window
    HWND hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "Finite Element Method Solver",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (hwnd == NULL) {
        throw std::runtime_error("Window Creation Failed!");
    }

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
    // Create static labels and edit controls for mesh parameters
    CreateWindow("Static", "Lx:", WS_VISIBLE | WS_CHILD, 10, 10, 50, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hLxEdit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 60, 10, 100, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    CreateWindow("Static", "Ly:", WS_VISIBLE | WS_CHILD, 10, 40, 50, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hLyEdit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 60, 40, 100, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    CreateWindow("Static", "Nx:", WS_VISIBLE | WS_CHILD, 10, 70, 50, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNxEdit = CreateWindow("Edit", "10", WS_VISIBLE | WS_CHILD | WS_BORDER, 60, 70, 100, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    CreateWindow("Static", "Ny:", WS_VISIBLE | WS_CHILD, 10, 100, 50, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNyEdit = CreateWindow("Edit", "10", WS_VISIBLE | WS_CHILD | WS_BORDER, 60, 100, 100, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    // Create static labels and edit controls for equation coefficients
    CreateWindow("Static", "a11(x,y):", WS_VISIBLE | WS_CHILD, 180, 10, 80, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA11Edit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 260, 10, 150, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    CreateWindow("Static", "a12(x,y):", WS_VISIBLE | WS_CHILD, 180, 40, 80, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA12Edit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 260, 40, 150, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    CreateWindow("Static", "a22(x,y):", WS_VISIBLE | WS_CHILD, 180, 70, 80, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA22Edit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 260, 70, 150, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    CreateWindow("Static", "b1(x,y):", WS_VISIBLE | WS_CHILD, 180, 100, 80, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hB1Edit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 260, 100, 150, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    CreateWindow("Static", "b2(x,y):", WS_VISIBLE | WS_CHILD, 180, 130, 80, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hB2Edit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 260, 130, 150, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    CreateWindow("Static", "c(x,y):", WS_VISIBLE | WS_CHILD, 180, 160, 80, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hCEdit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 260, 160, 150, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    CreateWindow("Static", "f(x,y):", WS_VISIBLE | WS_CHILD, 180, 190, 80, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hFEdit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 260, 190, 150, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    // Create buttons
    CreateWindow("Button", "Solve", WS_VISIBLE | WS_CHILD, 10, 140, 100, 30, hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);
    CreateWindow("Button", "Reset", WS_VISIBLE | WS_CHILD, 120, 140, 100, 30, hwnd, (HMENU)1002, GetModuleHandle(NULL), NULL);
    CreateWindow("Button", "Export", WS_VISIBLE | WS_CHILD, 230, 140, 100, 30, hwnd, (HMENU)1003, GetModuleHandle(NULL), NULL);
    
    // Create visualization frame (placeholder)
    g_appData.hVisualFrame = CreateWindow("Static", "Visualization Area", 
                                         WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTERIMAGE | SS_CENTER,
                                         10, 180, 700, 500, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    // Create status bar
    g_appData.hStatus = CreateWindow("Static", "Ready", 
                                    WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTERIMAGE | SS_CENTER,
                                    10, 690, 1160, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
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
    
    // For now, just show the values (in a real app, you'd solve the problem here)
    std::ostringstream oss;
    oss << "Solving: Lx=" << g_appData.Lx << ", Ly=" << g_appData.Ly 
        << ", Nx=" << g_appData.Nx << ", Ny=" << g_appData.Ny;
    
    SetWindowText(g_appData.hStatus, oss.str().c_str());
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