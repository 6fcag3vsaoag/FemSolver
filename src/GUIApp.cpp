#include "GUIApp.h"
#include "FemSolver.h"
#include "EllipticApp.h"
#include <iostream>
#include <stdexcept>
#include <clocale>
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

    // Last computed solution and mesh (for visualization)
    std::vector<double> lastSolution;
    Mesh lastMesh;
    bool hasLastSolution = false;
};

// Language enum
enum class Language {
    English,
    Russian
};

// Interface for language strategy
class ILanguageStrategy {
public:
    virtual ~ILanguageStrategy() = default;
    virtual const char* getEquationPreset() const = 0;
    virtual const char* getSolveBtn() const = 0;
    virtual const char* getResetBtn() const = 0;
    virtual const char* getExportBtn() const = 0;
    virtual const char* getHelpBtn() const = 0;
    virtual const char* getCoeffGroup() const = 0;
    virtual const char* getBcGroup() const = 0;
    virtual const char* getSolInfoGroup() const = 0;
    virtual const char* getPresetOption(int index) const = 0;
    virtual const char* getBcType(int index) const = 0;
    virtual const char* getStatusReady() const = 0;
    virtual const char* getStatusPresetLoaded() const = 0;
    virtual const char* getStatusSolved() const = 0;
    virtual const char* getHelpText() const = 0;
    virtual Language getLanguageType() const = 0;
};

// English language strategy
class EnglishLanguageStrategy : public ILanguageStrategy {
public:
    const char* getEquationPreset() const override { return "Equation Preset:"; }
    const char* getSolveBtn() const override { return "Solve"; }
    const char* getResetBtn() const override { return "Reset"; }
    const char* getExportBtn() const override { return "Export"; }
    const char* getHelpBtn() const override { return "Help"; }
    const char* getCoeffGroup() const override { return "Equation Coefficients"; }
    const char* getBcGroup() const override { return "Boundary Conditions"; }
    const char* getSolInfoGroup() const override { return "Solution Information"; }
    const char* getPresetOption(int index) const override {
        static const char* options[] = {
            "Select Preset...", "Laplace Equation", "Poisson Equation",
            "Helmholtz Equation", "Convection-Diffusion", "Reaction-Diffusion", "General Elliptic"
        };
        return (index >= 0 && index < 7) ? options[index] : "";
    }
    const char* getBcType(int index) const override {
        static const char* types[] = { "Dirichlet", "Neumann" };
        return (index >= 0 && index < 2) ? types[index] : "";
    }
    const char* getStatusReady() const override { return "Ready - Select an equation preset to begin"; }
    const char* getStatusPresetLoaded() const override { return "Preset loaded. Ready to solve."; }
    const char* getStatusSolved() const override { return "Solution computed successfully!"; }
    const char* getHelpText() const override {
        return "FEM Solver Help:\n\n"
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
               "f: source term";
    }
    Language getLanguageType() const override { return Language::English; }
};

// Russian language strategy
class RussianLanguageStrategy : public ILanguageStrategy {
public:
    const char* getEquationPreset() const override { return "Предустановка уравнения:"; }
    const char* getSolveBtn() const override { return "Решить"; }
    const char* getResetBtn() const override { return "Сброс"; }
    const char* getExportBtn() const override { return "Экспорт"; }
    const char* getHelpBtn() const override { return "Справка"; }
    const char* getCoeffGroup() const override { return "Коэффициенты уравнения"; }
    const char* getBcGroup() const override { return "Граничные условия"; }
    const char* getSolInfoGroup() const override { return "Информация о решении"; }
    const char* getPresetOption(int index) const override {
        static const char* options[] = {
            "Выберите предустановку...", "Уравнение Лапласа", "Уравнение Пуассона",
            "Уравнение Гельмгольца", "Конвективно-диффузионное", "Реакционно-диффузионное", "Общее эллиптическое"
        };
        return (index >= 0 && index < 7) ? options[index] : "";
    }
    const char* getBcType(int index) const override {
        static const char* types[] = { "Дирихле", "Нейман" };
        return (index >= 0 && index < 2) ? types[index] : "";
    }
    const char* getStatusReady() const override { return "Готово - Выберите предустановку уравнения для начала"; }
    const char* getStatusPresetLoaded() const override { return "Предустановка загружена. Готов к решению."; }
    const char* getStatusSolved() const override { return "Решение вычислено успешно!"; }
    const char* getHelpText() const override {
        return "Справка FEM Solver:\n\n"
               "1. Выберите предустановку уравнения из выпадающего списка\n"
               "2. Настройте параметры сетки (Lx, Ly, Nx, Ny)\n"
               "3. Измените коэффициенты уравнения при необходимости\n"
               "4. Установите граничные условия для каждой стороны\n"
               "5. Нажмите 'Решить' для вычисления решения\n"
               "6. Используйте 'Экспорт' для сохранения результатов\n\n"
               "Коэффициенты:\n"
               "a11, a22: коэффициенты диффузии\n"
               "a12: коэффициент смешанной производной\n"
               "b1, b2: коэффициенты конвекции\n"
               "c: коэффициент реакции\n"
               "f: источник";
    }
    Language getLanguageType() const override { return Language::Russian; }
};

// Context for language strategy
class LanguageContext {
private:
    ILanguageStrategy* strategy;
public:
    LanguageContext(ILanguageStrategy* s) : strategy(s) {}
    void setStrategy(ILanguageStrategy* s) { strategy = s; }
    const char* getEquationPreset() const { return strategy->getEquationPreset(); }
    const char* getSolveBtn() const { return strategy->getSolveBtn(); }
    const char* getResetBtn() const { return strategy->getResetBtn(); }
    const char* getExportBtn() const { return strategy->getExportBtn(); }
    const char* getHelpBtn() const { return strategy->getHelpBtn(); }
    const char* getCoeffGroup() const { return strategy->getCoeffGroup(); }
    const char* getBcGroup() const { return strategy->getBcGroup(); }
    const char* getSolInfoGroup() const { return strategy->getSolInfoGroup(); }
    const char* getPresetOption(int index) const { return strategy->getPresetOption(index); }
    const char* getBcType(int index) const { return strategy->getBcType(index); }
    const char* getStatusReady() const { return strategy->getStatusReady(); }
    const char* getStatusPresetLoaded() const { return strategy->getStatusPresetLoaded(); }
    const char* getStatusSolved() const { return strategy->getStatusSolved(); }
    const char* getHelpText() const { return strategy->getHelpText(); }
    Language getLanguageType() const { return strategy->getLanguageType(); }
};

// Global language context using strategy pattern
EnglishLanguageStrategy englishStrategy;
LanguageContext langContext(&englishStrategy);

