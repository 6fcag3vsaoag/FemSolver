#define NOMINMAX
#include "GdiVisualizer.h"
#include "StringUtils.h" // For string conversion utilities
#include <algorithm> // For std::min_element, std::max_element
#include <sstream>   // For std::wostringstream
#include <iomanip>   // For std::fixed, std::setprecision

GdiVisualizer::GdiVisualizer() : hwndTarget_(nullptr), Nx_(0), Ny_(0), hasSolution_(false) {}

void GdiVisualizer::setWindowHandle(HWND handle) {
    hwndTarget_ = handle;
}

void GdiVisualizer::render(const Mesh& mesh, const std::vector<double>& solution, int Nx, int Ny, const std::string& title) {
    if (mesh.nodes.empty() || solution.empty() || !hwndTarget_) {
        // No valid data or target window, clear previous solution and set flag
        hasSolution_ = false;
        mesh_ = Mesh(); // Clear mesh
        solution_ = std::vector<double>(); // Clear solution
        Nx_ = 0;
        Ny_ = 0;
        title_ = "";
    } else {
        mesh_ = mesh;
        solution_ = solution;
        Nx_ = Nx;
        Ny_ = Ny;
        title_ = title;
        hasSolution_ = true;
    }

    // Trigger a repaint of the target window to call drawInternal via WNDPROC
    if (hwndTarget_) {
        InvalidateRect(hwndTarget_, NULL, TRUE);
        UpdateWindow(hwndTarget_); // Force immediate redraw
    }
}

// This function contains the actual GDI drawing logic.
// It will be called from VisualFrameWndProc's WM_PAINT handler.
void GdiVisualizer::drawInternal(HDC hdc, const RECT& rect) {
    // Fill background
    HBRUSH bgBrush = CreateSolidBrush(RGB(240, 240, 240));
    FillRect(hdc, &rect, bgBrush);
    DeleteObject(bgBrush);

    // Visualize the solution
    if (hasSolution_ && !solution_.empty() && Nx_ > 0 && Ny_ > 0) {
        // Draw title
        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        std::wstring currentTitle = utf8_to_wstring(title_);
        if (currentTitle.empty()) currentTitle = L"Solution Visualization";
        TextOutW(hdc, 10, 10, currentTitle.c_str(), static_cast<int>(currentTitle.length()));

        // Draw solution stats
        std::wostringstream wstats;
        wstats << L"Nodes: " << solution_.size()
                << L", Elements: " << mesh_.elements.size();
        TextOutW(hdc, 10, 30, wstats.str().c_str(), static_cast<int>(wstats.str().length()));

        // Draw a simplified visualization
        int visChartLeft = 10;
        int visChartTop = 60;
        int visChartWidth = rect.right - 20;
        int visChartHeight = rect.bottom - 80;

        if (visChartWidth > 20 && visChartHeight > 20) {
            // Draw bounding rectangle
            Rectangle(hdc, visChartLeft, visChartTop, visChartLeft + visChartWidth, visChartTop + visChartHeight);

            if (solution_.size() >= 4) {
                double minVal = *std::min_element(solution_.begin(), solution_.end());
                double maxVal = *std::max_element(solution_.begin(), solution_.end());
                double range = (maxVal == minVal) ? 1.0 : (maxVal - minVal);

                int gridX = (Nx_ < 50) ? Nx_ : 50; // Use actual Nx, Ny from members
                int gridY = (Ny_ < 50) ? Ny_ : 50;

                int cellWidth = visChartWidth / gridX;
                int cellHeight = visChartHeight / gridY;

                if (cellWidth >= 1 && cellHeight >= 1) {
                    for (int y_map_idx = 0; y_map_idx < gridY; ++y_map_idx) {
                        for (int x_map_idx = 0; x_map_idx < gridX; ++x_map_idx) {
                            // Map gridX, gridY back to original Nx, Ny for solution indexing
                            int x_original = static_cast<int>((double)x_map_idx / gridX * Nx_);
                            int y_original = static_cast<int>((double)y_map_idx / gridY * Ny_);
                            x_original = std::min(x_original, Nx_ - 1); // Clamp to avoid out of bounds
                            y_original = std::min(y_original, Ny_ - 1); // Clamp to avoid out of bounds

                            int original_idx = y_original * Nx_ + x_original;
                            if (original_idx >= 0 && original_idx < static_cast<int>(solution_.size())) {
                                double val = solution_[original_idx];
                                double normVal = (range != 0) ? (val - minVal) / range : 0.0;

                                int r = static_cast<int>(normVal * 255);
                                int b = static_cast<int>((1.0 - normVal) * 255);
                                int g = 50; // Use a constant green component, similar to original

                                HBRUSH cellBrush = CreateSolidBrush(RGB(r, g, b));
                                RECT cellRect;
                                cellRect.left = visChartLeft + x_map_idx * cellWidth;
                                cellRect.top = visChartTop + y_map_idx * cellHeight;
                                cellRect.right = cellRect.left + cellWidth;
                                cellRect.bottom = cellRect.top + cellHeight;

                                FillRect(hdc, &cellRect, cellBrush);
                                FrameRect(hdc, &cellRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
                                DeleteObject(cellBrush);
                            }
                        }
                    }
                }

                int legendX = visChartLeft + visChartWidth + 5;
                int legendY = visChartTop;
                int legendHeight = 100;
                int legendWidth = 20;

                if (legendY + legendHeight < rect.bottom && legendX + legendWidth < rect.right) {
                    for (int i = 0; i < legendHeight; i++) {
                        double ratio = static_cast<double>(i) / legendHeight;
                        int r = static_cast<int>(ratio * 255);
                        int b = static_cast<int>((1.0 - ratio) * 255);
                        int g = 50; // Constant green component

                        HBRUSH legendBrush = CreateSolidBrush(RGB(r, g, b));
                        RECT legendRect;
                        legendRect.left = legendX;
                        legendRect.top = legendY + i;
                        legendRect.right = legendX + legendWidth;
                        legendRect.bottom = legendY + i + 1;
                        FillRect(hdc, &legendRect, legendBrush);
                        DeleteObject(legendBrush);
                    }

                    std::wostringstream lowLabel;
                    lowLabel << std::fixed << std::setprecision(2) << minVal;
                    std::wostringstream highLabel;
                    highLabel << std::fixed << std::setprecision(2) << maxVal;

                    TextOutW(hdc, legendX + legendWidth + 5, legendY, lowLabel.str().c_str(), static_cast<int>(lowLabel.str().length()));
                    TextOutW(hdc, legendX + legendWidth + 5, legendY + legendHeight - 15, highLabel.str().c_str(), static_cast<int>(highLabel.str().length()));
                }
            }
        }
    } else {
        SetTextColor(hdc, RGB(128, 128, 128));
        std::wstring placeholder = L"No solution computed yet.\nClick 'Solve' to compute.";
        RECT placeholderRect = {10, 10, rect.right - 10, rect.bottom - 10};
        DrawTextW(hdc, placeholder.c_str(), -1, &placeholderRect, DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX);
    }
}
