#ifndef PRESETMANAGER_H
#define PRESETMANAGER_H

#include <windows.h>
#include <string>

// Include GUIAppTypes.h for AppData structure
#include "GUIAppTypes.h"

// Structure to hold preset data
struct PresetData {
    const wchar_t* name;
    const wchar_t* description_en;
    const wchar_t* description_ru;
    const wchar_t* lx, * ly, * nx, * ny;
    const wchar_t* a11, * a12, * a22, * b1, * b2, * c, * f;
    const wchar_t* westValue, * eastValue, * southValue, * northValue;
};

// Preset manager class to handle preset functionality
class PresetManager {
public:
    // Get the number of presets
    static int getNumPresets();

    // Get preset data by index
    static const PresetData& getPreset(int index);

    // Show help for a specific preset
    static void showPresetHelp(HWND hwnd, int presetIndex);

    // Load a preset into the UI
    static void loadPreset(HWND hwnd, AppData& appData, int presetIndex);

    // Handle preset change event
    static void onPresetChanged(HWND hwnd, AppData& appData, int presetIndex);

    // Handle preset help click
    static void onPresetHelpClicked(HWND hwnd, AppData& appData);
};

#endif // PRESETMANAGER_H