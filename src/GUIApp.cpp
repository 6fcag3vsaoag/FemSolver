#include "GUIApp.h"
#include "FemSolver.h"
#include "EllipticApp.h"
#include "GdiVisualizer.h"
#include "StringUtils.h"
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
    HWND hSolutionInfo;  // Solution information display

    // Static text labels
    HWND hPresetLabel;   // "Equation Preset:" label
    HWND hCoeffLabels[7]; // Coefficient labels: a11, a12, a22, b1, b2, c, f
    HWND hMeshLabels[4];  // Mesh labels: Lx, Ly, Nx, Ny
    HWND hBCLabels[4];    // Boundary condition labels: West, East, South, North

    // Group box controls
    HWND hCoeffGroup;    // Coefficient group box
    HWND hBCGroup;       // Boundary condition group box
    HWND hSolInfoGroup;  // Solution info group box

    // Boundary condition combo boxes
    HWND hWestBC, hEastBC, hSouthBC, hNorthBC;

    // Solver instance
    FemSolver* solver;
};

// Language enum
enum class Language {
    English,
    Russian
};

// Interface for language strategy using wide strings
class ILanguageStrategy {
public:
    virtual ~ILanguageStrategy() = default;
    virtual const wchar_t* getEquationPreset() const = 0;
    virtual const wchar_t* getSolveBtn() const = 0;
    virtual const wchar_t* getResetBtn() const = 0;
    virtual const wchar_t* getExportBtn() const = 0;
    virtual const wchar_t* getCoeffGroup() const = 0;
    virtual const wchar_t* getBcGroup() const = 0;
    virtual const wchar_t* getSolInfoGroup() const = 0;
    virtual const wchar_t* getPresetOption(int index) const = 0;
    virtual const wchar_t* getBcType(int index) const = 0;
    virtual const wchar_t* getStatusReady() const = 0;
    virtual const wchar_t* getStatusPresetLoaded() const = 0;
    virtual const wchar_t* getStatusSolved() const = 0;
    virtual Language getLanguageType() const = 0;
};

// English language strategy
class EnglishLanguageStrategy : public ILanguageStrategy {
public:
    const wchar_t* getEquationPreset() const override { return L"Equation Preset:"; }
    const wchar_t* getSolveBtn() const override { return L"Solve"; }
    const wchar_t* getResetBtn() const override { return L"Reset"; }
    const wchar_t* getExportBtn() const override { return L"Export"; }
    const wchar_t* getCoeffGroup() const override { return L"Equation Coefficients"; }
    const wchar_t* getBcGroup() const override { return L"Boundary Conditions"; }
    const wchar_t* getSolInfoGroup() const override { return L"Solution Information"; }
    const wchar_t* getPresetOption(int index) const override {
        static const wchar_t* options[] = {
            L"Select Preset...", L"Laplace Equation", L"Poisson Equation",
            L"Helmholtz Equation", L"Convection-Diffusion", L"Reaction-Diffusion", L"General Elliptic"
        };
        return (index >= 0 && index < 7) ? options[index] : L"";
    }
    const wchar_t* getBcType(int index) const override {
        static const wchar_t* types[] = { L"Dirichlet", L"Neumann" };
        return (index >= 0 && index < 2) ? types[index] : L"";
    }
    const wchar_t* getStatusReady() const override { return L"Ready - Select an equation preset to begin"; }
    const wchar_t* getStatusPresetLoaded() const override { return L"Preset loaded. Ready to solve."; }
    const wchar_t* getStatusSolved() const override { return L"Solution computed successfully!"; }
    Language getLanguageType() const override { return Language::English; }
};

// Russian language strategy
class RussianLanguageStrategy : public ILanguageStrategy {
public:
    const wchar_t* getEquationPreset() const override { return L"Предустановка уравнения:"; }
    const wchar_t* getSolveBtn() const override { return L"Решить"; }
    const wchar_t* getResetBtn() const override { return L"Сброс"; }
    const wchar_t* getExportBtn() const override { return L"Экспорт"; }
    const wchar_t* getCoeffGroup() const override { return L"Коэффициенты уравнения"; }
    const wchar_t* getBcGroup() const override { return L"Граничные условия"; }
    const wchar_t* getSolInfoGroup() const override { return L"Информация о решении"; }
    const wchar_t* getPresetOption(int index) const override {
        static const wchar_t* options[] = {
            L"Выберите предустановку...", L"Уравнение Лапласа", L"Уравнение Пуассона",
            L"Уравнение Гельмгольца", L"Конвективно-диффузионное", L"Реакционно-диффузионное", L"Общее эллиптическое"
        };
        return (index >= 0 && index < 7) ? options[index] : L"";
    }
    const wchar_t* getBcType(int index) const override {
        static const wchar_t* types[] = { L"Дирихле", L"Нейман" };
        return (index >= 0 && index < 2) ? types[index] : L"";
    }
    const wchar_t* getStatusReady() const override { return L"Готово - Выберите предустановку уравнения для начала"; }
    const wchar_t* getStatusPresetLoaded() const override { return L"Предустановка загружена. Готов к решению."; }
    const wchar_t* getStatusSolved() const override { return L"Решение вычислено успешно!"; }
    Language getLanguageType() const override { return Language::Russian; }
};

// Context for language strategy
class LanguageContext {
private:
    ILanguageStrategy* strategy;
public:
    LanguageContext(ILanguageStrategy* s) : strategy(s) {}
    void setStrategy(ILanguageStrategy* s) { strategy = s; }
    const wchar_t* getEquationPreset() const { return strategy->getEquationPreset(); }
    const wchar_t* getSolveBtn() const { return strategy->getSolveBtn(); }
    const wchar_t* getResetBtn() const { return strategy->getResetBtn(); }
    const wchar_t* getExportBtn() const { return strategy->getExportBtn(); }
    const wchar_t* getCoeffGroup() const { return strategy->getCoeffGroup(); }
    const wchar_t* getBcGroup() const { return strategy->getBcGroup(); }
    const wchar_t* getSolInfoGroup() const { return strategy->getSolInfoGroup(); }
    const wchar_t* getPresetOption(int index) const { return strategy->getPresetOption(index); }
    const wchar_t* getBcType(int index) const { return strategy->getBcType(index); }
    const wchar_t* getStatusReady() const { return strategy->getStatusReady(); }
    const wchar_t* getStatusPresetLoaded() const { return strategy->getStatusPresetLoaded(); }
    const wchar_t* getStatusSolved() const { return strategy->getStatusSolved(); }
    Language getLanguageType() const { return strategy->getLanguageType(); }
};

// Global language context using strategy pattern
EnglishLanguageStrategy englishStrategy;
LanguageContext langContext(&englishStrategy);

AppData g_appData;
WNDPROC g_pfnOldVisualFrameProc = NULL;
GdiVisualizer* g_currentGdiVisualizer = nullptr; // TEMPORARY: For global access until refactor

