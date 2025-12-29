#include "../include/Localization.h"
#include <windows.h>

// Global language context using strategy pattern
static EnglishLanguageStrategy englishStrategy;
static LanguageContext langContext(&englishStrategy);
static RussianLanguageStrategy russianStrategy;

// Function to get the global language context
LanguageContext& getLanguageContext() {
    return langContext;
}

// Function to get the global English strategy
EnglishLanguageStrategy& getEnglishStrategy() {
    return englishStrategy;
}

// Function to get the global Russian strategy
RussianLanguageStrategy& getRussianStrategy() {
    return russianStrategy;
}

// Function to switch between languages using strategy pattern
void switchLanguage() {
    if (getLanguageContext().getLanguageType() == Language::English) {
        getLanguageContext().setStrategy(&getRussianStrategy());
    } else {
        getLanguageContext().setStrategy(&getEnglishStrategy());
    }
}

// Function to get the current language
Language getCurrentLanguage() {
    return getLanguageContext().getLanguageType();
}