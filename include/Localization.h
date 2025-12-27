#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <string>

// Forward declaration for Windows HWND
struct HWND__;
typedef HWND__* HWND;

// Language enum
enum class Language {
    English,
    Russian
};

// Interface for language strategy using wide strings
class ILanguageStrategy {
public:
    virtual ~ILanguageStrategy() = default;
    virtual const wchar_t* getEquationPreset() const = 0;
    virtual const wchar_t* getSolveBtn() const = 0;
    virtual const wchar_t* getResetBtn() const = 0;
    virtual const wchar_t* getExportBtn() const = 0;
    virtual const wchar_t* getCoeffGroup() const = 0;
    virtual const wchar_t* getBcGroup() const = 0;
    virtual const wchar_t* getSolInfoGroup() const = 0;
    virtual const wchar_t* getPresetOption(int index) const = 0;
    virtual const wchar_t* getBcType(int index) const = 0;
    virtual const wchar_t* getStatusReady() const = 0;
    virtual const wchar_t* getStatusPresetLoaded() const = 0;
    virtual const wchar_t* getStatusSolved() const = 0;
    virtual Language getLanguageType() const = 0;
};

// English language strategy
class EnglishLanguageStrategy : public ILanguageStrategy {
public:
    const wchar_t* getEquationPreset() const override { return L"Equation Preset:"; }
    const wchar_t* getSolveBtn() const override { return L"Solve"; }
    const wchar_t* getResetBtn() const override { return L"Reset"; }
    const wchar_t* getExportBtn() const override { return L"Export"; }
    const wchar_t* getCoeffGroup() const override { return L"Equation Coefficients"; }
    const wchar_t* getBcGroup() const override { return L"Boundary Conditions"; }
    const wchar_t* getSolInfoGroup() const override { return L"Solution Information"; }
    const wchar_t* getPresetOption(int index) const override {
        static const wchar_t* options[] = {
            L"Select Preset...", L"Laplace Equation", L"Poisson Equation",
            L"Helmholtz Equation", L"Convection-Diffusion", L"Reaction-Diffusion", L"General Elliptic"
        };
        return (index >= 0 && index < 7) ? options[index] : L"";
    }
    const wchar_t* getBcType(int index) const override {
        static const wchar_t* types[] = { L"Dirichlet", L"Neumann" };
        return (index >= 0 && index < 2) ? types[index] : L"";
    }
    const wchar_t* getStatusReady() const override { return L"Ready - Select an equation preset to begin"; }
    const wchar_t* getStatusPresetLoaded() const override { return L"Preset loaded. Ready to solve."; }
    const wchar_t* getStatusSolved() const override { return L"Solution computed successfully!"; }
    Language getLanguageType() const override { return Language::English; }
};

// Russian language strategy
class RussianLanguageStrategy : public ILanguageStrategy {
public:
    const wchar_t* getEquationPreset() const override { return L"Предустановка уравнения:"; }
    const wchar_t* getSolveBtn() const override { return L"Решить"; }
    const wchar_t* getResetBtn() const override { return L"Сброс"; }
    const wchar_t* getExportBtn() const override { return L"Экспорт"; }
    const wchar_t* getCoeffGroup() const override { return L"Коэффициенты уравнения"; }
    const wchar_t* getBcGroup() const override { return L"Граничные условия"; }
    const wchar_t* getSolInfoGroup() const override { return L"Информация о решении"; }
    const wchar_t* getPresetOption(int index) const override {
        static const wchar_t* options[] = {
            L"Выберите предустановку...", L"Уравнение Лапласа", L"Уравнение Пуассона",
            L"Уравнение Гельмгольца", L"Конвективно-диффузионное", L"Реакционно-диффузионное", L"Общее эллиптическое"
        };
        return (index >= 0 && index < 7) ? options[index] : L"";
    }
    const wchar_t* getBcType(int index) const override {
        static const wchar_t* types[] = { L"Дирихле", L"Нейман" };
        return (index >= 0 && index < 2) ? types[index] : L"";
    }
    const wchar_t* getStatusReady() const override { return L"Готово - Выберите предустановку уравнения для начала"; }
    const wchar_t* getStatusPresetLoaded() const override { return L"Предустановка загружена. Готов к решению."; }
    const wchar_t* getStatusSolved() const override { return L"Решение вычислено успешно!"; }
    Language getLanguageType() const override { return Language::Russian; }
};

// Context for language strategy
class LanguageContext {
private:
    ILanguageStrategy* strategy;
public:
    LanguageContext(ILanguageStrategy* s) : strategy(s) {}
    void setStrategy(ILanguageStrategy* s) { strategy = s; }
    const wchar_t* getEquationPreset() const { return strategy->getEquationPreset(); }
    const wchar_t* getSolveBtn() const { return strategy->getSolveBtn(); }
    const wchar_t* getResetBtn() const { return strategy->getResetBtn(); }
    const wchar_t* getExportBtn() const { return strategy->getExportBtn(); }
    const wchar_t* getCoeffGroup() const { return strategy->getCoeffGroup(); }
    const wchar_t* getBcGroup() const { return strategy->getBcGroup(); }
    const wchar_t* getSolInfoGroup() const { return strategy->getSolInfoGroup(); }
    const wchar_t* getPresetOption(int index) const { return strategy->getPresetOption(index); }
    const wchar_t* getBcType(int index) const { return strategy->getBcType(index); }
    const wchar_t* getStatusReady() const { return strategy->getStatusReady(); }
    const wchar_t* getStatusPresetLoaded() const { return strategy->getStatusPresetLoaded(); }
    const wchar_t* getStatusSolved() const { return strategy->getStatusSolved(); }
    Language getLanguageType() const { return strategy->getLanguageType(); }
};

// Global functions for localization
LanguageContext& getLanguageContext();
EnglishLanguageStrategy& getEnglishStrategy();
RussianLanguageStrategy& getRussianStrategy();
void switchLanguage();
Language getCurrentLanguage();

#endif // LOCALIZATION_H