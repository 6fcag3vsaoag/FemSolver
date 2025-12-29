#ifndef GUIAPPTYPES_H
#define GUIAPPTYPES_H

#include "Types.h"  // Include the basic types
#include <windows.h>  // Include Windows headers for HWND
#include <string>

// Structure to hold application data (moved from GUIApp.cpp)
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
    class FemSolver* solver;
};

#endif // GUIAPPTYPES_H