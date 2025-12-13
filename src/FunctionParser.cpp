#include "FunctionParser.h"
#include <cctype>
#include <sstream>
#include <stdexcept>
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

// Simple mathematical expression evaluator
class ExpressionEvaluator {
private:
    std::string expression;
    size_t pos;
    double x_val, y_val;

    char peek() const {
        if (pos >= expression.length()) return 0;
        return expression[pos];
    }

    char get() {
        if (pos >= expression.length()) return 0;
        return expression[pos++];
    }

    void skipWhitespace() {
        while (std::isspace(peek())) get();
    }

    double parseExpression();
    double parseTerm();
    double parseFactor();
    double parsePrimary();

public:
    ExpressionEvaluator(const std::string& expr, double x, double y) 
        : expression(expr), pos(0), x_val(x), y_val(y) {}

    double evaluate();
};

double ExpressionEvaluator::evaluate() {
    pos = 0;
    double result = parseExpression();
    skipWhitespace();
    if (pos < expression.length()) {
        throw std::runtime_error("Unexpected character at end of expression");
    }
    return result;
}

double ExpressionEvaluator::parseExpression() {
    double left = parseTerm();
    
    while (true) {
        skipWhitespace();
        char op = peek();
        if (op == '+' || op == '-') {
            get(); // consume operator
            double right = parseTerm();
            if (op == '+') {
                left += right;
            } else {
                left -= right;
            }
        } else {
            break;
        }
    }
    
    return left;
}

double ExpressionEvaluator::parseTerm() {
    double left = parseFactor();
    
    while (true) {
        skipWhitespace();
        char op = peek();
        if (op == '*' || op == '/') {
            get(); // consume operator
            double right = parseFactor();
            if (op == '*') {
                left *= right;
            } else {
                if (right == 0) throw std::runtime_error("Division by zero");
                left /= right;
            }
        } else {
            break;
        }
    }
    
    return left;
}

double ExpressionEvaluator::parseFactor() {
    skipWhitespace();
    char op = peek();
    if (op == '+' || op == '-') {
        get(); // consume operator
        double right = parsePrimary();
        return (op == '-') ? -right : right;
    }
    return parsePrimary();
}

double ExpressionEvaluator::parsePrimary() {
    skipWhitespace();
    
    // Handle numbers
    if (std::isdigit(peek()) || peek() == '.') {
        std::string numStr;
        while (std::isdigit(peek()) || peek() == '.') {
            numStr += get();
        }
        return std::stod(numStr);
    }
    
    // Handle variables
    if (peek() == 'x') {
        get();
        return x_val;
    }
    if (peek() == 'y') {
        get();
        return y_val;
    }
    
    // Handle functions
    std::string funcName;
    while (std::isalpha(peek()) || peek() == '_') {
        funcName += get();
    }
    
    if (funcName == "sin") {
        if (get() != '(') throw std::runtime_error("Expected '(' after function name");
        double arg = parseExpression();
        if (get() != ')') throw std::runtime_error("Expected ')' after function argument");
        return std::sin(arg);
    } else if (funcName == "cos") {
        if (get() != '(') throw std::runtime_error("Expected '(' after function name");
        double arg = parseExpression();
        if (get() != ')') throw std::runtime_error("Expected ')' after function argument");
        return std::cos(arg);
    } else if (funcName == "tan") {
        if (get() != '(') throw std::runtime_error("Expected '(' after function name");
        double arg = parseExpression();
        if (get() != ')') throw std::runtime_error("Expected ')' after function argument");
        return std::tan(arg);
    } else if (funcName == "exp") {
        if (get() != '(') throw std::runtime_error("Expected '(' after function name");
        double arg = parseExpression();
        if (get() != ')') throw std::runtime_error("Expected ')' after function argument");
        return std::exp(arg);
    } else if (funcName == "log") {
        if (get() != '(') throw std::runtime_error("Expected '(' after function name");
        double arg = parseExpression();
        if (get() != ')') throw std::runtime_error("Expected ')' after function argument");
        return std::log(arg);
    } else if (funcName == "sqrt") {
        if (get() != '(') throw std::runtime_error("Expected '(' after function name");
        double arg = parseExpression();
        if (get() != ')') throw std::runtime_error("Expected ')' after function argument");
        if (arg < 0) throw std::runtime_error("Square root of negative number");
        return std::sqrt(arg);
    } else if (funcName == "abs") {
        if (get() != '(') throw std::runtime_error("Expected '(' after function name");
        double arg = parseExpression();
        if (get() != ')') throw std::runtime_error("Expected ')' after function argument");
        return std::abs(arg);
    } else if (funcName == "pi") {
        return M_PI;
    } else if (funcName.empty()) {
        // Handle parentheses
        if (peek() == '(') {
            get(); // consume '('
            double result = parseExpression();
            if (get() != ')') throw std::runtime_error("Expected ')'");
            return result;
        }
    }
    
    throw std::runtime_error("Unknown function or variable: " + funcName);
}

// Implementation of FunctionParser methods
CoefficientFunction FunctionParser::parseFunction(const std::string& funcStr) {
    if (funcStr.empty()) {
        return [](double, double) -> double { return 0.0; };
    }
    
    return [funcStr](double x, double y) -> double {
        try {
            ExpressionEvaluator evaluator(funcStr, x, y);
            return evaluator.evaluate();
        } catch (...) {
            return 0.0; // Return 0.0 on error
        }
    };
}

double FunctionParser::safeEval(const std::string& expression, double x, double y) {
    try {
        ExpressionEvaluator evaluator(expression, x, y);
        return evaluator.evaluate();
    } catch (...) {
        return 0.0; // Return 0.0 on error
    }
}

bool FunctionParser::isValidExpression(const std::string& expression) {
    // Basic validation - just check for obviously invalid characters
    // In a real implementation, you'd want more sophisticated validation
    return !expression.empty();
}