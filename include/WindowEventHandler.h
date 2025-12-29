#ifndef WINDOWEVENTHANDLER_H
#define WINDOWEVENTHANDLER_H

#include <windows.h>
#include "GUIAppTypes.h"  // For AppData

// Window event handler class to manage window messages
class WindowEventHandler {
public:
    // Main window procedure
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    // Initialize the application data
    static void setAppData(AppData* appData);
    
    // Get the current application data
    static AppData* getAppData();
    
private:
    static AppData* s_appData;  // Static pointer to application data
};

#endif // WINDOWEVENTHANDLER_H