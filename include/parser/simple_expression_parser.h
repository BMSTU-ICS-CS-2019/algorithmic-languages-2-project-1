#ifndef INCLUDE_SIMPLE_EXPRESSION_PARSER_H_
#define INCLUDE_SIMPLE_EXPRESSION_PARSER_H_

#include <algorithm>
#include <boost/multiprecision/cpp_int.hpp>
#include <iostream> // TODO remove
#include <map>
#include <operation/algebraic_operations.h>
#include <operation/const_operation.h>
#include <parser/expression_parser.h>
#include <queue>
#include <stack>
#include <string_view>
#include <variant>
#include <vector>

using BigDecimal = boost::multiprecision::cpp_rational;

namespace calculator {

    // based on: https://en.wikipedia.org/wiki/Shunting-yard_algorithm
    namespace algorithm {
        typedef std::shared_ptr<Operation<BigDecimal>> OperationPointer;

        enum class NodeType { Operator, Operand };

        enum OperatorType {
            LEFT_BRACE,
            RIGHT_BRACE, // may be unused
            PLUS,
            MINUS,
            MULTIPLY,
            DIVIDE,
            MODULO,
            SQRT,
            EXP,
            POW,
            SIN,
            COS,
            TG,
            CTG,
            ASIN,
            ACOS,
            ATG,
            ACTG
        };

        struct Operator {
            OperatorType type;
            int priority;
            bool leftAssociative;
        };

        struct Context {
            typedef std::variant<std::shared_ptr<Operation<BigDecimal>>, OperatorType> OperandOrOperator;

            std::queue<OperandOrOperator> output;
            std::stack<Operator> operators;

            void pushValue(std::shared_ptr<Operation<BigDecimal>>&& value) { output.push(std::move(value)); }

            void pushConstant(BigDecimal&& number) {
                pushValue(std::make_shared<ConstOperation<BigDecimal>>(std::move(number)));
            }

            void pushVariable(char const name) { pushValue(std::make_shared<VariableOperation<BigDecimal>>(name)); }

            void pushFunction(OperatorType type) { operators.push({type, INT_MAX, false}); }

            void pushOperator(Operator const& pushedOperator) {
                std::cout << "\t\t\tpushop" << pushedOperator.type << std::endl;
                Operator topOperator;
                while (!operators.empty() && ((topOperator = operators.top()).type != LEFT_BRACE)
                       && ((topOperator.priority > pushedOperator.priority)
                           || (topOperator.priority == pushedOperator.priority && topOperator.leftAssociative))) {
                    operators.pop();
                    output.push(topOperator.type);
                }
                operators.push(pushedOperator);
            }

            void pushLeftBracket() { operators.push({LEFT_BRACE, INT_MAX, false}); }

            void pushRightBracket() {
                while (!operators.empty()) {
                    // handle top operator
                    auto const topOperator = operators.top();
                    if (topOperator.type == LEFT_BRACE) return;
                    output.push(topOperator.type);

                    operators.pop(); // go to the next operator
                }
                // this can be reached only in case of imbalanced parentheses
                if (operators.empty()) throw new InvalidExpression("Imbalanced parentheses in expression");
            }

            void flushRemaining() {
                while (!operators.empty()) {
                    output.push(operators.top().type);
                    operators.pop();
                }
            }

