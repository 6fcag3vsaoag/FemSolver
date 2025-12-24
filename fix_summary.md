The performance issue was caused by an incorrectly implemented `WM_PAINT` handler in `src/GUIApp.cpp`. The original code attempted to handle the painting for a child control (`hVisualFrame`) within the main window's `WndProc` in a way that prevented the main window's update region from being validated. This led to a continuous stream of `WM_PAINT` messages, causing a busy-wait loop and high CPU usage even when the application was idle.

To fix this, I made the following changes in `src/GUIApp.cpp`:

1.  **Subclassed the `hVisualFrame` control:** I introduced a new global `WNDPROC` pointer (`g_pfnOldVisualFrameProc`) and a custom window procedure `VisualFrameWndProc`.
2.  **Moved custom drawing logic:** The entire custom drawing code for the visualization frame was moved from the main `WndProc`'s `WM_PAINT` handler into the new `VisualFrameWndProc`.
3.  **Applied subclassing:** In the `CreateControls` function, after creating `hVisualFrame`, I subclassed it using `SetWindowLongPtr` to direct its messages to `VisualFrameWndProc`.
4.  **Corrected main `WM_PAINT` handler:** The `WM_PAINT` handler in the main `WndProc` was simplified. It now correctly calls `BeginPaint` and `EndPaint` for the main window, ensuring its update region is validated. This stops the infinite `WM_PAINT` loop and resolves the high CPU usage.

This approach ensures that each window handles its own painting correctly, eliminating the busy-wait and improving application performance.