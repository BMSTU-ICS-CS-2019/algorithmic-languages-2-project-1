#ifndef INCLUDE_EXPRESSION_PARSER_H_
#define INCLUDE_EXPRESSION_PARSER_H_

#include <istream>
#include <memory>
#include <operation/operation.h>
#include <stdexcept>

namespace calculator {

    class InvalidExpression : std::invalid_argument {
    public:
        InvalidExpression(const std::string& arg) : invalid_argument(arg) {}

        InvalidExpression(const char* string) : invalid_argument(string) {}

        InvalidExpression(const InvalidExpression& argument) : invalid_argument(argument) {}

        InvalidExpression(InvalidExpression&& argument) : invalid_argument(argument) {}
    };

    template<typename T>
    class ExpressionParser {
    public:
        virtual std::shared_ptr<Operation<T>> parse(std::istream& input) = 0;
    };
} // namespace calculator

#endif //INCLUDE_EXPRESSION_PARSER_H_
