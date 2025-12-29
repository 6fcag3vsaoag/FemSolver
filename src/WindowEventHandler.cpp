#include "../include/WindowEventHandler.h"
#include "../include/Localization.h"
#include "../include/PresetManager.h"
#include "../include/IVisualizer.h"
#include "../include/rendering/DirectXVisualizer.h" // For dynamic_cast
#include "../include/StringUtils.h"
#include <commctrl.h>
#include <windowsx.h> // For GET_X_LPARAM, GET_Y_LPARAM
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <iomanip>

// Static member definition
AppData* WindowEventHandler::s_appData = nullptr;

// Forward declarations for functions that will be defined in GUIApp.cpp
extern IVisualizer* g_currentVisualizer; // Global visualizer instance
extern void OnSolveButtonClicked(HWND hwnd);
extern void OnResetButtonClicked(HWND hwnd);
extern void OnExportButtonClicked(HWND hwnd);
extern void OnPresetHelpClicked(HWND hwnd);
extern void OnPresetChanged(HWND hwnd, int presetIndex);
extern void UpdateLanguageStrings(HWND hwnd);
extern void SwitchLanguage();
extern void CreateControls(HWND hwnd, IVisualizer* visualizer);

// Initialize the application data
void WindowEventHandler::setAppData(AppData* appData) {
    s_appData = appData;
}

// Get the current application data
AppData* WindowEventHandler::getAppData() {
    return s_appData;
}