            OperationPointer createOperation() {
                struct Operands {
                    std::stack<OperationPointer> operands_;

                    void push(OperationPointer&& operation) { operands_.push(std::move(operation)); }

                    void push(OperationPointer const& operation) { operands_.push(operation); }

                    OperationPointer pop() {
                        auto popped = operands_.top();
                        operands_.pop();

                        return popped;
                    }

                    bool empty() const { return operands_.empty(); }
                } operands;

                if (output.empty()) throw InvalidExpression("An empty sub-expression");

                while (!output.empty()) {
                    auto const either = output.front();
                    output.pop();
                    std::cout << '(' << (either.index()) << ')'
                              << (either.index() == 0 ? std::get<0>(either)->result(Variables<BigDecimal>({}))
                                                      : std::get<1>(either))
                              << ' ';
                    if (either.index() == 0) operands.push(std::get<0>(either)); // operand
                    else {                                                       // operator
                        switch (std::get<1>(either)) {
                            case LEFT_BRACE: throw InvalidExpression("Imbalanced parentheses");
                            case RIGHT_BRACE: throw std::runtime_error("Parser has produced a right brace token");
                            case PLUS: {
                                operands.push(
                                        std::make_shared<PlusOperation<BigDecimal>>(operands.pop(), operands.pop()));
                                break;
                            }
                            case MINUS: {
                                operands.push(
                                        std::make_shared<MinusOperation<BigDecimal>>(operands.pop(), operands.pop()));
                                break;
                            }
                            case MULTIPLY: {
                                operands.push(std::make_shared<MultiplyOperation<BigDecimal>>(operands.pop(),
                                                                                              operands.pop()));
                                break;
                            }
                            case DIVIDE: {
                                operands.push(
                                        std::make_shared<DivideOperation<BigDecimal>>(operands.pop(), operands.pop()));
                                break;
                            }
                            case MODULO: {
                                /*operands.push(std::make_shared<ModuloOperation<BigDecimal>>(
                                        operands.pop(), operands.pop()
                                        ));
                                break;*/
                            }
                            case SQRT:
                            case EXP:
                            case POW:
                            case SIN:
                            case COS:
                            case TG:
                            case CTG:
                            case ASIN:
                            case ACOS:
                            case ATG:
                            case ACTG: throw new std::runtime_error("Unsupported operation");
                        }
                    }
                }
                std::cout << std::endl;

                return operands.pop();
            }
        };

