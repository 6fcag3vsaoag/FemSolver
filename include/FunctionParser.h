#ifndef FUNCTIONPARSER_H
#define FUNCTIONPARSER_H

#include "Types.h"
#include <string>
#include <functional>
#include <map>

class FunctionParser {
public:
    FunctionParser() = default;
    ~FunctionParser() = default;

    // Parse and return a function from a string expression
    static CoefficientFunction parseFunction(const std::string& funcStr);

    // Safe evaluation of mathematical expressions
    static double safeEval(const std::string& expression, double x, double y);

private:
    // Helper function to validate expressions
    static bool isValidExpression(const std::string& expression);
};

#endif // FUNCTIONPARSER_H