// Main window procedure
LRESULT CALLBACK WindowEventHandler::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (s_appData == nullptr) {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    // Capture mouse state for dragging
    static bool isDragging = false;
    static int lastMouseX = 0;
    static int lastMouseY = 0;

    switch(msg) {
        case WM_CREATE:
            CreateControls(hwnd, g_currentVisualizer); // Pass the global IVisualizer instance
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

        case WM_LBUTTONDOWN:
            SetCapture(hwnd);
            isDragging = true;
            lastMouseX = GET_X_LPARAM(lParam);
            lastMouseY = GET_Y_LPARAM(lParam);
            break;

        case WM_LBUTTONUP:
            ReleaseCapture();
            isDragging = false;
            break;

        case WM_RBUTTONDOWN:
            SetCapture(hwnd);
            isDragging = true;
            lastMouseX = GET_X_LPARAM(lParam);
            lastMouseY = GET_Y_LPARAM(lParam);
            break;

        case WM_RBUTTONUP:
            ReleaseCapture();
            isDragging = false;
            break;

        case WM_MOUSEMOVE:
            if (isDragging) {
                int mouseX = GET_X_LPARAM(lParam);
                int mouseY = GET_Y_LPARAM(lParam);

                bool leftButton = (wParam & MK_LBUTTON) != 0;
                bool rightButton = (wParam & MK_RBUTTON) != 0;

                if (auto dxVis = dynamic_cast<DirectXVisualizer*>(g_currentVisualizer)) {
                    dxVis->handleMouseInput(mouseX, mouseY, leftButton, rightButton);
                }
            }
            break;

        case WM_MOUSEWHEEL:
            {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam); // Corrected: wParam instead of wPram
                if (auto dxVis = dynamic_cast<DirectXVisualizer*>(g_currentVisualizer)) {
                    dxVis->handleMouseWheel(delta);
                }
            }
            break;

        case WM_SIZE:
            // Handle window resize to make UI responsive
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
                if (s_appData->hStatus) {
                    MoveWindow(s_appData->hStatus, 10, newHeight - 40, newWidth - 20, 30, TRUE);
                }

                // Resize and reposition visualization frame (right panel)
                if (s_appData->hVisualFrame) {
                    // Calculate visualization height, leaving space for status bar and some margin at the bottom
                    int visHeight = newHeight - 60; // Leave 60px for status bar and margins
                    MoveWindow(s_appData->hVisualFrame, rightStart, 10, rightWidth - 20, visHeight, TRUE);
                }

                // Reposition Preset controls
                int presetLabelWidth = static_cast<int>(leftWidth * 0.28);
                int helpButtonWidth = 30;
                int comboWidth = leftWidth - presetLabelWidth - helpButtonWidth - 15;

                if (s_appData->hPresetLabel) {
                    MoveWindow(s_appData->hPresetLabel, leftStart, 10, presetLabelWidth, 25, TRUE);
                }
                if (s_appData->hPresetCombo) {
                    MoveWindow(s_appData->hPresetCombo, leftStart + presetLabelWidth, 8, comboWidth, 150, TRUE);
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
                if (s_appData->hCoeffGroup) {
                    MoveWindow(s_appData->hCoeffGroup, leftStart, coeffY, leftWidth - 10, coeffSectionHeight, TRUE);
                }
                if (s_appData->hBCGroup) {
                    MoveWindow(s_appData->hBCGroup, leftStart, bcY, leftWidth - 10, bcSectionHeight, TRUE);
                }
                if (s_appData->hSolInfoGroup) {
                    MoveWindow(s_appData->hSolInfoGroup, leftStart, solInfoY, leftWidth - 10, solInfoHeight, TRUE);
                }

                // Move the solution info text control inside its group
                if (s_appData->hSolutionInfo) {
                    MoveWindow(s_appData->hSolutionInfo, leftStart + 10, solInfoY + 20, leftWidth - 25, solInfoHeight - 30, TRUE);
                }

                // Now resize and reposition all static labels and controls within the coefficient section
                int coeffStartY = coeffY + 20;
                int coeffLabelWidth = static_cast<int>(leftWidth * 0.15);  // Reduced width for labels (like "a11(x,y):")
                int coeffFieldWidth = static_cast<int>(leftWidth * 0.28);  // Reduced width for text fields
                int coeffSpacingX = coeffLabelWidth + 5;  // Reduced spacing between label and field

                // Resize coefficient labels
                if (s_appData->hCoeffLabels[0]) MoveWindow(s_appData->hCoeffLabels[0], leftStart + 10, coeffStartY, coeffLabelWidth, 20, TRUE); // a11
                if (s_appData->hCoeffLabels[1]) MoveWindow(s_appData->hCoeffLabels[1], leftStart + coeffSpacingX + coeffFieldWidth + 10, coeffStartY, coeffLabelWidth, 20, TRUE); // a12
                if (s_appData->hCoeffLabels[2]) MoveWindow(s_appData->hCoeffLabels[2], leftStart + 10, coeffStartY + 30, coeffLabelWidth, 20, TRUE); // a22
                if (s_appData->hCoeffLabels[3]) MoveWindow(s_appData->hCoeffLabels[3], leftStart + coeffSpacingX + coeffFieldWidth + 10, coeffStartY + 30, coeffLabelWidth, 20, TRUE); // b1
                if (s_appData->hCoeffLabels[4]) MoveWindow(s_appData->hCoeffLabels[4], leftStart + 10, coeffStartY + 60, coeffLabelWidth, 20, TRUE); // b2
                if (s_appData->hCoeffLabels[5]) MoveWindow(s_appData->hCoeffLabels[5], leftStart + coeffSpacingX + coeffFieldWidth + 10, coeffStartY + 60, coeffLabelWidth, 20, TRUE); // c
                if (s_appData->hCoeffLabels[6]) MoveWindow(s_appData->hCoeffLabels[6], leftStart + 10, coeffStartY + 90, coeffLabelWidth, 20, TRUE); // f

                // Resize coefficient input fields
                if (s_appData->hA11Edit) MoveWindow(s_appData->hA11Edit, leftStart + coeffSpacingX, coeffStartY - 2, coeffFieldWidth, 22, TRUE);
                if (s_appData->hA12Edit) MoveWindow(s_appData->hA12Edit, leftStart + coeffSpacingX + coeffFieldWidth + coeffLabelWidth + 10, coeffStartY - 2, coeffFieldWidth, 22, TRUE);
                if (s_appData->hA22Edit) MoveWindow(s_appData->hA22Edit, leftStart + coeffSpacingX, coeffStartY + 28, coeffFieldWidth, 22, TRUE);
                if (s_appData->hB1Edit) MoveWindow(s_appData->hB1Edit, leftStart + coeffSpacingX + coeffFieldWidth + coeffLabelWidth + 10, coeffStartY + 28, coeffFieldWidth, 22, TRUE);
                if (s_appData->hB2Edit) MoveWindow(s_appData->hB2Edit, leftStart + coeffSpacingX, coeffStartY + 58, coeffFieldWidth, 22, TRUE);
                if (s_appData->hCEdit) MoveWindow(s_appData->hCEdit, leftStart + coeffSpacingX + coeffFieldWidth + coeffLabelWidth + 10, coeffStartY + 58, coeffFieldWidth, 22, TRUE);
                if (s_appData->hFEdit) MoveWindow(s_appData->hFEdit, leftStart + coeffSpacingX, coeffStartY + 88, coeffFieldWidth * 2 + coeffLabelWidth, 22, TRUE);

                // Resize mesh labels and parameters
                int meshLabelWidth = static_cast<int>(leftWidth * 0.06);  // Reduced width for labels like "Lx:"
                int meshFieldWidth = static_cast<int>(leftWidth * 0.10);  // Reduced width for text fields
                int meshSpacing = static_cast<int>(leftWidth * 0.02);     // Reduced spacing between elements

                if (s_appData->hMeshLabels[0]) MoveWindow(s_appData->hMeshLabels[0], leftStart + 10, coeffStartY + 125, meshLabelWidth, 20, TRUE);  // Lx
                if (s_appData->hLxEdit) MoveWindow(s_appData->hLxEdit, leftStart + 10 + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, TRUE);
                if (s_appData->hMeshLabels[1]) MoveWindow(s_appData->hMeshLabels[1], leftStart + 10 + meshLabelWidth + meshFieldWidth + meshSpacing, coeffStartY + 125, meshLabelWidth, 20, TRUE);  // Ly
                if (s_appData->hLyEdit) MoveWindow(s_appData->hLyEdit, leftStart + 10 + meshLabelWidth + meshFieldWidth + meshSpacing + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, TRUE);
                if (s_appData->hMeshLabels[2]) MoveWindow(s_appData->hMeshLabels[2], leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 2, coeffStartY + 125, meshLabelWidth, 20, TRUE);  // Nx
                if (s_appData->hNxEdit) MoveWindow(s_appData->hNxEdit, leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 2 + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, TRUE);
                if (s_appData->hMeshLabels[3]) MoveWindow(s_appData->hMeshLabels[3], leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 3, coeffStartY + 125, meshLabelWidth, 20, TRUE);  // Ny
                if (s_appData->hNyEdit) MoveWindow(s_appData->hNyEdit, leftStart + 10 + (meshLabelWidth + meshFieldWidth + meshSpacing) * 3 + meshLabelWidth, coeffStartY + 123, meshFieldWidth, 22, TRUE);

                // Resize boundary condition labels and controls
                int bcStartY = bcY + 20;
                int bcLabelWidth = static_cast<int>(leftWidth * 0.08);  // Reduced width for labels like "West:"
                int bcComboWidth = static_cast<int>(leftWidth * 0.18);  // Reduced combo box width
                int bcValueWidth = static_cast<int>(leftWidth * 0.18);  // Reduced value field width

                // Calculate positions for east/south controls to avoid overflow
                int eastStartX = leftStart + static_cast<int>(leftWidth * 0.5); // Position on the right half of the group box
                int eastSpacing = 5; // Reduced spacing

                // West boundary controls
                if (s_appData->hBCLabels[0]) MoveWindow(s_appData->hBCLabels[0], leftStart + 10, bcStartY, bcLabelWidth, 20, TRUE); // West
                if (s_appData->hWestBC) MoveWindow(s_appData->hWestBC, leftStart + 10 + bcLabelWidth, bcStartY-2, bcComboWidth, 60, TRUE);
                // Ensure West value field fits within the left half
                int westValueWidth = eastStartX - (leftStart + 10 + bcLabelWidth + bcComboWidth + 10) - 5;
                if (westValueWidth > 0) {
                    if (s_appData->hWestValue) MoveWindow(s_appData->hWestValue, leftStart + 10 + bcLabelWidth + bcComboWidth + 10, bcStartY-2, westValueWidth, 22, TRUE);
                }

                // East boundary controls
                if (s_appData->hBCLabels[1]) MoveWindow(s_appData->hBCLabels[1], eastStartX, bcStartY, bcLabelWidth, 20, TRUE); // East
                if (s_appData->hEastBC) MoveWindow(s_appData->hEastBC, eastStartX + bcLabelWidth, bcStartY-2, bcComboWidth, 60, TRUE);
                // Calculate available space for East value field to prevent overflow
                int eastValueWidth = leftWidth - 15 - (eastStartX + bcLabelWidth + bcComboWidth + 5);
                if (eastValueWidth > 0) {
                    if (s_appData->hEastValue) MoveWindow(s_appData->hEastValue, eastStartX + bcLabelWidth + bcComboWidth + 5, bcStartY-2, eastValueWidth, 22, TRUE);
                }

                // South boundary controls
                if (s_appData->hBCLabels[2]) MoveWindow(s_appData->hBCLabels[2], leftStart + 10, bcStartY + 35, bcLabelWidth, 20, TRUE); // South
                if (s_appData->hSouthBC) MoveWindow(s_appData->hSouthBC, leftStart + 10 + bcLabelWidth, bcStartY + 33, bcComboWidth, 60, TRUE);
                if (westValueWidth > 0) {
                    if (s_appData->hSouthValue) MoveWindow(s_appData->hSouthValue, leftStart + 10 + bcLabelWidth + bcComboWidth + 10, bcStartY + 33, westValueWidth, 22, TRUE);
                }

                // North boundary controls
                if (s_appData->hBCLabels[3]) MoveWindow(s_appData->hBCLabels[3], eastStartX, bcStartY + 35, bcLabelWidth, 20, TRUE); // North
                if (s_appData->hNorthBC) MoveWindow(s_appData->hNorthBC, eastStartX + bcLabelWidth, bcStartY + 33, bcComboWidth, 60, TRUE);
                if (eastValueWidth > 0) {
                    if (s_appData->hNorthValue) MoveWindow(s_appData->hNorthValue, eastStartX + bcLabelWidth + bcComboWidth + 5, bcStartY + 33, eastValueWidth, 22, TRUE);
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