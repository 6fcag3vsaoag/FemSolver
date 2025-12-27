#include "../include/PresetManager.h"
#include "../include/Localization.h"
#include <windows.h>
#include <sstream>

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
      L"This preset showcases a complex scenario with variable and anisotropic diffusion, convection in both x and y directions, a reaction term and a source. This can model complex physical systems such as heat transfer in a moving, heterogeneous medium.",
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

// Get the number of presets
int PresetManager::getNumPresets() {
    return NUM_PRESETS;
}

// Get preset data by index
const PresetData& PresetManager::getPreset(int index) {
    // Adjust index to be 0-based for array access
    if (index < 0 || index >= NUM_PRESETS) {
        // Return the first preset as default if index is out of bounds
        return PRESETS[0];
    }
    return PRESETS[index];
}

// Show help for a specific preset
void PresetManager::showPresetHelp(HWND hwnd, int presetIndex) {
    if (presetIndex > 0 && presetIndex <= NUM_PRESETS) {
        int idx = presetIndex - 1;
        const PresetData& preset = PRESETS[idx];
        const wchar_t* description = (getLanguageContext().getLanguageType() == Language::Russian) ? preset.description_ru : preset.description_en;
        MessageBoxW(hwnd, description, preset.name, MB_OK | MB_ICONINFORMATION);
    } else {
        const wchar_t* msg = (getLanguageContext().getLanguageType() == Language::Russian) ? L"Выберите предустановку, чтобы увидеть справку." : L"Please select a preset to see its help information.";
        const wchar_t* title = (getLanguageContext().getLanguageType() == Language::Russian) ? L"Справка" : L"Help";
        MessageBoxW(hwnd, msg, title, MB_OK | MB_ICONINFORMATION);
    }
}

// Forward declaration for AppData (this would normally be in a shared header)
struct AppData;