AppData g_appData;

// Function declarations
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateControls(HWND hwnd);
void OnSolveButtonClicked(HWND hwnd);
void OnResetButtonClicked(HWND hwnd);
void OnExportButtonClicked(HWND hwnd);
void OnPresetChanged(HWND hwnd, int presetIndex);
void LoadPreset(int presetIndex);
void SwitchLanguage();
void UpdateLanguageStrings(HWND hwnd);

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
    // Set the locale to support Cyrillic characters
    setlocale(LC_ALL, "Russian");

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
        "Finite Element Method Solver - Elliptic Equations", // Default title, will be updated after creation
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
                        langContext.getHelpText(),
                        "Help", MB_OK | MB_ICONINFORMATION);
                    break;
                case 1005: // Language toggle button
                    SwitchLanguage();
                    UpdateLanguageStrings(hwnd);
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

                // Resize the preset combo box
                int presetLabelWidth = static_cast<int>(leftWidth * 0.28);
                if (g_appData.hPresetCombo) {
                    MoveWindow(g_appData.hPresetCombo, leftStart + presetLabelWidth, 8, leftWidth - presetLabelWidth - 10, 150, TRUE);
                }

                // Resize the preset label
                if (g_appData.hPresetLabel) {
                    MoveWindow(g_appData.hPresetLabel, leftStart, 10, presetLabelWidth, 25, TRUE);
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
                int helpWidth = 60;
                int langWidth = 60;

                // Calculate text width for Solve button
                GetTextExtentPoint32A(hdc, langContext.getSolveBtn(), strlen(langContext.getSolveBtn()), &textSize);
                solveWidth = std::max<int>(50, textSize.cx + 16); // Add padding

                // Calculate text width for Reset button
                GetTextExtentPoint32A(hdc, langContext.getResetBtn(), strlen(langContext.getResetBtn()), &textSize);
                resetWidth = std::max<int>(50, textSize.cx + 16); // Add padding

                // Calculate text width for Export button
                GetTextExtentPoint32A(hdc, langContext.getExportBtn(), strlen(langContext.getExportBtn()), &textSize);
                exportWidth = std::max<int>(50, textSize.cx + 16); // Add padding

                // Calculate text width for Help button
                GetTextExtentPoint32A(hdc, langContext.getHelpBtn(), strlen(langContext.getHelpBtn()), &textSize);
                helpWidth = std::max<int>(50, textSize.cx + 16); // Add padding

                // Calculate text width for Language Toggle button
                GetTextExtentPoint32A(hdc, (langContext.getLanguageType() == Language::English) ? "RU" : "EN",
                                      strlen((langContext.getLanguageType() == Language::English) ? "RU" : "EN"), &textSize);
                langWidth = std::max<int>(50, textSize.cx + 16); // Add padding

                SelectObject(hdc, hOldFont);
                ReleaseDC(hwnd, hdc);

                // Calculate total required width
                int totalRequiredWidth = solveWidth + resetWidth + exportWidth + helpWidth + langWidth
                                       + (buttonSpacing * (totalButtons - 1)) + (buttonMargin * 2);

                // If total required width exceeds available space, scale down proportionally
                if (totalRequiredWidth > leftWidth) {
                    double scale = (double)(leftWidth - (buttonMargin * 2) - (buttonSpacing * (totalButtons - 1))) /
                                  (double)(solveWidth + resetWidth + exportWidth + helpWidth + langWidth);
                    solveWidth = (int)(solveWidth * scale);
                    resetWidth = (int)(resetWidth * scale);
                    exportWidth = (int)(exportWidth * scale);
                    helpWidth = (int)(helpWidth * scale);
                    langWidth = (int)(langWidth * scale);
                }

                // Position buttons with calculated widths
                int currentX = leftStart + buttonMargin;

                HWND hSolveBtn = GetDlgItem(hwnd, 1001);
                HWND hResetBtn = GetDlgItem(hwnd, 1002);
                HWND hExportBtn = GetDlgItem(hwnd, 1003);
                HWND hHelpBtn = GetDlgItem(hwnd, 1004);
                HWND hLangBtn = GetDlgItem(hwnd, 1005);

                if (hSolveBtn) MoveWindow(hSolveBtn, currentX, buttonY, solveWidth, buttonHeight, TRUE);
                currentX += solveWidth + buttonSpacing;
                if (hResetBtn) MoveWindow(hResetBtn, currentX, buttonY, resetWidth, buttonHeight, TRUE);
                currentX += resetWidth + buttonSpacing;
                if (hExportBtn) MoveWindow(hExportBtn, currentX, buttonY, exportWidth, buttonHeight, TRUE);
                currentX += exportWidth + buttonSpacing;
                if (hHelpBtn) MoveWindow(hHelpBtn, currentX, buttonY, helpWidth, buttonHeight, TRUE);
                currentX += helpWidth + buttonSpacing;
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
                // Handle painting for the visualization frame
                HWND visualFrame = g_appData.hVisualFrame;
                if (visualFrame && IsWindow(visualFrame)) {
                    // Check if this paint message is for the visualization frame
                    if (wParam == NULL || reinterpret_cast<HWND>(wParam) == visualFrame) {
                        PAINTSTRUCT ps;
                        HDC hdc = BeginPaint(visualFrame, &ps);

                        RECT rect;
                        GetClientRect(visualFrame, &rect);

                        // Fill background
                        HBRUSH bgBrush = CreateSolidBrush(RGB(240, 240, 240));
                        FillRect(hdc, &rect, bgBrush);
                        DeleteObject(bgBrush);

                        // Visualize the solution
                        // First, try to use the locally stored solution if available
                        if (g_appData.hasLastSolution && !g_appData.lastSolution.empty()) {
                            const std::vector<double>& solution = g_appData.lastSolution;
                            const Mesh& mesh = g_appData.lastMesh;

                            // Draw title
                            SetTextColor(hdc, RGB(0, 0, 0));
                            SetBkMode(hdc, TRANSPARENT);
                            std::string title = "Solution Visualization";
                            TextOutA(hdc, 10, 10, title.c_str(), static_cast<int>(title.length()));

                            // Draw solution stats (using precomputed values to avoid intensive calculation)
                            // We'll use the solution size and a simple placeholder for now to avoid intensive computation
                            std::ostringstream stats;
                            stats << "Nodes: " << solution.size()
                                  << ", Elements: " << mesh.elements.size();
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
                                // Compute min/max values for proper normalization
                                if (solution.size() >= 4) {
                                    // Find min and max values in the solution for proper scaling
                                    double minVal = *std::min_element(solution.begin(), solution.end());
                                    double maxVal = *std::max_element(solution.begin(), solution.end());
                                    double range = (maxVal == minVal) ? 1.0 : (maxVal - minVal);

                                    // Use a simplified grid view based on the mesh
                                    int gridX = (g_appData.Nx < 50) ? g_appData.Nx : 50;  // Increase grid limit for better visualization
                                    int gridY = (g_appData.Ny < 50) ? g_appData.Ny : 50;

                                    int cellWidth = visChartWidth / gridX;
                                    int cellHeight = visChartHeight / gridY;

                                    if (cellWidth >= 1 && cellHeight >= 1) {
                                        // Visualize all solution points (without sampling to show more detail)
                                        int xStep = std::max(1, g_appData.Nx / gridX);
                                        int yStep = std::max(1, g_appData.Ny / gridY);

                                        for (int y = 0; y < g_appData.Ny && y < gridY; y += yStep) {
                                            for (int x = 0; x < g_appData.Nx && x < gridX; x += xStep) {
                                                int idx = y * g_appData.Nx + x;
                                                if (idx >= 0 && idx < static_cast<int>(solution.size())) {
                                                    double val = solution[idx];
                                                    // Normalize using actual min/max values for better visualization
                                                    double normVal = (range != 0) ? (val - minVal) / range : 0.0;

                                                    // Create color based on solution value (blue=low, red=high, green=medium)
                                                    int r = static_cast<int>(normVal * 255);
                                                    int b = static_cast<int>((1.0 - normVal) * 255);
                                                    int g = static_cast<int>(normVal * 128); // Green channel for intermediate values

                                                    HBRUSH cellBrush = CreateSolidBrush(RGB(r, g, b));
                                                    RECT cellRect;
                                                    cellRect.left = visChartLeft + (x/xStep) * cellWidth;
                                                    cellRect.top = visChartTop + (y/yStep) * cellHeight;
                                                    cellRect.right = cellRect.left + cellWidth;
                                                    cellRect.bottom = cellRect.top + cellHeight;

                                                    FillRect(hdc, &cellRect, cellBrush);
                                                    FrameRect(hdc, &cellRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
                                                    DeleteObject(cellBrush);
                                                }
                                            }
                                        }
                                    }

                                    // Draw legend with actual min/max values
                                    int legendX = visChartLeft + visChartWidth + 5;
                                    int legendY = visChartTop;
                                    int legendHeight = 100;
                                    int legendWidth = 20;

                                    if (legendY + legendHeight < rect.bottom && legendX + legendWidth < rect.right) {
                                        // Draw color legend
                                        for (int i = 0; i < legendHeight; i++) {
                                            double ratio = static_cast<double>(i) / legendHeight;
                                            int r = static_cast<int>(ratio * 255);
                                            int b = static_cast<int>((1.0 - ratio) * 255);
                                            int g = static_cast<int>(ratio * 128);

                                            HBRUSH legendBrush = CreateSolidBrush(RGB(r, g, b));
                                            RECT legendRect;
                                            legendRect.left = legendX;
                                            legendRect.top = legendY + i;
                                            legendRect.right = legendX + legendWidth;
                                            legendRect.bottom = legendY + i + 1;
                                            FillRect(hdc, &legendRect, legendBrush);
                                            DeleteObject(legendBrush);
                                        }

                                        // Draw legend labels with actual min/max values
                                        std::ostringstream lowLabel;
                                        lowLabel << std::fixed << std::setprecision(2) << minVal;
                                        std::ostringstream highLabel;
                                        highLabel << std::fixed << std::setprecision(2) << maxVal;

                                        TextOutA(hdc, legendX + legendWidth + 5, legendY, lowLabel.str().c_str(), static_cast<int>(lowLabel.str().length()));
                                        TextOutA(hdc, legendX + legendWidth + 5, legendY + legendHeight - 15, highLabel.str().c_str(), static_cast<int>(highLabel.str().length()));
                                    }
                                }
                            }
                        } else {
                            // No solution yet - show placeholder
                            SetTextColor(hdc, RGB(128, 128, 128));
                            std::string placeholder = "No solution computed yet.\nClick 'Solve' to compute.";
                            RECT placeholderRect = {10, 10, rect.right - 10, rect.bottom - 10};
                            DrawTextA(hdc, placeholder.c_str(), -1, &placeholderRect, DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX);
                        }

                        EndPaint(visualFrame, &ps);
                        return 0; // Handle internally, don't pass to DefWindowProc
                    }
                }
            }
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

    // Calculate left and right panel widths (left 16%, right 80% with 4% margin between)
    int leftWidth = static_cast<int>(width * 0.16); // Reduced to 16% for controls (further reduced from 20%)
    int rightWidth = static_cast<int>(width * 0.80); // 80% for visualization
    int margin = static_cast<int>(width * 0.02); // Responsive margin

    // Left panel (controls)
    int leftStart = margin;
    int rightStart = leftStart + leftWidth + margin;

    // Top section: Preset selection
    int presetLabelWidth = static_cast<int>(leftWidth * 0.28); // About 28% of the left panel width
    g_appData.hPresetLabel = CreateWindow("Static", langContext.getEquationPreset(), WS_VISIBLE | WS_CHILD, leftStart, 10, presetLabelWidth, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hPresetCombo = CreateWindow("ComboBox", "",
                                          WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                          leftStart + presetLabelWidth, 8, leftWidth - presetLabelWidth - 10, 150, hwnd, (HMENU)2000, GetModuleHandle(NULL), NULL);

    // Add preset options based on current language
    for (int i = 0; i < 7; i++) {
        SendMessage(g_appData.hPresetCombo, CB_ADDSTRING, 0, (LPARAM)langContext.getPresetOption(i));
    }
    SendMessage(g_appData.hPresetCombo, CB_SETCURSEL, 0, 0); // Set default selection

    // Second section: Buttons (Solve, Reset, Export, Help, Language Toggle) - properly aligned layout
    int buttonY = 45;
    int buttonHeight = 30;
    int buttonSpacing = 2; // Fixed 2px spacing between buttons as requested
    int totalButtons = 5;
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
    int helpWidth = 60;
    int langWidth = 60;

    // Calculate text width for Solve button
    GetTextExtentPoint32A(hdc, langContext.getSolveBtn(), strlen(langContext.getSolveBtn()), &textSize);
    solveWidth = std::max<int>(50, textSize.cx + 16); // Add padding

    // Calculate text width for Reset button
    GetTextExtentPoint32A(hdc, langContext.getResetBtn(), strlen(langContext.getResetBtn()), &textSize);
    resetWidth = std::max<int>(50, textSize.cx + 16); // Add padding

    // Calculate text width for Export button
    GetTextExtentPoint32A(hdc, langContext.getExportBtn(), strlen(langContext.getExportBtn()), &textSize);
    exportWidth = std::max<int>(50, textSize.cx + 16); // Add padding

    // Calculate text width for Help button
    GetTextExtentPoint32A(hdc, langContext.getHelpBtn(), strlen(langContext.getHelpBtn()), &textSize);
    helpWidth = std::max<int>(50, textSize.cx + 16); // Add padding

    // Calculate text width for Language Toggle button
    GetTextExtentPoint32A(hdc, (langContext.getLanguageType() == Language::English) ? "RU" : "EN",
                          strlen((langContext.getLanguageType() == Language::English) ? "RU" : "EN"), &textSize);
    langWidth = std::max<int>(50, textSize.cx + 16); // Add padding

    SelectObject(hdc, hOldFont);
    ReleaseDC(hwnd, hdc);

    // Calculate total required width
    int totalRequiredWidth = solveWidth + resetWidth + exportWidth + helpWidth + langWidth
                           + (buttonSpacing * (totalButtons - 1)) + (buttonMargin * 2);

    // If total required width exceeds available space, scale down proportionally
    if (totalRequiredWidth > leftWidth) {
        double scale = (double)(leftWidth - (buttonMargin * 2) - (buttonSpacing * (totalButtons - 1))) /
                      (double)(solveWidth + resetWidth + exportWidth + helpWidth + langWidth);
        solveWidth = (int)(solveWidth * scale);
        resetWidth = (int)(resetWidth * scale);
        exportWidth = (int)(exportWidth * scale);
        helpWidth = (int)(helpWidth * scale);
        langWidth = (int)(langWidth * scale);
    }

    // Position buttons with calculated widths
    int currentX = leftStart + buttonMargin;

    // Create Solve button
    CreateWindow("Button", langContext.getSolveBtn(), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 currentX, buttonY, solveWidth, buttonHeight, hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);

    // Create Reset button
    currentX += solveWidth + buttonSpacing;
    CreateWindow("Button", langContext.getResetBtn(), WS_VISIBLE | WS_CHILD,
                 currentX, buttonY, resetWidth, buttonHeight, hwnd, (HMENU)1002, GetModuleHandle(NULL), NULL);

    // Create Export button
    currentX += resetWidth + buttonSpacing;
    CreateWindow("Button", langContext.getExportBtn(), WS_VISIBLE | WS_CHILD,
                 currentX, buttonY, exportWidth, buttonHeight, hwnd, (HMENU)1003, GetModuleHandle(NULL), NULL);

    // Create Help button
    currentX += exportWidth + buttonSpacing;
    CreateWindow("Button", langContext.getHelpBtn(), WS_VISIBLE | WS_CHILD,
                 currentX, buttonY, helpWidth, buttonHeight, hwnd, (HMENU)1004, GetModuleHandle(NULL), NULL);

    // Create Language Toggle button
    currentX += helpWidth + buttonSpacing;
    CreateWindow("Button", (langContext.getLanguageType() == Language::English) ? "RU" : "EN",
                 WS_VISIBLE | WS_CHILD,
                 currentX, buttonY, langWidth, buttonHeight, hwnd, (HMENU)1005, GetModuleHandle(NULL), NULL);

    // Third section: Equation coefficients (in group box) - with proper spacing
    int coeffY = buttonY + buttonHeight + 10;  // Position just below the buttons with appropriate spacing
    g_appData.hCoeffGroup = CreateWindow("Button", langContext.getCoeffGroup(),
                                    WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                    leftStart, coeffY, leftWidth - 10, 200, hwnd, NULL, GetModuleHandle(NULL), NULL);

    int coeffStartY = coeffY + 20;
    int coeffLabelWidth = static_cast<int>(leftWidth * 0.15);  // Reduced width for labels (like "a11(x,y):")
    int coeffFieldWidth = static_cast<int>(leftWidth * 0.28);  // Reduced width for text fields
    int coeffSpacingX = coeffLabelWidth + 5;  // Reduced spacing between label and field

    g_appData.hCoeffLabels[0] = CreateWindow("Static", "a11(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA11Edit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX, coeffStartY-2, coeffFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hCoeffLabels[1] = CreateWindow("Static", "a12(x,y):", WS_VISIBLE | WS_CHILD, leftStart + coeffSpacingX + coeffFieldWidth + 10, coeffStartY, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA12Edit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX + coeffFieldWidth + coeffLabelWidth + 10, coeffStartY-2, coeffFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hCoeffLabels[2] = CreateWindow("Static", "a22(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY + 30, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hA22Edit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX, coeffStartY + 28, coeffFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hCoeffLabels[3] = CreateWindow("Static", "b1(x,y):", WS_VISIBLE | WS_CHILD, leftStart + coeffSpacingX + coeffFieldWidth + 10, coeffStartY + 30, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hB1Edit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX + coeffFieldWidth + coeffLabelWidth + 10, coeffStartY + 28, coeffFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hCoeffLabels[4] = CreateWindow("Static", "b2(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY + 60, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hB2Edit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX, coeffStartY + 58, coeffFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hCoeffLabels[5] = CreateWindow("Static", "c(x,y):", WS_VISIBLE | WS_CHILD, leftStart + coeffSpacingX + coeffFieldWidth + 10, coeffStartY + 60, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hCEdit = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX + coeffFieldWidth + coeffLabelWidth + 10, coeffStartY + 58, coeffFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hCoeffLabels[6] = CreateWindow("Static", "f(x,y):", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY + 90, coeffLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hFEdit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + coeffSpacingX, coeffStartY + 88, coeffFieldWidth * 2 + coeffLabelWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Mesh parameters
    int meshLabelWidth = static_cast<int>(leftWidth * 0.06);  // Reduced width for labels like "Lx:"
    int meshFieldWidth = static_cast<int>(leftWidth * 0.10);  // Reduced width for text fields
    int meshSpacing = static_cast<int>(leftWidth * 0.02);     // Reduced spacing between elements

    g_appData.hMeshLabels[0] = CreateWindow("Static", "Lx:", WS_VISIBLE | WS_CHILD, leftStart + 10, coeffStartY + 125, meshLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hLxEdit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hMeshLabels[1] = CreateWindow("Static", "Ly:", WS_VISIBLE | WS_CHILD, leftStart + 10 + meshLabelWidth + meshFieldWidth + meshSpacing, coeffStartY + 125, meshLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hLyEdit = CreateWindow("Edit", "1.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + meshLabelWidth + meshFieldWidth + meshSpacing + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hMeshLabels[2] = CreateWindow("Static", "Nx:", WS_VISIBLE | WS_CHILD, leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 2, coeffStartY + 125, meshLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNxEdit = CreateWindow("Edit", "20", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 2 + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hMeshLabels[3] = CreateWindow("Static", "Ny:", WS_VISIBLE | WS_CHILD, leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 3, coeffStartY + 125, meshLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNyEdit = CreateWindow("Edit", "20", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 3 + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Fourth section: Boundary conditions (in group box)
    int bcY = coeffY + 210;
    g_appData.hBCGroup = CreateWindow("Button", langContext.getBcGroup(),
                                 WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                 leftStart, bcY, leftWidth - 10, 160, hwnd, NULL, GetModuleHandle(NULL), NULL);

    int bcStartY = bcY + 20;
    int bcLabelWidth = static_cast<int>(leftWidth * 0.08);  // Reduced width for labels like "West:"
    int bcComboWidth = static_cast<int>(leftWidth * 0.18);  // Reduced width for combo boxes
    int bcValueWidth = static_cast<int>(leftWidth * 0.18);  // Reduced width for value fields

    // Calculate positions for east/south controls to avoid overflow
    int eastStartX = leftStart + static_cast<int>(leftWidth * 0.5); // Position on the right half of the group box

    g_appData.hBCLabels[0] = CreateWindow("Static", "West:", WS_VISIBLE | WS_CHILD, leftStart + 10, bcStartY, bcLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hWestBC = CreateWindow("ComboBox", "",
                                WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                leftStart + 10 + bcLabelWidth, bcStartY-2, bcComboWidth, 60, hwnd, (HMENU)2001, GetModuleHandle(NULL), NULL);
    SendMessage(g_appData.hWestBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(0)); // Dirichlet
    SendMessage(g_appData.hWestBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(1)); // Neumann
    SendMessage(g_appData.hWestBC, CB_SETCURSEL, 0, 0);
    // Calculate available space for West value field to prevent overflow
    int westValueWidth = eastStartX - (leftStart + 10 + bcLabelWidth + bcComboWidth + 10) - 5;
    g_appData.hWestValue = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + bcLabelWidth + bcComboWidth + 10, bcStartY-2, westValueWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hBCLabels[1] = CreateWindow("Static", "East:", WS_VISIBLE | WS_CHILD, eastStartX, bcStartY, bcLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hEastBC = CreateWindow("ComboBox", "",
                                WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                eastStartX + bcLabelWidth, bcStartY-2, bcComboWidth, 60, hwnd, (HMENU)2002, GetModuleHandle(NULL), NULL);
    SendMessage(g_appData.hEastBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(0)); // Dirichlet
    SendMessage(g_appData.hEastBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(1)); // Neumann
    SendMessage(g_appData.hEastBC, CB_SETCURSEL, 0, 0);
    // Calculate available space for East value field to prevent overflow
    int eastValueWidth = leftWidth - 15 - (eastStartX + bcLabelWidth + bcComboWidth + 5);
    g_appData.hEastValue = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, eastStartX + bcLabelWidth + bcComboWidth + 5, bcStartY-2, eastValueWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hBCLabels[2] = CreateWindow("Static", "South:", WS_VISIBLE | WS_CHILD, leftStart + 10, bcStartY + 35, bcLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hSouthBC = CreateWindow("ComboBox", "",
                                 WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                 leftStart + 10 + bcLabelWidth, bcStartY + 33, bcComboWidth, 60, hwnd, (HMENU)2003, GetModuleHandle(NULL), NULL);
    SendMessage(g_appData.hSouthBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(0)); // Dirichlet
    SendMessage(g_appData.hSouthBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(1)); // Neumann
    SendMessage(g_appData.hSouthBC, CB_SETCURSEL, 0, 0);
    g_appData.hSouthValue = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, leftStart + 10 + bcLabelWidth + bcComboWidth + 10, bcStartY + 33, westValueWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    g_appData.hBCLabels[3] = CreateWindow("Static", "North:", WS_VISIBLE | WS_CHILD, eastStartX, bcStartY + 35, bcLabelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_appData.hNorthBC = CreateWindow("ComboBox", "",
                                 WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                 eastStartX + bcLabelWidth, bcStartY + 33, bcComboWidth, 60, hwnd, (HMENU)2004, GetModuleHandle(NULL), NULL);
    SendMessage(g_appData.hNorthBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(0)); // Dirichlet
    SendMessage(g_appData.hNorthBC, CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(1)); // Neumann
    SendMessage(g_appData.hNorthBC, CB_SETCURSEL, 0, 0);
    g_appData.hNorthValue = CreateWindow("Edit", "0.0", WS_VISIBLE | WS_CHILD | WS_BORDER, eastStartX + bcLabelWidth + bcComboWidth + 5, bcStartY + 33, eastValueWidth, 22, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Fifth section: Solution information panel (in group box) - extend to bottom
    int bcSectionHeight = 160; // Height of boundary conditions group box
    int solInfoY = bcY + bcSectionHeight + 10; // Position right after boundary conditions
    int solInfoHeight = height - solInfoY - 50; // Extend to above status bar (leaving space for status bar)

    g_appData.hSolInfoGroup = CreateWindow("Button", langContext.getSolInfoGroup(),
                                           WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                           leftStart, solInfoY, leftWidth - 10, solInfoHeight, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Create a static control to display solution information
    g_appData.hSolutionInfo = CreateWindow("Edit", "No solution computed yet. Click 'Solve' to begin.",
                                           WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                                           leftStart + 10, solInfoY + 20, leftWidth - 25, solInfoHeight - 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Right panel: Visualization
    g_appData.hVisualFrame = CreateWindow("Static", "Solution Visualization",
                                         WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTERIMAGE | SS_CENTER,
                                         rightStart, 10, rightWidth - 20, height - 60, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Status bar at the bottom - adjusted for the solution info panel
    g_appData.hStatus = CreateWindow("Static", langContext.getStatusReady(),
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
                    // Store the computed solution locally for persistent visualization
                    g_appData.lastSolution = solution;
                    g_appData.lastMesh = mesh;
                    g_appData.hasLastSolution = true;

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

                    // Update the solution information panel with detailed information
                    std::ostringstream solutionInfo;
                    solutionInfo << "Solution Details:\n";
                    solutionInfo << "-------------------\n";
                    solutionInfo << "Nodes: " << solution.size() << "\n";
                    solutionInfo << "Elements: " << mesh.elements.size() << "\n";
                    solutionInfo << "Domain: [0, " << g_appData.Lx << "] x [0, " << g_appData.Ly << "]\n";
                    solutionInfo << "Mesh: " << g_appData.Nx << " x " << g_appData.Ny << " nodes\n";
                    solutionInfo << "Min value: " << min_val << "\n";
                    solutionInfo << "Max value: " << max_val << "\n";
                    solutionInfo << "Avg value: " << (solution.empty() ? 0.0 : std::accumulate(solution.begin(), solution.end(), 0.0) / solution.size()) << "\n\n";
                    solutionInfo << "Coefficients:\n";
                    solutionInfo << "  a11(x,y) = " << g_appData.a11Func << "\n";
                    solutionInfo << "  a12(x,y) = " << g_appData.a12Func << "\n";
                    solutionInfo << "  a22(x,y) = " << g_appData.a22Func << "\n";
                    solutionInfo << "  b1(x,y) = " << g_appData.b1Func << "\n";
                    solutionInfo << "  b2(x,y) = " << g_appData.b2Func << "\n";
                    solutionInfo << "  c(x,y) = " << g_appData.cFunc << "\n";
                    solutionInfo << "  f(x,y) = " << g_appData.fFunc << "\n\n";
                    solutionInfo << "Boundary Conditions:\n";
                    solutionInfo << "  West: " << ((westBC == "dirichlet") ? "Dirichlet" : "Neumann")
                                 << " (value=" << westVal << ")\n";
                    solutionInfo << "  East: " << ((eastBC == "dirichlet") ? "Dirichlet" : "Neumann")
                                 << " (value=" << eastVal << ")\n";
                    solutionInfo << "  South: " << ((southBC == "dirichlet") ? "Dirichlet" : "Neumann")
                                 << " (value=" << southVal << ")\n";
                    solutionInfo << "  North: " << ((northBC == "dirichlet") ? "Dirichlet" : "Neumann")
                                 << " (value=" << northVal << ")\n\n";
                    solutionInfo << "Solution computed successfully at: " << __DATE__ << " " << __TIME__;

                    SetWindowText(g_appData.hSolutionInfo, solutionInfo.str().c_str());
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

        // Also update the solution info panel to show the error
        std::string errorInfo = "Solution Error:\n";
        errorInfo += "-------------------\n";
        errorInfo += "An error occurred during solution:\n";
        errorInfo += e.what();
        SetWindowText(g_appData.hSolutionInfo, errorInfo.c_str());
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
        SetWindowText(g_appData.hStatus, langContext.getStatusPresetLoaded());

        // Update the solution info panel to show current values
        char buffer[256];
        std::ostringstream solutionInfo;
        solutionInfo << "Current Configuration:\n";
        solutionInfo << "-------------------\n";

        // Get current values from UI
        GetWindowText(g_appData.hLxEdit, buffer, sizeof(buffer));
        solutionInfo << "Domain X: [0, " << buffer << "]\n";
        GetWindowText(g_appData.hLyEdit, buffer, sizeof(buffer));
        solutionInfo << "Domain Y: [0, " << buffer << "]\n";
        GetWindowText(g_appData.hNxEdit, buffer, sizeof(buffer));
        solutionInfo << "X nodes: " << buffer << "\n";
        GetWindowText(g_appData.hNyEdit, buffer, sizeof(buffer));
        solutionInfo << "Y nodes: " << buffer << "\n\n";

        solutionInfo << "Coefficients:\n";
        GetWindowText(g_appData.hA11Edit, buffer, sizeof(buffer));
        solutionInfo << "  a11(x,y) = " << buffer << "\n";
        GetWindowText(g_appData.hA12Edit, buffer, sizeof(buffer));
        solutionInfo << "  a12(x,y) = " << buffer << "\n";
        GetWindowText(g_appData.hA22Edit, buffer, sizeof(buffer));
        solutionInfo << "  a22(x,y) = " << buffer << "\n";
        GetWindowText(g_appData.hB1Edit, buffer, sizeof(buffer));
        solutionInfo << "  b1(x,y) = " << buffer << "\n";
        GetWindowText(g_appData.hB2Edit, buffer, sizeof(buffer));
        solutionInfo << "  b2(x,y) = " << buffer << "\n";
        GetWindowText(g_appData.hCEdit, buffer, sizeof(buffer));
        solutionInfo << "  c(x,y) = " << buffer << "\n";
        GetWindowText(g_appData.hFEdit, buffer, sizeof(buffer));
        solutionInfo << "  f(x,y) = " << buffer << "\n";

        SetWindowText(g_appData.hSolutionInfo, solutionInfo.str().c_str());

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
    const char* name;
    const char* description;
    const char* lx, * ly, * nx, * ny;
    const char* a11, * a12, * a22, * b1, * b2, * c, * f;
    const char* westValue, * eastValue, * southValue, * northValue;
};

// Define all presets in a structured way
const PresetData PRESETS[] = {
    { "Laplace Equation",
      "LAPLACE EQUATION\\n\\n"
      "Solves: ∂²u/∂x² + ∂²u/∂y² = 0\\n\\n"
      "Description:\\n"
      "Classic Laplace equation describing steady-state processes:\\n"
      "- Heat distribution without sources\\n"
      "- Electrostatic potential in vacuum\\n"
      "- Potential flow of incompressible fluid\\n\\n"
      "Coefficients:\\n"
      "- a11 = a22 = 1 (unit diffusion)\\n"
      "- a12 = b1 = b2 = c = f = 0 (no mixed derivatives, convection, reaction or sources)\\n\\n"
      "Boundary Conditions:\\n"
      "All sides: u = x² + y²",
      "1.0", "1.0", "20", "20",
      "1.0", "0.0", "1.0", "0.0", "0.0", "0.0", "0.0",
      "x*x + y*y", "x*x + y*y", "x*x + y*y", "x*x + y*y"
    },
    { "Poisson Equation",
      "POISSON EQUATION\\n\\n"
      "Solves: ∂²u/∂x² + ∂²u/∂y² = 8π²·sin(2πx)·sin(2πy)\\n\\n"
      "Description:\\n"
      "Poisson equation with known analytical solution:\\n"
      "- Heat distribution with sources\\n"
      "- Electrostatic potential with charges\\n"
      "- Stationary processes with sources/sinks\\n\\n"
      "Coefficients:\\n"
      "- a11 = a22 = 1 (unit diffusion)\\n"
      "- a12 = b1 = b2 = c = 0 (no mixed derivatives, convection, reaction)\\n"
      "- f(x,y) = 8π²·sin(2πx)·sin(2πy) (harmonic source)\\n\\n"
      "Analytical Solution:\\n"
      "u(x,y) = sin(2πx)·sin(2πy)\\n\\n"
      "Boundary Conditions:\\n"
      "Zero Dirichlet conditions on all boundaries (u = 0)",
      "1.0", "1.0", "20", "20",
      "1.0", "0.0", "1.0", "0.0", "0.0", "0.0", "8*pi*pi*sin(2*pi*x)*sin(2*pi*y)",
      "0.0", "0.0", "0.0", "0.0"
    },
    { "Helmholtz Equation",
      "HELMHOLTZ EQUATION\\n\\n"
      "Solves: ∂²u/∂x² + ∂²u/∂y² + u = cos(πx/3)·cos(πy)\\n\\n"
      "Description:\\n"
      "Helmholtz equation - important elliptic equation:\\n"
      "- Wave processes in frequency domain\\n"
      "- Quantum mechanics (stationary Schrödinger equation)\\n"
      "- Acoustics and electromagnetic waves\\n\\n"
      "Coefficients:\\n"
      "- a11 = a22 = 1 (diffusion)\\n"
      "- c = 1 (reaction term)\\n"
      "- a12 = b1 = b2 = 0\\n"
      "- f(x,y) = cos(πx/3)·cos(πy) (source)\\n\\n"
      "Boundary Conditions:\\n"
      "- West: Neumann (∂u/∂n = 0) - symmetry\\n"
      "- Others: Dirichlet with specified values",
      "3.0", "1.0", "30", "10",
      "1.0", "0.0", "1.0", "0.0", "0.0", "1.0", "cos(pi*x/3)*cos(pi*y)",
      "0.0", "cos(pi*3/3)*cos(pi*y)", "cos(pi*x/3)*cos(0)", "cos(pi*x/3)*cos(pi*1)"
    },
    { "Convection-Diffusion",
      "CONVECTION-DIFFUSION\\n\\n"
      "Solves: (0.01+0.005x)∂²u/∂x² + (0.01+0.005x)∂²u/∂y² + ∂u/∂x = exp(-10·((x-2)²+(y-0.5)²))\\n\\n"
      "Description:\\n"
      "Convection-diffusion equation describing transport:\\n"
      "- Pollutant transport in atmosphere or water\\n"
      "- Heat transfer with fluid motion\\n"
      "- Diffusion in flowing media\\n\\n"
      "Coefficients:\\n"
      "- a11 = a22 = 0.01 + 0.005x (variable diffusion)\\n"
      "- b1 = 1 (convection in positive X direction)\\n"
      "- b2 = 0 (no convection in Y)\\n"
      "- a12 = c = 0\\n"
      "- f(x,y) = exp(-10·((x-2)²+(y-0.5)²)) (local source)\\n\\n"
      "Boundary Conditions:\\n"
      "- West: Dirichlet u = 1 (inlet concentration)\\n"
      "- East: Dirichlet u = 0 (outlet concentration)\\n"
      "- South/North: Neumann (symmetry)",
      "2.0", "1.0", "40", "20",
      "0.01 + 0.005*x", "0.0", "0.01 + 0.005*x", "1.0", "0.0", "0.0", "exp(-10*((x-2)^2 + (y-0.5)^2))",
      "1.0", "0.0", "0.0", "0.0"
    },
    { "Reaction-Diffusion",
      "REACTION-DIFFUSION\\n\\n"
      "Solves: (0.1+0.05xy)∂²u/∂x² + (0.05xy)∂²u/∂y² + u = 10·exp(-5·((x-1)²+(y-1)²)) + 2π²·cos(πx)·cos(πy)\\n\\n"
      "Description:\\n"
      "Reaction-diffusion equations modeling many processes:\\n"
      "- Population dynamics of species\\n"
      "- Chemical reactions with diffusion\\n"
      "- Neural impulse propagation\\n\\n"
      "Coefficients:\\n"
      "- a11 = a22 = 0.1 + 0.05xy (variable diffusion)\\n"
      "- c = 1 (linear reaction)\\n"
      "- a12 = b1 = b2 = 0\\n"
      "- f(x,y) = complex source (exponential + harmonic)\\n\\n"
      "Boundary Conditions:\\n"
      "Mixed conditions:\\n"
      "- Some: Dirichlet with different values\\n"
      "- North: Neumann ∂u/∂n = 5",
      "2.0", "2.0", "30", "30",
      "0.1 + 0.05*x*y", "0.0", "0.1 + 0.05*x*y", "0.0", "0.0", "1.0", "10*exp(-5*((x-1)^2 + (y-1)^2)) + 2*pi^2*cos(pi*x)*cos(pi*y)",
      "20.0", "10.0", "15.0", "5.0"
    },
    { "General Elliptic",
      "GENERAL ELLIPTIC\\n\\n"
      "Solves: General elliptic equation with variable coefficients\\n"
      "(1+0.5sin(πx)cos(πy))∂²u/∂x² + 2·0.2∂²u/∂x∂y + (0.8+0.3cos(πx))∂²u/∂y² + 0.2x∂u/∂x + 0.1y∂u/∂y + 0.1u = sin(πx/2)sin(πy) + 0.5π²cos(πx/2)cos(πy)\\n\\n"
      "Description:\\n"
      "General elliptic equation encompassing all physical effects:\\n"
      "- Variable diffusion in both directions\\n"
      "- Mixed derivative term\\n"
      "- Convection in both directions\\n"
      "- Reaction term\\n\\n"
      "Coefficients:\\n"
      "- a11(x,y) = 1 + 0.5sin(πx)cos(πy) (variable diffusion)\\n"
      "- a12 = 0.2 (mixed derivative coefficient)\\n"
      "- a22(x,y) = 0.8 + 0.3cos(πx) (variable diffusion)\\n"
      "- b1(x,y) = 0.2x (convection in x)\\n"
      "- b2(x,y) = 0.1y (convection in y)\\n"
      "- c = 0.1 (reaction)\\n"
      "- f(x,y) = source term\\n\\n"
      "Boundary Conditions:\\n"
      "- West: Neumann\\n"
      "- Others: Dirichlet",
      "2.0", "1.0", "50", "25",
      "1 + 0.5*sin(pi*x)*cos(pi*y)", "0.2", "0.8 + 0.3*cos(pi*x)", "0.2*x", "0.1*y", "0.1", "sin(pi*x/2)*sin(pi*y) + 0.5*pi^2*cos(pi*x/2)*cos(pi*y)",
      "0.0", "1.0", "0.0", "0.0"
    }
};

const int NUM_PRESETS = sizeof(PRESETS) / sizeof(PRESETS[0]);

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
    SetWindowText(hwnd, (langContext.getLanguageType() == Language::English) ?
        "Finite Element Method Solver - Elliptic Equations" :
        "Решатель методом конечных элементов - Эллиптические уравнения");

    // Update button texts
    HWND hSolveBtn = GetDlgItem(hwnd, 1001);
    HWND hResetBtn = GetDlgItem(hwnd, 1002);
    HWND hExportBtn = GetDlgItem(hwnd, 1003);
    HWND hHelpBtn = GetDlgItem(hwnd, 1004);

    if (hSolveBtn) SetWindowText(hSolveBtn, langContext.getSolveBtn());
    if (hResetBtn) SetWindowText(hResetBtn, langContext.getResetBtn());
    if (hExportBtn) SetWindowText(hExportBtn, langContext.getExportBtn());
    if (hHelpBtn) SetWindowText(hHelpBtn, langContext.getHelpBtn());

    // Update group box titles
    if (g_appData.hCoeffGroup) SetWindowText(g_appData.hCoeffGroup, langContext.getCoeffGroup());
    if (g_appData.hBCGroup) SetWindowText(g_appData.hBCGroup, langContext.getBcGroup());
    if (g_appData.hSolInfoGroup) SetWindowText(g_appData.hSolInfoGroup, langContext.getSolInfoGroup());

    // Update preset combo box label
    if (g_appData.hPresetLabel) SetWindowText(g_appData.hPresetLabel, langContext.getEquationPreset());

    // Update preset combo box options
    if (g_appData.hPresetCombo) {
        // Save current selection
        int currentSel = static_cast<int>(SendMessage(g_appData.hPresetCombo, CB_GETCURSEL, 0, 0));

        // Clear and re-add items
        SendMessage(g_appData.hPresetCombo, CB_RESETCONTENT, 0, 0);
        for (int i = 0; i < 7; i++) {
            SendMessage(g_appData.hPresetCombo, CB_ADDSTRING, 0, (LPARAM)langContext.getPresetOption(i));
        }

        // Restore selection
        SendMessage(g_appData.hPresetCombo, CB_SETCURSEL, currentSel, 0);
    }

    // Update boundary condition combo boxes
    HWND bcCombos[] = {g_appData.hWestBC, g_appData.hEastBC, g_appData.hSouthBC, g_appData.hNorthBC};
    for (int i = 0; i < 4; i++) {
        if (bcCombos[i]) {
            // Save current selection
            int currentSel = static_cast<int>(SendMessage(bcCombos[i], CB_GETCURSEL, 0, 0));

            // Clear and re-add items
            SendMessage(bcCombos[i], CB_RESETCONTENT, 0, 0);
            for (int j = 0; j < 2; j++) {
                SendMessage(bcCombos[i], CB_ADDSTRING, 0, (LPARAM)langContext.getBcType(j));
            }

            // Restore selection
            SendMessage(bcCombos[i], CB_SETCURSEL, currentSel, 0);
        }
    }

    // Update status bar if it exists
    if (g_appData.hStatus) {
        int currentSel = static_cast<int>(SendMessage(g_appData.hPresetCombo, CB_GETCURSEL, 0, 0));
        if (currentSel == 0) {
            SetWindowText(g_appData.hStatus, langContext.getStatusReady());
        } else {
            SetWindowText(g_appData.hStatus, langContext.getStatusPresetLoaded());
        }
    }

    // Update the language toggle button to show the opposite language (so user knows which language will be switched to)
    HWND hLangBtn = GetDlgItem(hwnd, 1005);
    if (hLangBtn) {
        SetWindowText(hLangBtn, (langContext.getLanguageType() == Language::English) ? "RU" : "EN");
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
    SetWindowText(g_appData.hLxEdit, preset.lx);
    SetWindowText(g_appData.hLyEdit, preset.ly);
    SetWindowText(g_appData.hNxEdit, preset.nx);
    SetWindowText(g_appData.hNyEdit, preset.ny);
    SetWindowText(g_appData.hA11Edit, preset.a11);
    SetWindowText(g_appData.hA12Edit, preset.a12);
    SetWindowText(g_appData.hA22Edit, preset.a22);
    SetWindowText(g_appData.hB1Edit, preset.b1);
    SetWindowText(g_appData.hB2Edit, preset.b2);
    SetWindowText(g_appData.hCEdit, preset.c);
    SetWindowText(g_appData.hFEdit, preset.f);
    SetWindowText(g_appData.hWestValue, preset.westValue);
    SetWindowText(g_appData.hEastValue, preset.eastValue);
    SetWindowText(g_appData.hSouthValue, preset.southValue);
    SetWindowText(g_appData.hNorthValue, preset.northValue);

    // Update the solution info panel to reflect current values
    char buffer[256];
    std::ostringstream solutionInfo;
    solutionInfo << "Current Configuration:\n";
    solutionInfo << "-------------------\n";

    // Get current values from UI
    GetWindowText(g_appData.hLxEdit, buffer, sizeof(buffer));
    solutionInfo << "Domain X: [0, " << buffer << "]\n";
    GetWindowText(g_appData.hLyEdit, buffer, sizeof(buffer));
    solutionInfo << "Domain Y: [0, " << buffer << "]\n";
    GetWindowText(g_appData.hNxEdit, buffer, sizeof(buffer));
    solutionInfo << "X nodes: " << buffer << "\n";
    GetWindowText(g_appData.hNyEdit, buffer, sizeof(buffer));
    solutionInfo << "Y nodes: " << buffer << "\n\n";

    solutionInfo << "Coefficients:\n";
    GetWindowText(g_appData.hA11Edit, buffer, sizeof(buffer));
    solutionInfo << "  a11(x,y) = " << buffer << "\n";
    GetWindowText(g_appData.hA12Edit, buffer, sizeof(buffer));
    solutionInfo << "  a12(x,y) = " << buffer << "\n";
    GetWindowText(g_appData.hA22Edit, buffer, sizeof(buffer));
    solutionInfo << "  a22(x,y) = " << buffer << "\n";
    GetWindowText(g_appData.hB1Edit, buffer, sizeof(buffer));
    solutionInfo << "  b1(x,y) = " << buffer << "\n";
    GetWindowText(g_appData.hB2Edit, buffer, sizeof(buffer));
    solutionInfo << "  b2(x,y) = " << buffer << "\n";
    GetWindowText(g_appData.hCEdit, buffer, sizeof(buffer));
    solutionInfo << "  c(x,y) = " << buffer << "\n";
    GetWindowText(g_appData.hFEdit, buffer, sizeof(buffer));
    solutionInfo << "  f(x,y) = " << buffer << "\n";

    SetWindowText(g_appData.hSolutionInfo, solutionInfo.str().c_str());

    // Show preset description in message box
    MessageBox(NULL, preset.description, preset.name, MB_OK | MB_ICONINFORMATION);

    // Clear any stored solution data to prevent showing stale visualization
    g_appData.lastSolution.clear();
    g_appData.lastMesh.nodes.clear();
    g_appData.lastMesh.elements.clear();
    g_appData.hasLastSolution = false;

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