// Forward function declarations
LRESULT CALLBACK VisualFrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateControls(HWND hwnd, GdiVisualizer* visualizer);
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
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect);

            // Get the GdiVisualizer instance from window user data
            GdiVisualizer* visualizer = reinterpret_cast<GdiVisualizer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (visualizer) {
                visualizer->drawInternal(hdc, rect);
            } else {
                // Fallback drawing if visualizer not set up (shouldn't happen)
                HBRUSH bgBrush = CreateSolidBrush(RGB(240, 240, 240));
                FillRect(hdc, &rect, bgBrush);
                DeleteObject(bgBrush);
                SetTextColor(hdc, RGB(128, 128, 128));
                std::wstring placeholder = L"Visualizer not initialized.";
                RECT placeholderRect = {10, 10, rect.right - 10, rect.bottom - 10};
                DrawTextW(hdc, placeholder.c_str(), -1, &placeholderRect, DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX);
            }

            EndPaint(hwnd, &ps);
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

    // Initialize GDI Visualizer
    gdiVisualizer_ = std::make_unique<GdiVisualizer>();
    g_currentGdiVisualizer = gdiVisualizer_.get(); // TEMPORARY: Make it globally accessible
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
    switch(msg) {
        case WM_CREATE:
            CreateControls(hwnd, g_currentGdiVisualizer); // Pass the global GdiVisualizer instance
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
                case 1005: // Language toggle button
                    SwitchLanguage();
                    UpdateLanguageStrings(hwnd);
                    break;
                case 1006: // Preset help button
                    OnPresetHelpClicked(hwnd);
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

        case WM_GETMINMAXINFO:
            {
                LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
                lpMMI->ptMinTrackSize.x = 1000;  // Minimum width for proper layout
                lpMMI->ptMinTrackSize.y = 800;   // Increased minimum height for proper layout with extended solution panel
            }
            break;

        case WM_SIZE:
            {
                // Handle window resize to make UI responsive
                int newWidth = LOWORD(lParam);
                int newHeight = HIWORD(lParam);

                // Recalculate dimensions
                int leftWidth = static_cast<int>(newWidth * 0.35); // 35% for controls
                int rightWidth = static_cast<int>(newWidth * 0.60); // 60% for visualization
                int margin = static_cast<int>(newWidth * 0.025); // Responsive margin
                int leftStart = margin;
                int rightStart = leftStart + leftWidth + margin;

                // Resize and reposition controls
                if (g_appData.hStatus) {
                    MoveWindow(g_appData.hStatus, 10, newHeight - 40, newWidth - 20, 30, TRUE);
                }

                // Resize and reposition visualization frame (right panel)
                if (g_appData.hVisualFrame) {
                    // Calculate visualization height, leaving space for status bar and some margin at the bottom
                    int visHeight = newHeight - 60; // Leave 60px for status bar and margins
                    MoveWindow(g_appData.hVisualFrame, rightStart, 10, rightWidth - 20, visHeight, TRUE);
                }
                
                // Reposition Preset controls
                int presetLabelWidth = static_cast<int>(leftWidth * 0.28);
                int helpButtonWidth = 30;
                int comboWidth = leftWidth - presetLabelWidth - helpButtonWidth - 15;

                if (g_appData.hPresetLabel) {
                    MoveWindow(g_appData.hPresetLabel, leftStart, 10, presetLabelWidth, 25, TRUE);
                }
                if (g_appData.hPresetCombo) {
                    MoveWindow(g_appData.hPresetCombo, leftStart + presetLabelWidth, 8, comboWidth, 150, TRUE);
                }
                HWND hHelpBtn = GetDlgItem(hwnd, 1006);
                if (hHelpBtn) {
                    MoveWindow(hHelpBtn, leftStart + presetLabelWidth + comboWidth + 5, 8, helpButtonWidth, 28, TRUE);
                }


                // Resize the buttons - use dynamic width calculation based on text length
                int buttonY = 45;
                int buttonHeight = 30;
                int buttonSpacing = 2; // Fixed 2px spacing
                int totalButtons = 5;
                int buttonMargin = 10; // Left and right margin for buttons

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
                GetTextExtentPoint32W(hdc, langContext.getSolveBtn(), wcslen(langContext.getSolveBtn()), &textSize);
                solveWidth = std::max<int>(50, textSize.cx + 16); // Add padding

                // Calculate text width for Reset button
                GetTextExtentPoint32W(hdc, langContext.getResetBtn(), wcslen(langContext.getResetBtn()), &textSize);
                resetWidth = std::max<int>(50, textSize.cx + 16); // Add padding

                // Calculate text width for Export button
                GetTextExtentPoint32W(hdc, langContext.getExportBtn(), wcslen(langContext.getExportBtn()), &textSize);
                exportWidth = std::max<int>(50, textSize.cx + 16); // Add padding

                // Calculate text width for Language Toggle button
                const wchar_t* langToggleText = (langContext.getLanguageType() == Language::English) ? L"RU" : L"EN";
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

                HWND hSolveBtn = GetDlgItem(hwnd, 1001);
                HWND hResetBtn = GetDlgItem(hwnd, 1002);
                HWND hExportBtn = GetDlgItem(hwnd, 1003);
                HWND hLangBtn = GetDlgItem(hwnd, 1005);

                if (hSolveBtn) MoveWindow(hSolveBtn, currentX, buttonY, solveWidth, buttonHeight, TRUE);
                currentX += solveWidth + buttonSpacing;
                if (hResetBtn) MoveWindow(hResetBtn, currentX, buttonY, resetWidth, buttonHeight, TRUE);
                currentX += resetWidth + buttonSpacing;
                if (hExportBtn) MoveWindow(hExportBtn, currentX, buttonY, exportWidth, buttonHeight, TRUE);
                currentX += exportWidth + buttonSpacing;
                if (hLangBtn) MoveWindow(hLangBtn, currentX, buttonY, langWidth, buttonHeight, TRUE);

                // Calculate positions for group boxes with proper spacing
                int coeffY = 110;  // Fixed position to match CreateControls
                int coeffSectionHeight = 200;  // Fixed height for coefficients
                int bcY = coeffY + coeffSectionHeight + 10;  // Boundary conditions after coeff section with margin
                int bcSectionHeight = 160;  // Fixed height for boundary conditions
                int solInfoY = bcY + bcSectionHeight + 10;  // Solution info after boundary conditions
                int solInfoHeight = newHeight - solInfoY - 50;  // Extend to above status bar (leaving space for status bar)

                // Move and resize group boxes
                if (g_appData.hCoeffGroup) {
                    MoveWindow(g_appData.hCoeffGroup, leftStart, coeffY, leftWidth - 10, coeffSectionHeight, TRUE);
                }
                if (g_appData.hBCGroup) {
                    MoveWindow(g_appData.hBCGroup, leftStart, bcY, leftWidth - 10, bcSectionHeight, TRUE);
                }
                if (g_appData.hSolInfoGroup) {
                    MoveWindow(g_appData.hSolInfoGroup, leftStart, solInfoY, leftWidth - 10, solInfoHeight, TRUE);
                }

                // Move the solution info text control inside its group
                if (g_appData.hSolutionInfo) {
                    MoveWindow(g_appData.hSolutionInfo, leftStart + 10, solInfoY + 20, leftWidth - 25, solInfoHeight - 30, TRUE);
                }

                // Now resize and reposition all static labels and controls within the coefficient section
                int coeffStartY = coeffY + 20;
                int coeffLabelWidth = static_cast<int>(leftWidth * 0.15);  // Reduced width for labels (like "a11(x,y):")
                int coeffFieldWidth = static_cast<int>(leftWidth * 0.28);  // Reduced width for text fields
                int coeffSpacingX = coeffLabelWidth + 5;  // Reduced spacing between label and field

                // Resize coefficient labels
                if (g_appData.hCoeffLabels[0]) MoveWindow(g_appData.hCoeffLabels[0], leftStart + 10, coeffStartY, coeffLabelWidth, 20, TRUE); // a11
                if (g_appData.hCoeffLabels[1]) MoveWindow(g_appData.hCoeffLabels[1], leftStart + coeffSpacingX + coeffFieldWidth + 10, coeffStartY, coeffLabelWidth, 20, TRUE); // a12
                if (g_appData.hCoeffLabels[2]) MoveWindow(g_appData.hCoeffLabels[2], leftStart + 10, coeffStartY + 30, coeffLabelWidth, 20, TRUE); // a22
                if (g_appData.hCoeffLabels[3]) MoveWindow(g_appData.hCoeffLabels[3], leftStart + coeffSpacingX + coeffFieldWidth + 10, coeffStartY + 30, coeffLabelWidth, 20, TRUE); // b1
                if (g_appData.hCoeffLabels[4]) MoveWindow(g_appData.hCoeffLabels[4], leftStart + 10, coeffStartY + 60, coeffLabelWidth, 20, TRUE); // b2
                if (g_appData.hCoeffLabels[5]) MoveWindow(g_appData.hCoeffLabels[5], leftStart + coeffSpacingX + coeffFieldWidth + 10, coeffStartY + 60, coeffLabelWidth, 20, TRUE); // c
                if (g_appData.hCoeffLabels[6]) MoveWindow(g_appData.hCoeffLabels[6], leftStart + 10, coeffStartY + 90, coeffLabelWidth, 20, TRUE); // f

                // Resize coefficient input fields
                if (g_appData.hA11Edit) MoveWindow(g_appData.hA11Edit, leftStart + coeffSpacingX, coeffStartY - 2, coeffFieldWidth, 22, TRUE);
                if (g_appData.hA12Edit) MoveWindow(g_appData.hA12Edit, leftStart + coeffSpacingX + coeffFieldWidth + coeffLabelWidth + 10, coeffStartY - 2, coeffFieldWidth, 22, TRUE);
                if (g_appData.hA22Edit) MoveWindow(g_appData.hA22Edit, leftStart + coeffSpacingX, coeffStartY + 28, coeffFieldWidth, 22, TRUE);
                if (g_appData.hB1Edit) MoveWindow(g_appData.hB1Edit, leftStart + coeffSpacingX + coeffFieldWidth + coeffLabelWidth + 10, coeffStartY + 28, coeffFieldWidth, 22, TRUE);
                if (g_appData.hB2Edit) MoveWindow(g_appData.hB2Edit, leftStart + coeffSpacingX, coeffStartY + 58, coeffFieldWidth, 22, TRUE);
                if (g_appData.hCEdit) MoveWindow(g_appData.hCEdit, leftStart + coeffSpacingX + coeffFieldWidth + coeffLabelWidth + 10, coeffStartY + 58, coeffFieldWidth, 22, TRUE);
                if (g_appData.hFEdit) MoveWindow(g_appData.hFEdit, leftStart + coeffSpacingX, coeffStartY + 88, coeffFieldWidth * 2 + coeffLabelWidth, 22, TRUE);

                // Resize mesh labels and parameters
                int meshLabelWidth = static_cast<int>(leftWidth * 0.06);  // Reduced width for labels like "Lx:"
                int meshFieldWidth = static_cast<int>(leftWidth * 0.10);  // Reduced width for text fields
                int meshSpacing = static_cast<int>(leftWidth * 0.02);     // Reduced spacing between elements

                if (g_appData.hMeshLabels[0]) MoveWindow(g_appData.hMeshLabels[0], leftStart + 10, coeffStartY + 125, meshLabelWidth, 20, TRUE);  // Lx
                if (g_appData.hLxEdit) MoveWindow(g_appData.hLxEdit, leftStart + 10 + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, TRUE);
                if (g_appData.hMeshLabels[1]) MoveWindow(g_appData.hMeshLabels[1], leftStart + 10 + meshLabelWidth + meshFieldWidth + meshSpacing, coeffStartY + 125, meshLabelWidth, 20, TRUE);  // Ly
                if (g_appData.hLyEdit) MoveWindow(g_appData.hLyEdit, leftStart + 10 + meshLabelWidth + meshFieldWidth + meshSpacing + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, TRUE);
                if (g_appData.hMeshLabels[2]) MoveWindow(g_appData.hMeshLabels[2], leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 2, coeffStartY + 125, meshLabelWidth, 20, TRUE);  // Nx
                if (g_appData.hNxEdit) MoveWindow(g_appData.hNxEdit, leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 2 + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, TRUE);
                if (g_appData.hMeshLabels[3]) MoveWindow(g_appData.hMeshLabels[3], leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 3, coeffStartY + 125, meshLabelWidth, 20, TRUE);  // Ny
                if (g_appData.hNyEdit) MoveWindow(g_appData.hNyEdit, leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 3 + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, TRUE);

                // Resize boundary condition labels and controls
                int bcStartY = bcY + 20;
                int bcLabelWidth = static_cast<int>(leftWidth * 0.08);  // Reduced width for labels like "West:"
                int bcComboWidth = static_cast<int>(leftWidth * 0.18);  // Reduced combo box width
                int bcValueWidth = static_cast<int>(leftWidth * 0.18);  // Reduced value field width

                // Calculate positions for east/south controls to avoid overflow
                int eastStartX = leftStart + static_cast<int>(leftWidth * 0.5); // Position on the right half of the group box
                int eastSpacing = 5; // Reduced spacing

                // West boundary controls
                if (g_appData.hBCLabels[0]) MoveWindow(g_appData.hBCLabels[0], leftStart + 10, bcStartY, bcLabelWidth, 20, TRUE); // West
                if (g_appData.hWestBC) MoveWindow(g_appData.hWestBC, leftStart + 10 + bcLabelWidth, bcStartY-2, bcComboWidth, 60, TRUE);
                // Ensure West value field fits within the left half
                int westValueWidth = eastStartX - (leftStart + 10 + bcLabelWidth + bcComboWidth + 10) - 5;
                if (westValueWidth > 0) {
                    if (g_appData.hWestValue) MoveWindow(g_appData.hWestValue, leftStart + 10 + bcLabelWidth + bcComboWidth + 10, bcStartY-2, westValueWidth, 22, TRUE);
                }

                // East boundary controls
                if (g_appData.hBCLabels[1]) MoveWindow(g_appData.hBCLabels[1], eastStartX, bcStartY, bcLabelWidth, 20, TRUE); // East
                if (g_appData.hEastBC) MoveWindow(g_appData.hEastBC, eastStartX + bcLabelWidth, bcStartY-2, bcComboWidth, 60, TRUE);
                // Calculate available space for East value field to prevent overflow
                int eastValueWidth = leftWidth - 15 - (eastStartX + bcLabelWidth + bcComboWidth + 5);
                if (eastValueWidth > 0) {
                    if (g_appData.hEastValue) MoveWindow(g_appData.hEastValue, eastStartX + bcLabelWidth + bcComboWidth + 5, bcStartY-2, eastValueWidth, 22, TRUE);
                }

                // South boundary controls
                if (g_appData.hBCLabels[2]) MoveWindow(g_appData.hBCLabels[2], leftStart + 10, bcStartY + 35, bcLabelWidth, 20, TRUE); // South
                if (g_appData.hSouthBC) MoveWindow(g_appData.hSouthBC, leftStart + 10 + bcLabelWidth, bcStartY + 33, bcComboWidth, 60, TRUE);
                if (westValueWidth > 0) {
                    if (g_appData.hSouthValue) MoveWindow(g_appData.hSouthValue, leftStart + 10 + bcLabelWidth + bcComboWidth + 10, bcStartY + 33, westValueWidth, 22, TRUE);
                }

                // North boundary controls
                if (g_appData.hBCLabels[3]) MoveWindow(g_appData.hBCLabels[3], eastStartX, bcStartY + 35, bcLabelWidth, 20, TRUE); // North
                if (g_appData.hNorthBC) MoveWindow(g_appData.hNorthBC, eastStartX + bcLabelWidth, bcStartY + 33, bcComboWidth, 60, TRUE);
                if (eastValueWidth > 0) {
                    if (g_appData.hNorthValue) MoveWindow(g_appData.hNorthValue, eastStartX + bcLabelWidth + bcComboWidth + 5, bcStartY + 33, eastValueWidth, 22, TRUE);
                }
            }
            break;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);
                EndPaint(hwnd, &ps);
                return 0;
            }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void CreateControls(HWND hwnd, GdiVisualizer* visualizer) {
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

    g_appData.hPresetLabel = CreateWindowW(L"Static", langContext.getEquationPreset(), WS_VISIBLE | WS_CHILD, leftStart, 10, presetLabelWidth, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hPresetCombo = CreateWindowW(L"ComboBox", L"",
                                          WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                          leftStart + presetLabelWidth + 5, 8, comboWidth, 150, hwnd, (HMENU)2000, GetModuleHandle(NULL), NULL);

    // Add Help button
    CreateWindowW(L"Button", L"?", WS_VISIBLE | WS_CHILD,
                  leftStart + presetLabelWidth + comboWidth + 10, 8, helpButtonWidth, 28, hwnd, (HMENU)1006, GetModuleHandle(NULL), NULL);


    // Add preset options based on current language
    for (int i = 0; i < 7; i++) {
        SendMessageW(g_appData.hPresetCombo, CB_ADDSTRING, 0, (LPARAM)langContext.getPresetOption(i));
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
    GetTextExtentPoint32W(hdc, langContext.getSolveBtn(), wcslen(langContext.getSolveBtn()), &textSize);
    solveWidth = std::max<int>(50, textSize.cx + 16); // Add padding

    // Calculate text width for Reset button
    GetTextExtentPoint32W(hdc, langContext.getResetBtn(), wcslen(langContext.getResetBtn()), &textSize);
    resetWidth = std::max<int>(50, textSize.cx + 16); // Add padding

    // Calculate text width for Export button
    GetTextExtentPoint32W(hdc, langContext.getExportBtn(), wcslen(langContext.getExportBtn()), &textSize);
    exportWidth = std::max<int>(50, textSize.cx + 16); // Add padding

    // Calculate text width for Language Toggle button
    const wchar_t* langToggleText = (langContext.getLanguageType() == Language::English) ? L"RU" : L"EN";
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
    CreateWindowW(L"Button", langContext.getSolveBtn(), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 currentX, buttonY, solveWidth, buttonHeight, hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);

    // Create Reset button
    currentX += solveWidth + buttonSpacing;
    CreateWindowW(L"Button", langContext.getResetBtn(), WS_VISIBLE | WS_CHILD,
                 currentX, buttonY, resetWidth, buttonHeight, hwnd, (HMENU)1002, GetModuleHandle(NULL), NULL);

    // Create Export button
    currentX += resetWidth + buttonSpacing;
    CreateWindowW(L"Button", langContext.getExportBtn(), WS_VISIBLE | WS_CHILD,
                 currentX, buttonY, exportWidth, buttonHeight, hwnd, (HMENU)1003, GetModuleHandle(NULL), NULL);

    // Create Language Toggle button
    currentX += exportWidth + buttonSpacing;
    CreateWindowW(L"Button", langToggleText,
                 WS_VISIBLE | WS_CHILD,
                 currentX, buttonY, langWidth, buttonHeight, hwnd, (HMENU)1005, GetModuleHandle(NULL), NULL);

    // Third section: Equation coefficients (in group box) - with proper spacing
    int coeffY = buttonY + buttonHeight + 10;  // Position just below the buttons with appropriate spacing
    g_appData.hCoeffGroup = CreateWindowW(L"Button", langContext.getCoeffGroup(),
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
    g_appData.hBCGroup = CreateWindowW(L"Button", langContext.getBcGroup(),
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
    SendMessageW(g_appData.hWestBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(0)); // Dirichlet
    SendMessageW(g_appData.hWestBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(1)); // Neumann
    SendMessageW(g_appData.hWestBC, CB_SETCURSEL, 0, 0);
    // Calculate available space for West value field to prevent overflow
    int westValueWidth = eastStartX - (leftStart + 10 + bcLabelWidth + bcComboWidth + 10) - 5;
    g_appData.hWestValue = CreateWindowW(L"Edit", L"0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + bcLabelWidth + bcComboWidth + 10, bcStartY-2, westValueWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hBCLabels[1] = CreateWindowW(L"Static", L"East:", WS_VISIBLE | WS_CHILD, eastStartX, bcStartY, bcLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hEastBC = CreateWindowW(L"ComboBox", L"",
                                WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                eastStartX + bcLabelWidth, bcStartY-2, bcComboWidth, 60, hwnd, (HMENU)2002, GetModuleHandle(NULL), NULL);
    SendMessageW(g_appData.hEastBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(0)); // Dirichlet
    SendMessageW(g_appData.hEastBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(1)); // Neumann
    SendMessageW(g_appData.hEastBC, CB_SETCURSEL, 0, 0);
    // Calculate available space for East value field to prevent overflow
    int eastValueWidth = leftWidth - 15 - (eastStartX + bcLabelWidth + bcComboWidth + 5);
    g_appData.hEastValue = CreateWindowW(L"Edit", L"0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, eastStartX + bcLabelWidth + bcComboWidth + 5, bcStartY-2, eastValueWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hBCLabels[2] = CreateWindowW(L"Static", L"South:", WS_VISIBLE | WS_CHILD, leftStart + 10, bcStartY + 35, bcLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hSouthBC = CreateWindowW(L"ComboBox", L"",
                                 WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                 leftStart + 10 + bcLabelWidth, bcStartY + 33, bcComboWidth, 60, hwnd, (HMENU)2003, GetModuleHandle(NULL), NULL);
    SendMessageW(g_appData.hSouthBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(0)); // Dirichlet
    SendMessageW(g_appData.hSouthBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(1)); // Neumann
    SendMessageW(g_appData.hSouthBC, CB_SETCURSEL, 0, 0);
    g_appData.hSouthValue = CreateWindowW(L"Edit", L"0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + bcLabelWidth + bcComboWidth + 10, bcStartY + 33, westValueWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hBCLabels[3] = CreateWindowW(L"Static", L"North:", WS_VISIBLE | WS_CHILD, eastStartX, bcStartY + 35, bcLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNorthBC = CreateWindowW(L"ComboBox", L"",
                                 WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                 eastStartX + bcLabelWidth, bcStartY + 33, bcComboWidth, 60, hwnd, (HMENU)2004, GetModuleHandle(NULL), NULL);
    SendMessageW(g_appData.hNorthBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(0)); // Dirichlet
    SendMessageW(g_appData.hNorthBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(1)); // Neumann
    SendMessageW(g_appData.hNorthBC, CB_SETCURSEL, 0, 0);
    g_appData.hNorthValue = CreateWindowW(L"Edit", L"0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, eastStartX + bcLabelWidth + bcComboWidth + 5, bcStartY + 33, eastValueWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Fifth section: Solution information panel (in group box) - extend to bottom
    int bcSectionHeight = 160; // Height of boundary conditions group box
    int solInfoY = bcY + bcSectionHeight + 10; // Position right after boundary conditions
    int solInfoHeight = height - solInfoY - 50; // Extend to above status bar (leaving space for status bar)

    g_appData.hSolInfoGroup = CreateWindowW(L"Button", langContext.getSolInfoGroup(),
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
    SetWindowLongPtr(g_appData.hVisualFrame, GWLP_USERDATA, (LONG_PTR)visualizer); // Store GdiVisualizer pointer

    // Set the visualizer's window handle
    visualizer->setWindowHandle(g_appData.hVisualFrame);

    // Status bar at the bottom - adjusted for the solution info panel
    g_appData.hStatus = CreateWindowW(L"Static", langContext.getStatusReady(),
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
                SetWindowTextW(g_appData.hStatus, L"Solution computed successfully!");

                // Show solution information
                const std::vector<double>& solution = ellipticApp->getSolution();
                const Mesh& mesh = ellipticApp->getMesh();

                if (!solution.empty()) {
                    double min_val = *std::min_element(solution.begin(), solution.end());
                    double max_val = *std::max_element(solution.begin(), solution.end());

                    std::wostringstream woss;
                    woss << L"Solution computed: " << solution.size() << L" nodes, "
                         << L"Range: [" << min_val << L", " << max_val << L"]";
                    SetWindowTextW(g_appData.hStatus, woss.str().c_str());

                    // Call GDI Visualizer to render the solution
                    if (g_currentGdiVisualizer) {
                        g_currentGdiVisualizer->render(mesh, solution, g_appData.Nx, g_appData.Ny, "Solution Visualization");
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
                MessageBoxW(hwnd, L"Failed to access solver application.",
                          L"Error", MB_OK | MB_ICONERROR);
            }
        } else {
            MessageBoxW(hwnd, L"Solver not initialized. The application should be run from the main FemSolver instance.",
                      L"Error", MB_OK | MB_ICONERROR);
            SetWindowTextW(g_appData.hStatus, L"Solver not initialized");
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
    // Reset to default values
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

    SetWindowTextW(g_appData.hStatus, L"Reset to defaults");
}

void OnExportButtonClicked(HWND hwnd) {
    SetWindowTextW(g_appData.hStatus, L"Exporting results...");

    if (g_appData.solver) {
        EllipticApp* ellipticApp = g_appData.solver->getApp();
        if (ellipticApp) {
            const std::vector<double>& solution = ellipticApp->getSolution();
            const Mesh& mesh = ellipticApp->getMesh();

            if (!solution.empty()) {
                // Export solution data to a file
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
                    // Open file in binary mode to ensure UTF-8 output without locale interference
                    std::ofstream outFile(ofn.lpstrFile, std::ios_base::binary);
                    if (outFile.is_open()) {
                        // Write UTF-8 BOM
                        outFile.write("\xEF\xBB\xBF", 3);

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

                        SetWindowTextW(g_appData.hStatus, L"Results successfully exported!");
                        MessageBoxW(hwnd, L"Results exported successfully to:\n", L"Export Complete", MB_OK | MB_ICONINFORMATION);
                    } else {
                        SetWindowTextW(g_appData.hStatus, L"Error: Could not open file for export.");
                        MessageBoxW(hwnd, L"Error opening file for export.", L"Export Error", MB_OK | MB_ICONERROR);
                    }
                } else {
                    SetWindowTextW(g_appData.hStatus, L"Export cancelled.");
                }
            } else {
                MessageBoxW(hwnd, L"No solution data available to export. Please solve the problem first.",
                          L"No Data", MB_OK | MB_ICONINFORMATION);
                SetWindowTextW(g_appData.hStatus, L"No solution data to export. Solve first.");
            }
        } else {
            MessageBoxW(hwnd, L"Cannot access solver application.", L"Error", MB_OK | MB_ICONERROR);
            SetWindowTextW(g_appData.hStatus, L"Cannot access solver application.");
        }
    } else {
        MessageBoxW(hwnd, L"Solver not initialized.", L"Error", MB_OK | MB_ICONERROR);
        SetWindowTextW(g_appData.hStatus, L"Solver not initialized.");
    }
}

void OnPresetChanged(HWND hwnd, int presetIndex) {
    if (presetIndex > 0) { // Skip the "Select Preset..." option
        LoadPreset(presetIndex);
        SetWindowTextW(g_appData.hStatus, langContext.getStatusPresetLoaded());

        // Update the solution info panel to show current values
        wchar_t buffer[256];
        std::wostringstream solutionInfo;
        solutionInfo << L"Current Configuration:\n";
        solutionInfo << L"-------------------\n";

        // Get current values from UI
        GetWindowTextW(g_appData.hLxEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"Domain X: [0, " << buffer << L"]\n";
        GetWindowTextW(g_appData.hLyEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"Domain Y: [0, " << buffer << L"]\n";
        GetWindowTextW(g_appData.hNxEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"X nodes: " << buffer << L"\n";
        GetWindowTextW(g_appData.hNyEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"Y nodes: " << buffer << L"\n\n";

        solutionInfo << L"Coefficients:\n";
        GetWindowTextW(g_appData.hA11Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  a11(x,y) = " << buffer << L"\n";
        GetWindowTextW(g_appData.hA12Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  a12(x,y) = " << buffer << L"\n";
        GetWindowTextW(g_appData.hA22Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  a22(x,y) = " << buffer << L"\n";
        GetWindowTextW(g_appData.hB1Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  b1(x,y) = " << buffer << L"\n";
        GetWindowTextW(g_appData.hB2Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  b2(x,y) = " << buffer << L"\n";
        GetWindowTextW(g_appData.hCEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  c(x,y) = " << buffer << L"\n";
        GetWindowTextW(g_appData.hFEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  f(x,y) = " << buffer << L"\n";

        SetWindowTextW(g_appData.hSolutionInfo, solutionInfo.str().c_str());

        // Force refresh of all coefficient controls to ensure visual update
        UpdateWindow(g_appData.hLxEdit);
        UpdateWindow(g_appData.hLyEdit);
        UpdateWindow(g_appData.hNxEdit);
        UpdateWindow(g_appData.hNyEdit);
        UpdateWindow(g_appData.hA11Edit);
        UpdateWindow(g_appData.hA12Edit);
        UpdateWindow(g_appData.hA22Edit);
        UpdateWindow(g_appData.hB1Edit);
        UpdateWindow(g_appData.hB2Edit);
        UpdateWindow(g_appData.hCEdit);
        UpdateWindow(g_appData.hFEdit);
        UpdateWindow(g_appData.hWestValue);
        UpdateWindow(g_appData.hEastValue);
        UpdateWindow(g_appData.hSouthValue);
        UpdateWindow(g_appData.hNorthValue);
        UpdateWindow(g_appData.hSolutionInfo);

        // Force a refresh of the visualization area to show the new state
        InvalidateRect(g_appData.hVisualFrame, NULL, TRUE);
        UpdateWindow(g_appData.hVisualFrame);
    }
}

// Structure to hold preset data
struct PresetData {
    const wchar_t* name;
    const wchar_t* description_en;
    const wchar_t* description_ru;
    const wchar_t* lx, * ly, * nx, * ny;
    const wchar_t* a11, * a12, * a22, * b1, * b2, * c, * f;
    const wchar_t* westValue, * eastValue, * southValue, * northValue;
};

// Define all presets in a structured way
const PresetData PRESETS[] = {
    { L"Laplace Equation",
      L"--- PHYSICAL INTERPRETATION ---\n" 
      L"Describes steady-state phenomena without sources or sinks, such as:\n" 
      L"- Temperature distribution in a stationary state.\n" 
      L"- Electrostatic potential in a charge-free region.\n" 
      L"- Potential of an ideal incompressible fluid flow.\n\n" 
      L"--- EQUATION ---\n" 
      L"∇²u = ∂²u/∂x² + ∂²u/∂y² = 0\n\n" 
      L"--- PRESET PARAMETERS ---\n" 
      L"This preset models heat distribution on a unit plate where the boundary temperature is defined by the function u(x,y) = x² + y².",
      L"--- ФИЗИЧЕСКАЯ ИНТЕРПРЕТАЦИЯ ---\n" 
      L"Описывает стационарные явления без источников или стоков, такие как:\n" 
      L"- Распределение температуры в стационарном состоянии.\n" 
      L"- Электростатический потенциал в области без зарядов.\n" 
      L"- Потенциал идеального течения несжимаемой жидкости.\n\n" 
      L"--- УРАВНЕНИЕ ---\n" 
      L"∇²u = ∂²u/∂x² + ∂²u/∂y² = 0\n\n" 
      L"--- ПАРАМЕТРЫ ПРЕДУСТАНОВКИ ---\n" 
      L"Эта предустановка моделирует распределение тепла на единичной пластине, где температура на границе задана функцией u(x,y) = x² + y².",
      L"1.0", L"1.0", L"20", L"20",
      L"1.0", L"0.0", L"1.0", L"0.0", L"0.0", L"0.0", L"0.0",
      L"y*y", L"1+y*y", L"x*x", L"1+x*x"
    },
    { L"Poisson Equation",
      L"--- PHYSICAL INTERPRETATION ---\n" 
      L"Describes phenomena with a specified source or sink term 'f', such as:\n" 
      L"- Heat distribution with an internal heat source.\n" 
      L"- Electrostatic potential in the presence of a charge distribution.\n\n" 
      L"--- EQUATION ---\n" 
      L"-∇²u = f(x,y)\n\n" 
      L"--- PRESET PARAMETERS ---\n" 
      L"This preset has a known analytical solution u(x,y) = sin(2πx)sin(2πy) for the source f(x,y) = 8π²sin(2πx)sin(2πy). The boundary conditions are set to zero. The numerical solution can be compared to the analytical one to verify the solver's accuracy.",
      L"--- ФИЗИЧЕСКАЯ ИНТЕРПРЕТАЦИЯ ---\n" 
      L"Описывает явления с заданным источником или стоком 'f', такие как:\n" 
      L"- Распределение тепла с внутренним источником тепла.\n" 
      L"- Электростатический потенциал при наличии распределения заряда.\n\n" 
      L"--- УРАВНЕНИЕ ---\n" 
      L"-∇²u = f(x,y)\n\n" 
      L"--- ПАРАМЕТРЫ ПРЕДУСТАНОВКИ ---\n" 
      L"Эта предустановка имеет известное аналитическое решение u(x,y) = sin(2πx)sin(2πy) для источника f(x,y) = 8π²sin(2πx)sin(2πy). Граничные условия нулевые. Численное решение можно сравнить с аналитическим для проверки точности решателя.",
      L"1.0", L"1.0", L"20", L"20",
      L"1.0", L"0.0", L"1.0", L"0.0", L"0.0", L"0.0", L"8*pi*pi*sin(2*pi*x)*sin(2*pi*y)",
      L"0.0", L"0.0", L"0.0", L"0.0"
    },
    { L"Helmholtz Equation",
      L"--- PHYSICAL INTERPRETATION ---\n" 
      L"Arises in physics when studying wave phenomena, vibrations, or diffusion-reaction processes. It is often called the 'reaction-diffusion' equation. Examples:\n" 
      L"- Time-independent form of the wave equation.\n" 
      L"- Propagation of acoustic or electromagnetic waves.\n\n" 
      L"--- EQUATION ---\n" 
      L"∇²u + k²u = f(x,y)\n\n" 
      L"--- PRESET PARAMETERS ---\n" 
      L"Here, k² is represented by the coefficient 'c'. This preset models a wave-like phenomenon on a 3x1 rectangular domain with a source term and mixed boundary conditions.",
      L"--- ФИЗИЧЕСКАЯ ИНТЕРПРЕТАЦИЯ ---\n" 
      L"Возникает в физике при изучении волновых явлений, колебаний или процессов диффузии-реакции. Его часто называют уравнением 'реакция-диффузия'. Примеры:\n" 
      L"- Стационарная форма волнового уравнения.\n" 
      L"- Распространение акустических или электромагнитных волн.\n\n" 
      L"--- УРАВНЕНИЕ ---\n" 
      L"∇²u + k²u = f(x,y)\n\n" 
      L"--- ПАРАМЕТРЫ ПРЕДУСТАНОВКИ ---\n" 
      L"Здесь k² представлен коэффициентом 'c'. Эта предустановка моделирует волноподобное явление на прямоугольной области 3x1 с источником и смешанными граничными условиями.",
      L"3.0", L"1.0", L"30", L"10",
      L"1.0", L"0.0", L"1.0", L"0.0", L"0.0", L"1.0", L"cos(pi*x/3)*cos(pi*y)",
      L"0.0", L"cos(pi*3/3)*cos(pi*y)", L"cos(pi*x/3)*cos(0)", L"cos(pi*x/3)*cos(pi*1)"
    },
    { L"Convection-Diffusion",
      L"--- PHYSICAL INTERPRETATION ---\n" 
      L"Models the transport of a substance (e.g., pollutant, heat) due to two processes: convection (transport by bulk motion of a fluid) and diffusion (transport from high to low concentration). The Péclet number (Pe = |b|L/a) indicates which process dominates.\n\n" 
      L"--- EQUATION ---\n" 
      L"-∇·(a∇u) + b·∇u = f(x,y)\n\n" 
      L"--- PRESET PARAMETERS ---\n" 
      L"Models the transport of a substance with concentration 1 at the left inlet (x=0) and 0 at the right outlet (x=2). The flow field 'b' pushes the substance to the right, while diffusion 'a' spreads it out. A source 'f' is present near the outlet. Expect a plume developing from left to right.",
      L"--- ФИЗИЧЕСКАЯ ИНТЕРПРЕТАЦИЯ ---\n" 
      L"Моделирует перенос вещества (напр., загрязнителя, тепла) за счет двух процессов: конвекции (перенос объемным движением жидкости) и диффузии (перенос из области высокой концентрации в низкую). Число Пекле (Pe = |b|L/a) показывает, какой процесс доминирует.\n\n" 
      L"--- УРАВНЕНИЕ ---\n" 
      L"-∇·(a∇u) + b·∇u = f(x,y)\n\n" 
      L"--- ПАРАМЕТРЫ ПРЕДУСТАНОВКИ ---\n" 
      L"Моделирует перенос вещества с концентрацией 1 на левом входе (x=0) и 0 на правом выходе (x=2). Поле течения 'b' переносит вещество вправо, а диффузия 'a' его рассеивает. Вблизи выхода имеется источник 'f'. Ожидается шлейф, распространяющийся слева направо.",
      L"2.0", L"1.0", L"40", L"20",
      L"0.01 + 0.005*x", L"0.0", L"0.01 + 0.005*x", L"1.0", L"0.0", L"0.0", L"exp(-10*((x-2)^2 + (y-0.5)^2))",
      L"1.0", L"0.0", L"0.0", L"0.0"
    },
    { L"Reaction-Diffusion",
      L"--- PHYSICAL INTERPRETATION ---\n" 
      L"Similar to the Helmholtz equation, this equation models processes where a substance both diffuses and reacts. The 'c' term represents the reaction rate. If c > 0, it's a decay/absorption; if c < 0, it's a growth/generation.\n\n" 
      L"--- EQUATION ---\n" 
      L"-∇·(a∇u) + cu = f(x,y)\n\n" 
      L"--- PRESET PARAMETERS ---\n" 
      L"This preset features a variable diffusion coefficient 'a' and a positive reaction term 'c', modeling a process with diffusion and absorption. A complex source 'f' is applied. The boundary conditions are of mixed types.",
      L"--- ФИЗИЧЕСКАЯ ИНТЕРПРЕТАЦИЯ ---\n" 
      L"Подобно уравнению Гельмгольца, это уравнение моделирует процессы, в которых вещество одновременно диффундирует и реагирует. Член 'c' представляет скорость реакции. Если c > 0 - это распад/поглощение; если c < 0 - рост/генерация.\n\n" 
      L"--- УРАВНЕНИЕ ---\n" 
      L"-∇·(a∇u) + cu = f(x,y)\n\n" 
      L"--- ПАРАМЕТРЫ ПРЕДУСТАНОВКИ ---\n" 
      L"В этой предустановке используется переменный коэффициент диффузии 'a' и положительный реакционный член 'c', моделируя процесс с диффузией и поглощением. Применяется сложный источник 'f'. Граничные условия - смешанного типа.",
      L"2.0", L"2.0", L"30", L"30",
      L"0.1 + 0.05*x*y", L"0.0", L"0.1 + 0.05*x*y", L"0.0", L"0.0", L"1.0", L"10*exp(-5*((x-1)^2 + (y-1)^2)) + 2*pi^2*cos(pi*x)*cos(pi*y)",
      L"20.0", L"10.0", L"15.0", L"5.0"
    },
    { L"General Elliptic",
      L"--- PHYSICAL INTERPRETATION ---\n" 
      L"This is the most general form, combining diffusion (a), convection (b), and reaction (c). The 'a12' term represents anisotropic diffusion, where the diffusion rate depends on the direction.\n\n" 
      L"--- EQUATION ---\n" 
      L"-∇·(a∇u) + b·∇u + cu = f(x,y)\n\n" 
      L"--- PRESET PARAMETERS ---\n" 
      L"This preset showcases a complex scenario with variable and anisotropic diffusion, convection in both x and y directions, a reaction term, and a source. This can model complex physical systems like heat transfer in a moving, non-uniform medium.",
      L"--- ФИЗИЧЕСКАЯ ИНТЕРПРЕТАЦИЯ ---\n" 
      L"Это наиболее общая форма, объединяющая диффузию (a), конвекцию (b) и реакцию (c). Член 'a12' представляет анизотропную диффузию, где скорость диффузии зависит от направления.\n\n" 
      L"--- УРАВНЕНИЕ ---\n" 
      L"-∇·(a∇u) + b·∇u + cu = f(x,y)\n\n" 
      L"--- ПАРАМЕТРЫ ПРЕДУСТАНОВКИ ---\n" 
      L"Эта предустановка демонстрирует сложный сценарий с переменной и анизотропной диффузией, конвекцией в направлениях x и y, реакционным членом и источником. Это может моделировать сложные физические системы, такие как теплопередача в движущейся, неоднородной среде.",
      L"2.0", L"1.0", L"50", L"25",
      L"1 + 0.5*sin(pi*x)*cos(pi*y)", L"0.2", L"0.8 + 0.3*cos(pi*x)", L"0.2*x", L"0.1*y", L"0.1", L"sin(pi*x/2)*sin(pi*y) + 0.5*pi^2*cos(pi*x/2)*cos(pi*y)",
      L"0.0", L"1.0", L"0.0", L"0.0"
    }
};

const int NUM_PRESETS = sizeof(PRESETS) / sizeof(PRESETS[0]);

void OnPresetHelpClicked(HWND hwnd) {
    LRESULT selResult = SendMessage(g_appData.hPresetCombo, CB_GETCURSEL, 0, 0);
    int selection = static_cast<int>(selResult);

    if (selection > 0 && selection <= NUM_PRESETS) {
        int idx = selection - 1;
        const PresetData& preset = PRESETS[idx];
        const wchar_t* description = (langContext.getLanguageType() == Language::Russian) ? preset.description_ru : preset.description_en;
        MessageBoxW(hwnd, description, preset.name, MB_OK | MB_ICONINFORMATION);
    } else {
        const wchar_t* msg = (langContext.getLanguageType() == Language::Russian) ? L"Выберите предустановку, чтобы увидеть справку." : L"Please select a preset to see its help information.";
        const wchar_t* title = (langContext.getLanguageType() == Language::Russian) ? L"Справка" : L"Help";
        MessageBoxW(hwnd, msg, title, MB_OK | MB_ICONINFORMATION);
    }
}


// Function to switch between languages using strategy pattern
void SwitchLanguage() {
    static EnglishLanguageStrategy englishStrategy;
    static RussianLanguageStrategy russianStrategy;

    if (langContext.getLanguageType() == Language::English) {
        langContext.setStrategy(&russianStrategy);
    } else {
        langContext.setStrategy(&englishStrategy);
    }
}

// Function to update all UI elements with current language strings
void UpdateLanguageStrings(HWND hwnd) {
    // Update window title
    SetWindowTextW(hwnd, (langContext.getLanguageType() == Language::English) ?
        L"Finite Element Method Solver - Elliptic Equations" :
        L"Решатель методом конечных элементов - Эллиптические уравнения");

    // Update button texts
    HWND hSolveBtn = GetDlgItem(hwnd, 1001);
    HWND hResetBtn = GetDlgItem(hwnd, 1002);
    HWND hExportBtn = GetDlgItem(hwnd, 1003);

    if (hSolveBtn) SetWindowTextW(hSolveBtn, langContext.getSolveBtn());
    if (hResetBtn) SetWindowTextW(hResetBtn, langContext.getResetBtn());
    if (hExportBtn) SetWindowTextW(hExportBtn, langContext.getExportBtn());

    // Update group box titles
    if (g_appData.hCoeffGroup) SetWindowTextW(g_appData.hCoeffGroup, langContext.getCoeffGroup());
    if (g_appData.hBCGroup) SetWindowTextW(g_appData.hBCGroup, langContext.getBcGroup());
    if (g_appData.hSolInfoGroup) SetWindowTextW(g_appData.hSolInfoGroup, langContext.getSolInfoGroup());

    // Update preset combo box label
    if (g_appData.hPresetLabel) SetWindowTextW(g_appData.hPresetLabel, langContext.getEquationPreset());

    // Update preset combo box options
    if (g_appData.hPresetCombo) {
        // Save current selection
        int currentSel = static_cast<int>(SendMessageW(g_appData.hPresetCombo, CB_GETCURSEL, 0, 0));

        // Clear and re-add items
        SendMessageW(g_appData.hPresetCombo, CB_RESETCONTENT, 0, 0);
        for (int i = 0; i < 7; i++) {
            SendMessageW(g_appData.hPresetCombo, CB_ADDSTRING, 0, (LPARAM)langContext.getPresetOption(i));
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
                SendMessageW(bcCombos[i], CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(j));
            }

            // Restore selection
            SendMessageW(bcCombos[i], CB_SETCURSEL, currentSel, 0);
        }
    }

    // Update status bar if it exists
    if (g_appData.hStatus) {
        int currentSel = static_cast<int>(SendMessageW(g_appData.hPresetCombo, CB_GETCURSEL, 0, 0));
        if (currentSel == 0) {
            SetWindowTextW(g_appData.hStatus, langContext.getStatusReady());
        } else {
            SetWindowTextW(g_appData.hStatus, langContext.getStatusPresetLoaded());
        }
    }

    // Update the language toggle button to show the opposite language (so user knows which language will be switched to)
    HWND hLangBtn = GetDlgItem(hwnd, 1005);
    if (hLangBtn) {
        SetWindowTextW(hLangBtn, (langContext.getLanguageType() == Language::English) ? L"RU" : L"EN");
    }

    // Refresh the window to update all controls
    InvalidateRect(hwnd, NULL, TRUE);
}

void LoadPreset(int presetIndex) {
    // Adjust presetIndex to be 0-based for array access (since combo box starts from 1)
    if (presetIndex < 1 || presetIndex > NUM_PRESETS) {
        return; // Invalid preset index
    }

    // Convert to 0-based index
    int idx = presetIndex - 1;
    const PresetData& preset = PRESETS[idx];

    // Set all values using the preset data
    SetWindowTextW(g_appData.hLxEdit, preset.lx);
    SetWindowTextW(g_appData.hLyEdit, preset.ly);
    SetWindowTextW(g_appData.hNxEdit, preset.nx);
    SetWindowTextW(g_appData.hNyEdit, preset.ny);
    SetWindowTextW(g_appData.hA11Edit, preset.a11);
    SetWindowTextW(g_appData.hA12Edit, preset.a12);
    SetWindowTextW(g_appData.hA22Edit, preset.a22);
    SetWindowTextW(g_appData.hB1Edit, preset.b1);
    SetWindowTextW(g_appData.hB2Edit, preset.b2);
    SetWindowTextW(g_appData.hCEdit, preset.c);
    SetWindowTextW(g_appData.hFEdit, preset.f);
    SetWindowTextW(g_appData.hWestValue, preset.westValue);
    SetWindowTextW(g_appData.hEastValue, preset.eastValue);
    SetWindowTextW(g_appData.hSouthValue, preset.southValue);
    SetWindowTextW(g_appData.hNorthValue, preset.northValue);

    // Update the solution info panel to reflect current values
    wchar_t buffer[256];
    std::wostringstream solutionInfo;
    solutionInfo << L"Current Configuration:\n";
    solutionInfo << L"-------------------\n";

    // Get current values from UI
    GetWindowTextW(g_appData.hLxEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"Domain X: [0, " << buffer << L"]\n";
    GetWindowTextW(g_appData.hLyEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"Domain Y: [0, " << buffer << L"]\n";
    GetWindowTextW(g_appData.hNxEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"X nodes: " << buffer << L"\n";
    GetWindowTextW(g_appData.hNyEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"Y nodes: " << buffer << L"\n\n";

    solutionInfo << L"Coefficients:\n";
    GetWindowTextW(g_appData.hA11Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  a11(x,y) = " << buffer << L"\n";
    GetWindowTextW(g_appData.hA12Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  a12(x,y) = " << buffer << L"\n";
    GetWindowTextW(g_appData.hA22Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  a22(x,y) = " << buffer << L"\n";
    GetWindowTextW(g_appData.hB1Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  b1(x,y) = " << buffer << L"\n";
    GetWindowTextW(g_appData.hB2Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  b2(x,y) = " << buffer << L"\n";
    GetWindowTextW(g_appData.hCEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  c(x,y) = " << buffer << L"\n";
    GetWindowTextW(g_appData.hFEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  f(x,y) = " << buffer << L"\n";

    SetWindowTextW(g_appData.hSolutionInfo, solutionInfo.str().c_str());

    // Clear any stored solution data and visualization
    if (g_currentGdiVisualizer) {
        g_currentGdiVisualizer->render(Mesh(), std::vector<double>(), 0, 0, "");
    }

    // Force immediate update of all affected controls to ensure visual update
    // Use both SetWindowPos with SWP_FRAMECHANGED and RedrawWindow to ensure update
    RedrawWindow(g_appData.hLxEdit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hLyEdit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hNxEdit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hNyEdit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hA11Edit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hA12Edit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hA22Edit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hB1Edit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hB2Edit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hCEdit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hFEdit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hWestValue, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hEastValue, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hSouthValue, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hNorthValue, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(g_appData.hSolutionInfo, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

    // Force refresh of the visualization frame to clear any stale display
    InvalidateRect(g_appData.hVisualFrame, NULL, TRUE);
}