// Load a preset into the UI
void PresetManager::loadPreset(HWND hwnd, AppData& appData, int presetIndex) {
    // Adjust presetIndex to be 0-based for array access (since combo box starts from 1)
    if (presetIndex < 1 || presetIndex > NUM_PRESETS) {
        return; // Invalid preset index
    }

    // Convert to 0-based index
    int idx = presetIndex - 1;
    const PresetData& preset = PRESETS[idx];

    // Set all values using the preset data
    SetWindowTextW(appData.hLxEdit, preset.lx);
    SetWindowTextW(appData.hLyEdit, preset.ly);
    SetWindowTextW(appData.hNxEdit, preset.nx);
    SetWindowTextW(appData.hNyEdit, preset.ny);
    SetWindowTextW(appData.hA11Edit, preset.a11);
    SetWindowTextW(appData.hA12Edit, preset.a12);
    SetWindowTextW(appData.hA22Edit, preset.a22);
    SetWindowTextW(appData.hB1Edit, preset.b1);
    SetWindowTextW(appData.hB2Edit, preset.b2);
    SetWindowTextW(appData.hCEdit, preset.c);
    SetWindowTextW(appData.hFEdit, preset.f);
    SetWindowTextW(appData.hWestValue, preset.westValue);
    SetWindowTextW(appData.hEastValue, preset.eastValue);
    SetWindowTextW(appData.hSouthValue, preset.southValue);
    SetWindowTextW(appData.hNorthValue, preset.northValue);

    // Update the solution info panel to reflect current values
    wchar_t buffer[256];
    std::wostringstream solutionInfo;
    solutionInfo << L"Current Configuration:\n";
    solutionInfo << L"-------------------\n";

    // Get current values from UI
    GetWindowTextW(appData.hLxEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"Domain X: [0, " << buffer << L"]\n";
    GetWindowTextW(appData.hLyEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"Domain Y: [0, " << buffer << L"]\n";
    GetWindowTextW(appData.hNxEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"X nodes: " << buffer << L"\n";
    GetWindowTextW(appData.hNyEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"Y nodes: " << buffer << L"\n\n";

    solutionInfo << L"Coefficients:\n";
    GetWindowTextW(appData.hA11Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  a11(x,y) = " << buffer << L"\n";
    GetWindowTextW(appData.hA12Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  a12(x,y) = " << buffer << L"\n";
    GetWindowTextW(appData.hA22Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  a22(x,y) = " << buffer << L"\n";
    GetWindowTextW(appData.hB1Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  b1(x,y) = " << buffer << L"\n";
    GetWindowTextW(appData.hB2Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  b2(x,y) = " << buffer << L"\n";
    GetWindowTextW(appData.hCEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  c(x,y) = " << buffer << L"\n";
    GetWindowTextW(appData.hFEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
    solutionInfo << L"  f(x,y) = " << buffer << L"\n";

    SetWindowTextW(appData.hSolutionInfo, solutionInfo.str().c_str());

    // Clear any stored solution data and visualization
    // This would require access to the visualizer, which we don't have here
    // So this part would need to be handled in the GUIApp

    // Force immediate update of all affected controls to ensure visual update
    // Use both SetWindowPos with SWP_FRAMECHANGED and RedrawWindow to ensure update
    RedrawWindow(appData.hLxEdit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hLyEdit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hNxEdit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hNyEdit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hA11Edit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hA12Edit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hA22Edit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hB1Edit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hB2Edit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hCEdit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hFEdit, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hWestValue, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hEastValue, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hSouthValue, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hNorthValue, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    RedrawWindow(appData.hSolutionInfo, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

    // Force refresh of the visualization frame to clear any stale display
    InvalidateRect(appData.hVisualFrame, NULL, TRUE);
}

// Handle preset change event
void PresetManager::onPresetChanged(HWND hwnd, AppData& appData, int presetIndex) {
    if (presetIndex > 0) { // Skip the "Select Preset..." option
        loadPreset(hwnd, appData, presetIndex);

        // Update status to show preset is loaded
        SetWindowTextW(appData.hStatus, getLanguageContext().getStatusPresetLoaded());

        // Update the solution info panel to show current values
        wchar_t buffer[256];
        std::wostringstream solutionInfo;
        solutionInfo << L"Current Configuration:\n";
        solutionInfo << L"-------------------\n";

        // Get current values from UI
        GetWindowTextW(appData.hLxEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"Domain X: [0, " << buffer << L"]\n";
        GetWindowTextW(appData.hLyEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"Domain Y: [0, " << buffer << L"]\n";
        GetWindowTextW(appData.hNxEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"X nodes: " << buffer << L"\n";
        GetWindowTextW(appData.hNyEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"Y nodes: " << buffer << L"\n\n";

        solutionInfo << L"Coefficients:\n";
        GetWindowTextW(appData.hA11Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  a11(x,y) = " << buffer << L"\n";
        GetWindowTextW(appData.hA12Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  a12(x,y) = " << buffer << L"\n";
        GetWindowTextW(appData.hA22Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  a22(x,y) = " << buffer << L"\n";
        GetWindowTextW(appData.hB1Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  b1(x,y) = " << buffer << L"\n";
        GetWindowTextW(appData.hB2Edit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  b2(x,y) = " << buffer << L"\n";
        GetWindowTextW(appData.hCEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  c(x,y) = " << buffer << L"\n";
        GetWindowTextW(appData.hFEdit, buffer, sizeof(buffer)/sizeof(wchar_t));
        solutionInfo << L"  f(x,y) = " << buffer << L"\n";

        SetWindowTextW(appData.hSolutionInfo, solutionInfo.str().c_str());

        // Force refresh of all controls to ensure visual update
        UpdateWindow(appData.hLxEdit);
        UpdateWindow(appData.hLyEdit);
        UpdateWindow(appData.hNxEdit);
        UpdateWindow(appData.hNyEdit);
        UpdateWindow(appData.hA11Edit);
        UpdateWindow(appData.hA12Edit);
        UpdateWindow(appData.hA22Edit);
        UpdateWindow(appData.hB1Edit);
        UpdateWindow(appData.hB2Edit);
        UpdateWindow(appData.hCEdit);
        UpdateWindow(appData.hFEdit);
        UpdateWindow(appData.hWestValue);
        UpdateWindow(appData.hEastValue);
        UpdateWindow(appData.hSouthValue);
        UpdateWindow(appData.hNorthValue);
        UpdateWindow(appData.hSolutionInfo);

        // Force a refresh of the visualization area to show the new state
        InvalidateRect(appData.hVisualFrame, NULL, TRUE);
        UpdateWindow(appData.hVisualFrame);
    }
}

// Handle preset help click
void PresetManager::onPresetHelpClicked(HWND hwnd, AppData& appData) {
    LRESULT selResult = SendMessage(appData.hPresetCombo, CB_GETCURSEL, 0, 0);
    int selection = static_cast<int>(selResult);

    showPresetHelp(hwnd, selection);
}