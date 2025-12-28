#include "GUIApp.h"
#include "FemSolver.h"
#include "EllipticApp.h"
#include "StringUtils.h"
#include "Localization.h" // Include the new localization module
#include "PresetManager.h" // Include the new preset manager module
#include "GUIAppTypes.h" // Include the GUI app types module
#include "WindowEventHandler.h" // Include the window event handler module
#include "VisualizationFactory.h" // Include the visualization factory
#include <iostream>
#include <stdexcept>
#include <clocale>
#include <cwchar>
#include <locale>
#include <codecvt>
#define NOMINMAX  // Prevent windows.h from defining min/max macros
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <iomanip>

// Define window class name
const wchar_t g_szClassName[] = L"FemSolverWindowClass";

// Global language context is now in the Localization module
// Using the functions from Localization.h

AppData g_appData;
WNDPROC g_pfnOldVisualFrameProc = NULL;
IVisualizer* g_currentVisualizer = nullptr; // For global access to current visualizer
VisualizationManager* g_visualizationManager = nullptr; // Global visualization manager instance
SolutionManager* g_solutionManager = nullptr; // Global solution manager instance
ExportManager* g_exportManager = nullptr; // Global export manager instance
ResetManager* g_resetManager = nullptr; // Global reset manager instance

// Forward function declarations
LRESULT CALLBACK VisualFrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateControls(HWND hwnd, IVisualizer* visualizer);
void OnSolveButtonClicked(HWND hwnd);
void OnResetButtonClicked(HWND hwnd);
void OnExportButtonClicked(HWND hwnd);
void OnPresetHelpClicked(HWND hwnd);
void OnPresetChanged(HWND hwnd, int presetIndex);
void LoadPreset(int presetIndex);
void SwitchLanguage();
void UpdateLanguageStrings(HWND hwnd);