        static OperationPointer parseExpression(std::string_view expression) {
            std::cout << "\tHandling single expression: (|" << expression << "|)" << std::endl; // TODO remove

            Context context;

            auto const length = expression.length(), maxIndex = length - 1;
            for (size_t index = 0; index < length; ++index) {
                auto character = expression[index];
                switch (character) { // whole part of the number: (\d*)
                    /*
                     * Numbers
                     */
                    //@formatter:off
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9': {
                        //@formatter:on
                        size_t numberLength = 1;
                        while (index < maxIndex) { // read numbers until non-digit symbol is found
                            if (std::isdigit(character = expression[index + 1]))  {
                                ++numberLength;
                                ++index;
                            }
                            else
                                break;
                        }
                        if (character == '.') {
                            ++numberLength;
                            while (index < maxIndex) { // read numbers until non-digit symbol is found
                                if (std::isdigit(character = expression[index + 1])) {
                                    ++numberLength;
                                    ++index;
                                }
                                else
                                    break;
                            }
                        }

                        context.pushConstant(BigDecimal(expression.substr(index + 1 - numberLength, numberLength)));

                        break;
                    }
                    case '.': {                // decimal part of the number: (\d+)
                        if (index == maxIndex) // no possible digits
                            throw InvalidExpression("Meaningless dot at index " + std::to_string(index));

                        size_t numberLength = 1;
                        while (index < maxIndex) { // read numbers until non-digit symbol is found
                            if (std::isdigit(character = expression[index + 1])) {
                                ++numberLength;
                                ++index;
                            }
                            else
                                break;
                        }
                        if (numberLength == 1) // no digits
                            throw InvalidExpression("Meaningless dot at index " + std::to_string(index));

                        context.pushConstant(BigDecimal(expression.substr(index + 1 - numberLength, numberLength)));

                        break;
                    }
                        /*
                         * Standard functions:
                         * [e]xp []
                         * [s]in [c]os [tg] [ctg]
                         * [a](*)
                         */
                    case 'E':
                    case 'e': {
                        if (index < maxIndex - 2) { // 2 extra symbols (`xp`) + at least one symbol after
                            // continue reading
                            if (((character = expression[index + 1]) == 'X' || character == 'x')
                                && ((character = expression[index + 2]) == 'P' || character == 'p')) {
                                index += 2;
                                // this is an exp function
                                context.pushFunction(OperatorType::EXP);
                                break;
                            }
                        }
                        // this is an e constant
                        context.pushValue(std::make_shared<ConstEOperation<BigDecimal>>());
                        break;
                    }
                    case 'P':
                    case 'p': {
                        if (index < maxIndex) {
                            auto const char1 = expression[character + 1];
                            if (char1 == 'I' || char1 == 'i') {
                                // this is a PI const
                                context.pushValue(std::make_shared<ConstPiOperation<BigDecimal>>());
                                break;
                            }
                        }
                        // this is a variable
                        context.pushVariable(character); // P or p
                        break;
                    }
                    case 'S': // sin or sqrt
                    case 's': {
                        // 2 extra symbols (`in`) + at least one symbol after for `sin`
                        if (index < maxIndex - 2) {
                            auto const char1 = expression[index + 1], char2 = expression[index + 2];
                            if ((char1 == 'I' || char1 == 'i') && (char2 == 'N' || char2 == 'n')) {
                                index += 2;
                                // this is a sin function
                                context.pushFunction(OperatorType::SIN);
                                break;
                            }

                            if (index < maxIndex - 3) {
                                if ((char1 == 'Q' || char1 == 'q') && (char2 == 'R' || char2 == 'r')) {
                                    auto const char3 = expression[index + 3];
                                    if (char3 == 'T' || char3 == 't') {
                                        index += 3;
                                        // this is a sqrt function
                                        context.pushFunction(OperatorType::SQRT);
                                        break;
                                    }
                                }
                            }
                        }
                        // this is a variable
                        context.pushVariable(character); // S or s
                        break;
                    }
                    case 'C':
                    case 'c': {
                        // 2 extra symbols (`in`) + at least one symbol after for `cos` or `ctg`
                        if (index < maxIndex - 2) {
                            auto const char1 = expression[index + 1], char2 = expression[index + 2];
                            if ((char1 == 'O' || char1 == 'o') && (char2 == 'S' || char2 == 's')) {
                                index += 2;
                                // this is a cos function
                                context.pushFunction(OperatorType::COS);
                                break;
                            }

                            if ((char1 == 'T' || char1 == 't') && (char2 == 'G' || char2 == 'g')) {
                                index += 2;
                                // this is a ctg function
                                context.pushFunction(OperatorType::CTG);
                                break;
                            }
                        }
                        // this is a variable
                        context.pushVariable(character); // C or c
                        break;
                    }
                        // TODO add tg
                        // TODO arc-function
                    case '+': {
                        context.pushOperator({PLUS, 0, false});
                        break;
                    }
                        // TODO unary minus
                    case '-': {
                        context.pushOperator({MINUS, 0, false});
                        break;
                    }
                    case '*': {
                        context.pushOperator({MULTIPLY, 1, false});
                        break;
                    }
                    case '/': {
                        context.pushOperator({DIVIDE, 1, false});
                        break;
                    }
                    case '%': {
                        context.pushOperator({MODULO, 1, false});
                        break;
                    }
                    case '^': {
                        context.pushOperator({POW, 2, false});
                        break;
                    }
                    case '(': {
                        context.pushLeftBracket();
                        break;
                    }
                    case ')': {
                        context.pushRightBracket();
                        break;
                    }
                }
            }

            context.flushRemaining();

