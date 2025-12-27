#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <windows.h> // For WideCharToMultiByte and MultiByteToWideChar

/**
 * @brief Converts a wide string (wstring) to a UTF-8 encoded string.
 * @param wstr The wide string to convert.
 * @return The UTF-8 encoded string.
 */
inline std::string wstring_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

/**
 * @brief Converts a UTF-8 encoded string to a wide string (wstring).
 * @param str The UTF-8 encoded string to convert.
 * @return The wide string.
 */
inline std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

#endif // STRINGUTILS_H