// Custom Window Procedure for the Visualization Frame
LRESULT CALLBACK VisualFrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT:
        {
            // For DirectX, we don't handle WM_PAINT directly since DirectX manages its own rendering
            // Instead, we would trigger DirectX rendering through a timer or other mechanism
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // Just clear the paint struct
            EndPaint(hwnd, &ps);

            // Trigger rendering for the current visualizer after painting
            if (g_currentVisualizer) {
                g_currentVisualizer->render();
            }
            return 0;
        }
        case WM_SIZE:
        {
            // Handle window resize for DirectX
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            // If we have a visualizer, resize it
            if (g_currentVisualizer) {
                g_currentVisualizer->resize(width, height);
            }
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {
            // Capture mouse for camera control
            SetCapture(hwnd);

            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            bool leftButton = (wParam & MK_LBUTTON) != 0;
            bool rightButton = (wParam & MK_RBUTTON) != 0;

            if (g_currentVisualizer) {
                g_currentVisualizer->handleMouseInput(x, y, leftButton, rightButton);

                // Force a redraw to reflect the camera changes
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            // Check if mouse buttons are pressed for camera control
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            bool leftButton = (wParam & MK_LBUTTON) != 0;
            bool rightButton = (wParam & MK_RBUTTON) != 0;

            if (leftButton || rightButton) {
                if (g_currentVisualizer) {
                    g_currentVisualizer->handleMouseInput(x, y, leftButton, rightButton);

                    // Force a redraw to reflect the camera changes
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        {
            // Release mouse capture
            ReleaseCapture();
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);

            if (g_currentVisualizer) {
                g_currentVisualizer->handleMouseWheel(delta);

                // Force a redraw to reflect the camera changes
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        case WM_DESTROY:
        {
            return 0;
        }
    }
    return CallWindowProc(g_pfnOldVisualFrameProc, hwnd, msg, wParam, lParam);
}


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

    // Initialize DirectX Visualizer (instead of GDI Visualizer)
    visualizer_ = VisualizationFactory::createVisualizer(VisualizationType::Direct3D);
    g_currentVisualizer = visualizer_.get(); // Set global access to current visualizer

    // Initialize Visualization Manager with the DirectX visualizer
    visualizationManager_ = std::make_unique<VisualizationManager>();
    visualizationManager_->initialize(visualizer_.get());

    // Set the global visualization manager
    g_visualizationManager = visualizationManager_.get();

    // Initialize Solution Manager
    solutionManager_ = std::make_unique<SolutionManager>();

    // Set the global solution manager
    g_solutionManager = solutionManager_.get();

    // Initialize Export Manager
    exportManager_ = std::make_unique<ExportManager>();

    // Set the global export manager
    g_exportManager = exportManager_.get();

    // Initialize Reset Manager
    resetManager_ = std::make_unique<ResetManager>();
    resetManager_->initialize(&g_appData);

    // Set the global reset manager
    g_resetManager = resetManager_.get();
}

void GUIApp::setSolver(FemSolver* solver) {
    coreSolver = solver;
    g_appData.solver = solver;

    // Initialize the solution manager with the solver and app data
    if (solutionManager_) {
        solutionManager_->initialize(solver, &g_appData);
    }

    // Initialize the export manager with the solver and app data
    if (exportManager_) {
        exportManager_->initialize(solver, &g_appData);
    }
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
    WNDCLASSEXW wc;
    wc.cbSize = sizeof(WNDCLASSEXW);
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

    if (!RegisterClassExW(&wc)) {
        throw std::runtime_error("Window Registration Failed!");
    }
}

int GUIApp::run() {
    // Create the main window with better size and positioning
    HWND hwnd = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        L"Finite Element Method Solver - Elliptic Equations", // Default title, will be updated after creation
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1100, 750,  // More reasonable initial size with proper aspect ratio
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

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Set the app data for the window event handler
    WindowEventHandler::setAppData(&g_appData);
    // Call the window event handler
    return WindowEventHandler::WndProc(hwnd, msg, wParam, lParam);
}

void CreateControls(HWND hwnd, IVisualizer* visualizer) {
    // Get window dimensions
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Calculate left and right panel widths (left 16%, right 80% with 4% margin between)
    int leftWidth = static_cast<int>(width * 0.16); // Reduced to 16% for controls (further reduced from 20%)
    int rightWidth = static_cast<int>(width * 0.80); // 80% for visualization
    int margin = static_cast<int>(width * 0.02); // Responsive margin

    // Left panel (controls)
    int leftStart = margin;
    int rightStart = leftStart + leftWidth + margin;

    // Top section: Preset selection
    int presetLabelWidth = static_cast<int>(leftWidth * 0.28); // About 28% of the left panel width
    int helpButtonWidth = 30; // Width for the '?' button
    int comboWidth = leftWidth - presetLabelWidth - helpButtonWidth - 15; // Remaining width for the combo box

    g_appData.hPresetLabel = CreateWindowW(L"Static", getLanguageContext().getEquationPreset(), WS_VISIBLE | WS_CHILD, leftStart, 10, presetLabelWidth, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hPresetCombo = CreateWindowW(L"ComboBox", L"",
                                          WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                          leftStart + presetLabelWidth + 5, 8, comboWidth, 150, hwnd, (HMENU)2000, GetModuleHandle(NULL), NULL);

    // Add Help button
    CreateWindowW(L"Button", L"?", WS_VISIBLE | WS_CHILD,
                  leftStart + presetLabelWidth + comboWidth + 10, 8, helpButtonWidth, 28, hwnd, (HMENU)1006, GetModuleHandle(NULL), NULL);


    // Add preset options based on current language
    for (int i = 0; i < 7; i++) {
        SendMessageW(g_appData.hPresetCombo, CB_ADDSTRING, 0, (LPARAM)getLanguageContext().getPresetOption(i));
    }
    SendMessageW(g_appData.hPresetCombo, CB_SETCURSEL, 0, 0); // Set default selection

    // Second section: Buttons (Solve, Reset, Export, Language Toggle) - properly aligned layout
    int buttonY = 45;
    int buttonHeight = 30;
    int buttonSpacing = 2; // Fixed 2px spacing between buttons as requested
    int totalButtons = 4; // Reduced from 5 (removed Help button)
    int buttonMargin = 10; // Left and right margin for buttons (different from panel margin)

    // Calculate individual button widths based on text length to prevent overlapping
    HDC hdc = GetDC(hwnd);
    HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    // Measure text widths for each button in the current language
    SIZE textSize;
    int solveWidth = 60; // Default width
    int resetWidth = 60;
    int exportWidth = 60;
    int langWidth = 60;

    // Calculate text width for Solve button
    GetTextExtentPoint32W(hdc, getLanguageContext().getSolveBtn(), wcslen(getLanguageContext().getSolveBtn()), &textSize);
    solveWidth = std::max<int>(50, textSize.cx + 16); // Add padding

    // Calculate text width for Reset button
    GetTextExtentPoint32W(hdc, getLanguageContext().getResetBtn(), wcslen(getLanguageContext().getResetBtn()), &textSize);
    resetWidth = std::max<int>(50, textSize.cx + 16); // Add padding

    // Calculate text width for Export button
    GetTextExtentPoint32W(hdc, getLanguageContext().getExportBtn(), wcslen(getLanguageContext().getExportBtn()), &textSize);
    exportWidth = std::max<int>(50, textSize.cx + 16); // Add padding

    // Calculate text width for Language Toggle button
    const wchar_t* langToggleText = (getLanguageContext().getLanguageType() == Language::English) ? L"RU" : L"EN";
    GetTextExtentPoint32W(hdc, langToggleText, wcslen(langToggleText), &textSize);
    langWidth = std::max<int>(50, textSize.cx + 16); // Add padding

    SelectObject(hdc, hOldFont);
    ReleaseDC(hwnd, hdc);

    // Calculate total required width
    int totalRequiredWidth = solveWidth + resetWidth + exportWidth + langWidth
                           + (buttonSpacing * (totalButtons - 1)) + (buttonMargin * 2);

    // If total required width exceeds available space, scale down proportionally
    if (totalRequiredWidth > leftWidth) {
        double scale = (double)(leftWidth - (buttonMargin * 2) - (buttonSpacing * (totalButtons - 1))) /
                      (double)(solveWidth + resetWidth + exportWidth + langWidth);
        solveWidth = (int)(solveWidth * scale);
        resetWidth = (int)(resetWidth * scale);
        exportWidth = (int)(exportWidth * scale);
        langWidth = (int)(langWidth * scale);
    }

    // Position buttons with calculated widths
    int currentX = leftStart + buttonMargin;

    // Create Solve button
    CreateWindowW(L"Button", getLanguageContext().getSolveBtn(), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 currentX, buttonY, solveWidth, buttonHeight, hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);

    // Create Reset button
    currentX += solveWidth + buttonSpacing;
    CreateWindowW(L"Button", getLanguageContext().getResetBtn(), WS_VISIBLE | WS_CHILD,
                 currentX, buttonY, resetWidth, buttonHeight, hwnd, (HMENU)1002, GetModuleHandle(NULL), NULL);

    // Create Export button
    currentX += resetWidth + buttonSpacing;
    CreateWindowW(L"Button", getLanguageContext().getExportBtn(), WS_VISIBLE | WS_CHILD,
                 currentX, buttonY, exportWidth, buttonHeight, hwnd, (HMENU)1003, GetModuleHandle(NULL), NULL);

    // Create Language Toggle button
    currentX += exportWidth + buttonSpacing;
    CreateWindowW(L"Button", langToggleText,
                 WS_VISIBLE | WS_CHILD,
                 currentX, buttonY, langWidth, buttonHeight, hwnd, (HMENU)1005, GetModuleHandle(NULL), NULL);

    // Third section: Equation coefficients (in group box) - with proper spacing
    int coeffY = buttonY + buttonHeight + 10;  // Position just below the buttons with appropriate spacing
    g_appData.hCoeffGroup = CreateWindowW(L"Button", getLanguageContext().getCoeffGroup(),
                                    WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                    leftStart, coeffY, leftWidth - 10, 200, hwnd, NULL, GetModuleHandle(NULL), NULL);

    int coeffStartY = coeffY + 20;
    int coeffLabelWidth = static_cast<int>(leftWidth * 0.15);  // Reduced width for labels (like "a11(x,y):")
    int coeffFieldWidth = static_cast<int>(leftWidth * 0.28);  // Reduced width for text fields
    int coeffSpacingX = coeffLabelWidth + 5;  // Reduced spacing between label and field

    g_appData.hCoeffLabels[0] = CreateWindowW(L"Static", L"a11(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA11Edit = CreateWindowW(L"Edit", L"1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX, coeffStartY-2, coeffFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hCoeffLabels[1] = CreateWindowW(L"Static", L"a12(x,y):", WS_VISIBLE | WS_CHILD, leftStart + coeffSpacingX + coeffFieldWidth + 10, coeffStartY, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA12Edit = CreateWindowW(L"Edit", L"0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX + coeffFieldWidth + coeffLabelWidth + 10, coeffStartY-2, coeffFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hCoeffLabels[2] = CreateWindowW(L"Static", L"a22(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY + 30, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA22Edit = CreateWindowW(L"Edit", L"1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX, coeffStartY + 28, coeffFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hCoeffLabels[3] = CreateWindowW(L"Static", L"b1(x,y):", WS_VISIBLE | WS_CHILD, leftStart + coeffSpacingX + coeffFieldWidth + 10, coeffStartY + 30, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hB1Edit = CreateWindowW(L"Edit", L"0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX + coeffFieldWidth + coeffLabelWidth + 10, coeffStartY + 28, coeffFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hCoeffLabels[4] = CreateWindowW(L"Static", L"b2(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY + 60, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hB2Edit = CreateWindowW(L"Edit", L"0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX, coeffStartY + 58, coeffFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hCoeffLabels[5] = CreateWindowW(L"Static", L"c(x,y):", WS_VISIBLE | WS_CHILD, leftStart + coeffSpacingX + coeffFieldWidth + 10, coeffStartY + 60, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hCEdit = CreateWindowW(L"Edit", L"0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX + coeffFieldWidth + coeffLabelWidth + 10, coeffStartY + 58, coeffFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hCoeffLabels[6] = CreateWindowW(L"Static", L"f(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY + 90, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hFEdit = CreateWindowW(L"Edit", L"1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX, coeffStartY + 88, coeffFieldWidth * 2 + coeffLabelWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Mesh parameters
    int meshLabelWidth = static_cast<int>(leftWidth * 0.06);  // Reduced width for labels like "Lx:"
    int meshFieldWidth = static_cast<int>(leftWidth * 0.10);  // Reduced width for text fields
    int meshSpacing = static_cast<int>(leftWidth * 0.02);     // Reduced spacing between elements

    g_appData.hMeshLabels[0] = CreateWindowW(L"Static", L"Lx:", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY + 125, meshLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hLxEdit = CreateWindowW(L"Edit", L"1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hMeshLabels[1] = CreateWindowW(L"Static", L"Ly:", WS_VISIBLE | WS_CHILD, leftStart + 10 + meshLabelWidth + meshFieldWidth + meshSpacing, coeffStartY + 125, meshLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hLyEdit = CreateWindowW(L"Edit", L"1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + meshLabelWidth + meshFieldWidth + meshSpacing + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hMeshLabels[2] = CreateWindowW(L"Static", L"Nx:", WS_VISIBLE | WS_CHILD, leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 2, coeffStartY + 125, meshLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNxEdit = CreateWindowW(L"Edit", L"20", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 2 + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hMeshLabels[3] = CreateWindowW(L"Static", L"Ny:", WS_VISIBLE | WS_CHILD, leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 3, coeffStartY + 125, meshLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNyEdit = CreateWindowW(L"Edit", L"20", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 3 + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Fourth section: Boundary conditions (in group box)
    int bcY = coeffY + 210;
    g_appData.hBCGroup = CreateWindowW(L"Button", getLanguageContext().getBcGroup(),
                                 WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                 leftStart, bcY, leftWidth - 10, 160, hwnd, NULL, GetModuleHandle(NULL), NULL);

    int bcStartY = bcY + 20;
    int bcLabelWidth = static_cast<int>(leftWidth * 0.08);  // Reduced width for labels like "West:"
    int bcComboWidth = static_cast<int>(leftWidth * 0.18);  // Reduced width for combo boxes
    int bcValueWidth = static_cast<int>(leftWidth * 0.18);  // Reduced width for value fields

    // Calculate positions for east/south controls to avoid overflow
    int eastStartX = leftStart + static_cast<int>(leftWidth * 0.5); // Position on the right half of the group box

    g_appData.hBCLabels[0] = CreateWindowW(L"Static", L"West:", WS_VISIBLE | WS_CHILD, leftStart + 10, bcStartY, bcLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hWestBC = CreateWindowW(L"ComboBox", L"",
                                WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                leftStart + 10 + bcLabelWidth, bcStartY-2, bcComboWidth, 60, hwnd, (HMENU)2001, GetModuleHandle(NULL), NULL);
    SendMessageW(g_appData.hWestBC, CB_ADDSTRING, 0, (LPARAM)getLanguageContext().getBcType(0)); // Dirichlet
    SendMessageW(g_appData.hWestBC, CB_ADDSTRING, 0, (LPARAM)getLanguageContext().getBcType(1)); // Neumann
    SendMessageW(g_appData.hWestBC, CB_SETCURSEL, 0, 0);
    // Calculate available space for West value field to prevent overflow
    int westValueWidth = eastStartX - (leftStart + 10 + bcLabelWidth + bcComboWidth + 10) - 5;
    g_appData.hWestValue = CreateWindowW(L"Edit", L"0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + bcLabelWidth + bcComboWidth + 10, bcStartY-2, westValueWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hBCLabels[1] = CreateWindowW(L"Static", L"East:", WS_VISIBLE | WS_CHILD, eastStartX, bcStartY, bcLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hEastBC = CreateWindowW(L"ComboBox", L"",
                                WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                eastStartX + bcLabelWidth, bcStartY-2, bcComboWidth, 60, hwnd, (HMENU)2002, GetModuleHandle(NULL), NULL);
    SendMessageW(g_appData.hEastBC, CB_ADDSTRING, 0, (LPARAM)getLanguageContext().getBcType(0)); // Dirichlet
    SendMessageW(g_appData.hEastBC, CB_ADDSTRING, 0, (LPARAM)getLanguageContext().getBcType(1)); // Neumann
    SendMessageW(g_appData.hEastBC, CB_SETCURSEL, 0, 0);
    // Calculate available space for East value field to prevent overflow
    int eastValueWidth = leftWidth - 15 - (eastStartX + bcLabelWidth + bcComboWidth + 5);
    g_appData.hEastValue = CreateWindowW(L"Edit", L"0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, eastStartX + bcLabelWidth + bcComboWidth + 5, bcStartY-2, eastValueWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hBCLabels[2] = CreateWindowW(L"Static", L"South:", WS_VISIBLE | WS_CHILD, leftStart + 10, bcStartY + 35, bcLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hSouthBC = CreateWindowW(L"ComboBox", L"",
                                 WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                 leftStart + 10 + bcLabelWidth, bcStartY + 33, bcComboWidth, 60, hwnd, (HMENU)2003, GetModuleHandle(NULL), NULL);
    SendMessageW(g_appData.hSouthBC, CB_ADDSTRING, 0, (LPARAM)getLanguageContext().getBcType(0)); // Dirichlet
    SendMessageW(g_appData.hSouthBC, CB_ADDSTRING, 0, (LPARAM)getLanguageContext().getBcType(1)); // Neumann
    SendMessageW(g_appData.hSouthBC, CB_SETCURSEL, 0, 0);
    g_appData.hSouthValue = CreateWindowW(L"Edit", L"0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + bcLabelWidth + bcComboWidth + 10, bcStartY + 33, westValueWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hBCLabels[3] = CreateWindowW(L"Static", L"North:", WS_VISIBLE | WS_CHILD, eastStartX, bcStartY + 35, bcLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNorthBC = CreateWindowW(L"ComboBox", L"",
                                 WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                 eastStartX + bcLabelWidth, bcStartY + 33, bcComboWidth, 60, hwnd, (HMENU)2004, GetModuleHandle(NULL), NULL);
    SendMessageW(g_appData.hNorthBC, CB_ADDSTRING, 0, (LPARAM)getLanguageContext().getBcType(0)); // Dirichlet
    SendMessageW(g_appData.hNorthBC, CB_ADDSTRING, 0, (LPARAM)getLanguageContext().getBcType(1)); // Neumann
    SendMessageW(g_appData.hNorthBC, CB_SETCURSEL, 0, 0);
    g_appData.hNorthValue = CreateWindowW(L"Edit", L"0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, eastStartX + bcLabelWidth + bcComboWidth + 5, bcStartY + 33, eastValueWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Fifth section: Solution information panel (in group box) - extend to bottom
    int bcSectionHeight = 160; // Height of boundary conditions group box
    int solInfoY = bcY + bcSectionHeight + 10; // Position right after boundary conditions
    int solInfoHeight = height - solInfoY - 50; // Extend to above status bar (leaving space for status bar)

    g_appData.hSolInfoGroup = CreateWindowW(L"Button", getLanguageContext().getSolInfoGroup(),
                                           WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                           leftStart, solInfoY, leftWidth - 10, solInfoHeight, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Create a static control to display solution information
    g_appData.hSolutionInfo = CreateWindowW(L"Edit", L"No solution computed yet. Click 'Solve' to begin.",
                                           WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                                           leftStart + 10, solInfoY + 20, leftWidth - 25, solInfoHeight - 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Right panel: Visualization
    g_appData.hVisualFrame = CreateWindowW(L"Static", L"Solution Visualization",
                                         WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTERIMAGE | SS_CENTER,
                                         rightStart, 10, rightWidth - 20, height - 60, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Subclass the visual frame to handle its painting
    g_pfnOldVisualFrameProc = (WNDPROC)SetWindowLongPtr(g_appData.hVisualFrame, GWLP_WNDPROC, (LONG_PTR)VisualFrameWndProc);
    SetWindowLongPtr(g_appData.hVisualFrame, GWLP_USERDATA, (LONG_PTR)visualizer); // Store IVisualizer pointer

    // Set the visualizer's window handle
    if (visualizer) {
        visualizer->setWindowHandle(g_appData.hVisualFrame);
    }

    // Status bar at the bottom - adjusted for the solution info panel
    g_appData.hStatus = CreateWindowW(L"Static", getLanguageContext().getStatusReady(),
                                    WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTERIMAGE | SS_CENTER,
                                    10, height - 40, width - 20, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
}

void OnSolveButtonClicked(HWND hwnd) {
    // Get values from the UI
    wchar_t buffer[256];

    GetWindowTextW(g_appData.hLxEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    g_appData.Lx = wcstod(buffer, NULL);

    GetWindowTextW(g_appData.hLyEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    g_appData.Ly = wcstod(buffer, NULL);

    GetWindowTextW(g_appData.hNxEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    g_appData.Nx = _wtoi(buffer);

    GetWindowTextW(g_appData.hNyEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    g_appData.Ny = _wtoi(buffer);

    // Get coefficient functions
    GetWindowTextW(g_appData.hA11Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    g_appData.a11Func = wstring_to_utf8(buffer);

    GetWindowTextW(g_appData.hA12Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    g_appData.a12Func = wstring_to_utf8(buffer);

    GetWindowTextW(g_appData.hA22Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    g_appData.a22Func = wstring_to_utf8(buffer);

    GetWindowTextW(g_appData.hB1Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    g_appData.b1Func = wstring_to_utf8(buffer);

    GetWindowTextW(g_appData.hB2Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    g_appData.b2Func = wstring_to_utf8(buffer);

    GetWindowTextW(g_appData.hCEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    g_appData.cFunc = wstring_to_utf8(buffer);

    GetWindowTextW(g_appData.hFEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    g_appData.fFunc = wstring_to_utf8(buffer);

    // Get boundary condition types
    wchar_t bcBuffer[50];
    GetWindowTextW(g_appData.hWestValue, buffer, sizeof(buffer)/sizeof(wchar_t));
    double westVal = wcstod(buffer, NULL);

    GetWindowTextW(g_appData.hEastValue, buffer, sizeof(buffer)/sizeof(wchar_t));
    double eastVal = wcstod(buffer, NULL);

    GetWindowTextW(g_appData.hSouthValue, buffer, sizeof(buffer)/sizeof(wchar_t));
    double southVal = wcstod(buffer, NULL);

    GetWindowTextW(g_appData.hNorthValue, buffer, sizeof(buffer)/sizeof(wchar_t));
    double northVal = wcstod(buffer, NULL);

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
    SetWindowTextW(g_appData.hStatus, L"Solving...");

    try {
        // Use the solution manager to solve the problem
        if (g_solutionManager) {
            bool success = g_solutionManager->solveWithParameters(
                g_appData.Lx, g_appData.Ly, g_appData.Nx, g_appData.Ny,
                g_appData.a11Func, g_appData.a12Func, g_appData.a22Func,
                g_appData.b1Func, g_appData.b2Func, g_appData.cFunc, g_appData.fFunc,
                westBC, eastBC, southBC, northBC,
                westVal, eastVal, southVal, northVal
            );

            if (success) {
                // Get the solution and mesh from the solution manager
                const std::vector<double>& solution = g_solutionManager->getSolution();
                const Mesh& mesh = g_solutionManager->getMesh();

                if (!solution.empty()) {
                    double min_val = *std::min_element(solution.begin(), solution.end());
                    double max_val = *std::max_element(solution.begin(), solution.end());

                    std::wostringstream woss;
                    woss << L"Solution computed: " << solution.size() << L" nodes, "
                         << L"Range: [" << min_val << L", " << max_val << L"]";
                    SetWindowTextW(g_appData.hStatus, utf8_to_wstring(g_solutionManager->getSolutionStatus()).c_str());

                    // Call Visualization Manager to render the solution
                    if (g_visualizationManager) {
                        g_visualizationManager->renderSolution(mesh, solution, g_appData.Nx, g_appData.Ny, "Solution Visualization");

                        // Force a repaint of the visualization window to trigger DirectX rendering
                        if (g_appData.hVisualFrame) {
                            InvalidateRect(g_appData.hVisualFrame, NULL, TRUE);
                            UpdateWindow(g_appData.hVisualFrame);
                        }
                    }

                    // Update the solution information panel with detailed information
                    std::wostringstream solutionInfo;
                    solutionInfo << L"Solution Details:\n";
                    solutionInfo << L"-------------------\n";
                    solutionInfo << L"Nodes: " << solution.size() << L"\n";
                    solutionInfo << L"Elements: " << mesh.elements.size() << L"\n";
                    solutionInfo << L"Domain: [0, " << g_appData.Lx << L"] x [0, " << g_appData.Ly << L"]\n";
                    solutionInfo << L"Mesh: " << g_appData.Nx << L" x " << g_appData.Ny << L" nodes\n";
                    solutionInfo << L"Min value: " << min_val << L"\n";
                    solutionInfo << L"Max value: " << max_val << L"\n";
                    solutionInfo << L"Avg value: " << (solution.empty() ? 0.0 : std::accumulate(solution.begin(), solution.end(), 0.0) / solution.size()) << L"\n\n";
                    solutionInfo << L"Coefficients:\n";
                    solutionInfo << L"  a11(x,y) = " << utf8_to_wstring(g_appData.a11Func) << L"\n";
                    solutionInfo << L"  a12(x,y) = " << utf8_to_wstring(g_appData.a12Func) << L"\n";
                    solutionInfo << L"  a22(x,y) = " << utf8_to_wstring(g_appData.a22Func) << L"\n";
                    solutionInfo << L"  b1(x,y) = " << utf8_to_wstring(g_appData.b1Func) << L"\n";
                    solutionInfo << L"  b2(x,y) = " << utf8_to_wstring(g_appData.b2Func) << L"\n";
                    solutionInfo << L"  c(x,y) = " << utf8_to_wstring(g_appData.cFunc) << L"\n";
                    solutionInfo << L"  f(x,y) = " << utf8_to_wstring(g_appData.fFunc) << L"\n\n";
                    solutionInfo << L"Boundary Conditions:\n";
                    solutionInfo << L"  West: " << ((westBC == "dirichlet") ? L"Dirichlet" : L"Neumann")
                                 << L" (value=" << westVal << L")\n";
                    solutionInfo << L"  East: " << ((eastBC == "dirichlet") ? L"Dirichlet" : L"Neumann")
                                 << L" (value=" << eastVal << L")\n";
                    solutionInfo << L"  South: " << ((southBC == "dirichlet") ? L"Dirichlet" : L"Neumann")
                                 << L" (value=" << southVal << L")\n";
                    solutionInfo << L"  North: " << ((northBC == "dirichlet") ? L"Dirichlet" : L"Neumann")
                                 << L" (value=" << northVal << L")\n\n";
                    solutionInfo << L"Solution computed successfully at: " << __DATE__ << L" " << __TIME__;

                    SetWindowTextW(g_appData.hSolutionInfo, solutionInfo.str().c_str());
                }
            } else {
                MessageBoxW(hwnd, L"Failed to solve the problem.",
                          L"Error", MB_OK | MB_ICONERROR);
                SetWindowTextW(g_appData.hStatus, L"Failed to solve the problem");
            }
        } else {
            MessageBoxW(hwnd, L"Solution manager not initialized.",
                      L"Error", MB_OK | MB_ICONERROR);
            SetWindowTextW(g_appData.hStatus, L"Solution manager not initialized");
        }
    } catch (const std::exception& e) {
        // Convert narrow string to wide string
        std::wstring wide_error = utf8_to_wstring(e.what());

        std::wostringstream woss;
        woss << L"Error: " << wide_error;
        SetWindowTextW(g_appData.hStatus, woss.str().c_str());

        // Also update the solution info panel to show the error
        std::wstring errorInfo = L"Solution Error:\n";
        errorInfo += L"-------------------\n";
        errorInfo += L"An error occurred during solution:\n";
        errorInfo += wide_error;
        SetWindowTextW(g_appData.hSolutionInfo, errorInfo.c_str());
    }
}

void OnResetButtonClicked(HWND hwnd) {
    if (g_resetManager) {
        // Use the reset manager to reset to defaults
        g_resetManager->resetToDefaults();

        // Update the UI controls with the reset values
        SetWindowTextW(g_appData.hLxEdit, L"1.0");
        SetWindowTextW(g_appData.hLyEdit, L"1.0");
        SetWindowTextW(g_appData.hNxEdit, L"20");
        SetWindowTextW(g_appData.hNyEdit, L"20");
        SetWindowTextW(g_appData.hA11Edit, L"1.0");
        SetWindowTextW(g_appData.hA12Edit, L"0.0");
        SetWindowTextW(g_appData.hA22Edit, L"1.0");
        SetWindowTextW(g_appData.hB1Edit, L"0.0");
        SetWindowTextW(g_appData.hB2Edit, L"0.0");
        SetWindowTextW(g_appData.hCEdit, L"0.0");
        SetWindowTextW(g_appData.hFEdit, L"1.0");

        // Reset boundary conditions
        SetWindowTextW(g_appData.hWestValue, L"0.0");
        SetWindowTextW(g_appData.hEastValue, L"0.0");
        SetWindowTextW(g_appData.hSouthValue, L"0.0");
        SetWindowTextW(g_appData.hNorthValue, L"0.0");

        // Reset boundary condition types to Dirichlet
        SendMessageW(g_appData.hWestBC, CB_SETCURSEL, 0, 0);
        SendMessageW(g_appData.hEastBC, CB_SETCURSEL, 0, 0);
        SendMessageW(g_appData.hSouthBC, CB_SETCURSEL, 0, 0);
        SendMessageW(g_appData.hNorthBC, CB_SETCURSEL, 0, 0);

        // Reset preset to default
        SendMessageW(g_appData.hPresetCombo, CB_SETCURSEL, 0, 0);

        SetWindowTextW(g_appData.hStatus, utf8_to_wstring(g_resetManager->getResetStatus()).c_str());
    } else {
        SetWindowTextW(g_appData.hStatus, L"Reset manager not initialized.");
    }
}

void OnExportButtonClicked(HWND hwnd) {
    SetWindowTextW(g_appData.hStatus, L"Exporting results...");

    if (g_exportManager) {
        // Show file dialog to get export filename
        OPENFILENAMEW ofn;
        wchar_t szFile[260] = L"";

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
        ofn.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

        if (GetSaveFileNameW(&ofn)) {
            // Export using the export manager
            std::string filename = wstring_to_utf8(std::wstring(ofn.lpstrFile));
            bool success = g_exportManager->exportSolutionToFile(filename);

            if (success) {
                SetWindowTextW(g_appData.hStatus, utf8_to_wstring(g_exportManager->getExportStatus()).c_str());
                MessageBoxW(hwnd, L"Results exported successfully!", L"Export Complete", MB_OK | MB_ICONINFORMATION);
            } else {
                SetWindowTextW(g_appData.hStatus, utf8_to_wstring(g_exportManager->getExportStatus()).c_str());
                MessageBoxW(hwnd, utf8_to_wstring(g_exportManager->getExportStatus()).c_str(), L"Export Error", MB_OK | MB_ICONERROR);
            }
        } else {
            SetWindowTextW(g_appData.hStatus, L"Export cancelled.");
        }
    } else {
        MessageBoxW(hwnd, L"Export manager not initialized.", L"Error", MB_OK | MB_ICONERROR);
        SetWindowTextW(g_appData.hStatus, L"Export manager not initialized.");
    }
}

void OnPresetChanged(HWND hwnd, int presetIndex) {
    PresetManager::onPresetChanged(hwnd, g_appData, presetIndex);
}

void OnPresetHelpClicked(HWND hwnd) {
    PresetManager::onPresetHelpClicked(hwnd, g_appData);
}


// Function to switch between languages using strategy pattern
void SwitchLanguage() {
    if (getLanguageContext().getLanguageType() == Language::English) {
        getLanguageContext().setStrategy(&getRussianStrategy());
    } else {
        getLanguageContext().setStrategy(&getEnglishStrategy());
    }
}

// Function to update all UI elements with current language strings
void UpdateLanguageStrings(HWND hwnd) {
    // Update window title
    SetWindowTextW(hwnd, (getLanguageContext().getLanguageType() == Language::English) ?
        L"Finite Element Method Solver - Elliptic Equations" :
        L"Решатель методом конечных элементов - Эллиптические уравнения");

    // Update button texts
    HWND hSolveBtn = GetDlgItem(hwnd, 1001);
    HWND hResetBtn = GetDlgItem(hwnd, 1002);
    HWND hExportBtn = GetDlgItem(hwnd, 1003);

    if (hSolveBtn) SetWindowTextW(hSolveBtn, getLanguageContext().getSolveBtn());
    if (hResetBtn) SetWindowTextW(hResetBtn, getLanguageContext().getResetBtn());
    if (hExportBtn) SetWindowTextW(hExportBtn, getLanguageContext().getExportBtn());

    // Update group box titles
    if (g_appData.hCoeffGroup) SetWindowTextW(g_appData.hCoeffGroup, getLanguageContext().getCoeffGroup());
    if (g_appData.hBCGroup) SetWindowTextW(g_appData.hBCGroup, getLanguageContext().getBcGroup());
    if (g_appData.hSolInfoGroup) SetWindowTextW(g_appData.hSolInfoGroup, getLanguageContext().getSolInfoGroup());

    // Update preset combo box label
    if (g_appData.hPresetLabel) SetWindowTextW(g_appData.hPresetLabel, getLanguageContext().getEquationPreset());

    // Update preset combo box options
    if (g_appData.hPresetCombo) {
        // Save current selection
        int currentSel = static_cast<int>(SendMessageW(g_appData.hPresetCombo, CB_GETCURSEL, 0, 0));

        // Clear and re-add items
        SendMessageW(g_appData.hPresetCombo, CB_RESETCONTENT, 0, 0);
        for (int i = 0; i < 7; i++) {
            SendMessageW(g_appData.hPresetCombo, CB_ADDSTRING, 0, (LPARAM)getLanguageContext().getPresetOption(i));
        }

        // Restore selection
        SendMessageW(g_appData.hPresetCombo, CB_SETCURSEL, currentSel, 0);
    }

    // Update boundary condition combo boxes
    HWND bcCombos[] = {g_appData.hWestBC, g_appData.hEastBC, g_appData.hSouthBC, g_appData.hNorthBC};
    for (int i = 0; i < 4; i++) {
        if (bcCombos[i]) {
            // Save current selection
            int currentSel = static_cast<int>(SendMessageW(bcCombos[i], CB_GETCURSEL, 0, 0));

            // Clear and re-add items
            SendMessageW(bcCombos[i], CB_RESETCONTENT, 0, 0);
            for (int j = 0; j < 2; j++) {
                SendMessageW(bcCombos[i], CB_ADDSTRING, 0, (LPARAM)getLanguageContext().getBcType(j));
            }

            // Restore selection
            SendMessageW(bcCombos[i], CB_SETCURSEL, currentSel, 0);
        }
    }

    // Update status bar if it exists
    if (g_appData.hStatus) {
        int currentSel = static_cast<int>(SendMessageW(g_appData.hPresetCombo, CB_GETCURSEL, 0, 0));
        if (currentSel == 0) {
            SetWindowTextW(g_appData.hStatus, getLanguageContext().getStatusReady());
        } else {
            SetWindowTextW(g_appData.hStatus, getLanguageContext().getStatusPresetLoaded());
        }
    }

    // Update the language toggle button to show the opposite language (so user knows which language will be switched to)
    HWND hLangBtn = GetDlgItem(hwnd, 1005);
    if (hLangBtn) {
        SetWindowTextW(hLangBtn, (getLanguageContext().getLanguageType() == Language::English) ? L"RU" : L"EN");
    }

    // Refresh the window to update all controls
    InvalidateRect(hwnd, NULL, TRUE);
}

void LoadPreset(int presetIndex) {
    PresetManager::loadPreset(NULL, g_appData, presetIndex);  // hwnd is not used in the preset manager, so passing NULL

    // Clear any stored solution data and visualization
    if (g_visualizationManager) {
        g_visualizationManager->renderSolution(Mesh(), std::vector<double>(), 0, 0, "");
    }
}

void GUIApp::updateGUIVisualization() {
    if (visualizationManager_ && g_appData.solver) {
        // Get the current solution from the solver
        EllipticApp* ellipticApp = g_appData.solver->getApp();
        if (ellipticApp) {
            const std::vector<double>& solution = ellipticApp->getSolution();
            const Mesh& mesh = ellipticApp->getMesh();

            // Update visualization with the current solution
            visualizationManager_->updateVisualization(g_appData, mesh, solution);

            // Force a repaint of the visualization window to trigger DirectX rendering
            if (g_appData.hVisualFrame) {
                InvalidateRect(g_appData.hVisualFrame, NULL, TRUE);
                UpdateWindow(g_appData.hVisualFrame);
            }
        }
    }
}