            return context.createOperation();
        }
    } // namespace algorithm

    template<typename T>
    class SimpleOperationParser final : public ExpressionParser<T> {
        typedef std::shared_ptr<Operation<T>> OperationPointer;

        struct Term {
            std::string_view const expression;
            bool const negative;
        };

        /*
         * 1.) split by pluses and minuses treating minuses as pluses of inverted values
         * 2.) parse each term separately (performing inversion if needed) using Shunting-yard algorithm
         * 3.) Sum terms
         */

        static std::vector<Term> splitToTerms(std::string_view const& expression) {
            if (expression.empty()) return std::vector<Term>();

            std::vector<Term> terms;

            {
                size_t depth = 0, startIndex = 0;
                auto negative = false, empty = true;
                auto const length = expression.length();
                for (size_t index = 0; index < length; ++index) {
                    auto const token = expression[index];
                    if (depth == 0) {
                        if (token == ')')
                            throw InvalidExpression("Imbalanced brackets: a closing bracket was found at index "
                                                    + std::to_string(index));

                        if (token == '(') {
                            ++depth;
                            continue;
                        }

                        bool minus;
                        if (token == '+' || (minus = token == '-')) {
                            if (!empty) {
                                // flush sub-expression
                                terms.push_back({expression.substr(startIndex, index - startIndex), negative});

                                empty = true;
                                startIndex = index + 1;
                                negative = minus;
                            } else {
                                negative ^= minus; // XOR true is inversion
                                ++startIndex;      // omit this symbol
                            }
                            continue;
                        }

                        // regular token => not empty
                        empty = false;
                    } else {
                        // in case of increasing depth, simply not forget about checking depth
                        if (token == ')' && --depth == 0) {
                            // flush sub-expression (omitting brackets)
                            terms.push_back({expression.substr(startIndex + 1, index - startIndex - 1), negative});

                            empty = true;
                            startIndex = index + 1;
                            negative = false;
                            continue;
                        }
                        if (token == '(') ++depth;
                    }
                }

                if (empty)
                    throw InvalidExpression("Missing operand: no right operand provided for "
                                            + std::to_string(expression[length - 1]));

                // flush the last sub-expression
                terms.push_back({expression.substr(startIndex, length - startIndex), negative});
            }

            return terms;
        }

        // parses an expression with no possible reordering
        static OperationPointer parseSingleTermExpression(std::string_view expression) {
            return algorithm::parseExpression(expression);
        }

        // parses an expression with no possible reordering
        static OperationPointer parseSingleTerm(Term term) {
            return term.negative ? std::make_shared<NegativeOperation<T>>(parseSingleTermExpression(term.expression))
                                 : parseSingleTermExpression(term.expression);
        }

        // parse expression knowing that it itself is a valid term (but may consist of multiple terms)
        static OperationPointer parseExpressionOfTerm(std::string_view expression) {
            auto const terms = splitToTerms(expression);

            auto const size = terms.size();
            switch (size) {
                case 0: throw new InvalidExpression("The expression contains an empty term");
                case 1: {
                    // todo handle cases like `(((expr)))`
                    auto const term = terms[0];
                    return parseSingleTerm(term);
                }
                // if there are more than 2 terms then the each may consist of multiple terms
                case 2: {
                    auto const term1 = terms[0], term2 = terms[1];
                    return std::make_shared<PlusOperation<T>>(parseTerm(term1), parseTerm(term2));
                }
                default: {
                    std::vector<OperationPointer> operations(size);
                    std::transform(terms.begin(), terms.end(), operations.begin(), parseSingleTerm);

                    return std::make_shared<VectorSumOperation<T>>(operations);
                }
            }
        }

        static OperationPointer parseTerm(Term term) {
            return term.negative ? std::make_shared<NegativeOperation<T>>(parseExpressionOfTerm(term.expression))
                                 : parseExpressionOfTerm(term.expression);
        }

    public:
        virtual OperationPointer parse(std::istream& input) override {
            std::string line;
            getline(input, line);
            {
                auto const end = line.end();
                line.erase(std::remove_if(line.begin(), end, isspace), end);
            }

            return parseExpressionOfTerm(line);
        }
    };
} // namespace calculator


#endif //INCLUDE_SIMPLE_EXPRESSION_PARSER_H_
