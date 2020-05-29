#ifndef INCLUDE_SIMPLE_EXPRESSION_PARSER_H_
#define INCLUDE_SIMPLE_EXPRESSION_PARSER_H_

#include <algorithm>
#include <boost/convert.hpp>
#include <boost/convert/strtol.hpp>
#include <boost/multiprecision/cpp_int.hpp>
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

    template<typename T>
    class SimpleOperationParser final : public ExpressionParser<T> {
        typedef std::shared_ptr<Operation<T>> OperationPointer;

        /*
         * Type conversions

        template<typename C>
        C fromStringView(std::string_view const& number) {
            struct None {};
            static_assert(std::is_same<T, C>::value, "Type is not convertible");
        }

        template<>
        double fromStringView<double>(std::string_view const& number) {
            return boost::convert<double>(number, boost::cnv::strtol());
        }

        template<>
        BigDecimal fromStringView<BigDecimal>(std::string_view const& number) {
            return BigDecimal(number);
        }
         */

        /*
         * Parsing
         */

        enum class OperatorType : uint8_t {
            LEFT_BRACE,
            RIGHT_BRACE, // may be unused
            INVERT,
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

        // based on: https://en.wikipedia.org/wiki/Shunting-yard_algorithm
        struct Context final {
            typedef std::variant<OperationPointer, OperatorType> OperandOrOperator;

            std::queue<OperandOrOperator> output;
            std::stack<Operator> operators;

        public:
            void pushValue(std::shared_ptr<Operation<T>>&& value) { output.push(std::move(value)); }

            void pushConstant(T&& number) { pushValue(std::make_shared<ConstOperation<T>>(std::move(number))); }

            void pushVariable(char const name) { pushValue(std::make_shared<VariableOperation<T>>(name)); }

            void pushFunction(OperatorType type) { operators.push({type, INT_MAX, false}); }

            void pushOperator(Operator const& pushedOperator) {
                Operator topOperator;
                while (!operators.empty() && ((topOperator = operators.top()).type != OperatorType::LEFT_BRACE)
                       && ((topOperator.priority > pushedOperator.priority)
                           || (topOperator.priority == pushedOperator.priority && topOperator.leftAssociative))) {
                    operators.pop();
                    output.push(topOperator.type);
                }
                operators.push(pushedOperator);
            }

            void pushLeftBracket() { operators.push({OperatorType::LEFT_BRACE, INT_MAX, false}); }

            void pushRightBracket() {
                while (!operators.empty()) {
                    // handle top operator
                    auto const topOperator = operators.top();
                    if (topOperator.type == OperatorType::LEFT_BRACE) return;
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
                        if (operands_.empty()) throw InvalidExpression("Missing operand for expression");

                        auto const popped = operands_.top();
                        operands_.pop();

                        return popped;
                    }
                } operands;

                if (output.empty()) throw InvalidExpression("An empty sub-expression");

                while (!output.empty()) {
                    auto const either = output.front();
                    output.pop();
                    /*
                    std::cout << '(' << (either.index()) << ')'
                              << (either.index() == 0 ? std::get<0>(either)->result(Variables<T>({}))
                                                      : std::get<1>(either))
                              << ' ';
                              */
                    if (either.index() == 0) operands.push(std::get<0>(either)); // operand
                    else {                                                       // operator
                        switch (std::get<1>(either)) {
                            case OperatorType::LEFT_BRACE: throw InvalidExpression("Imbalanced parentheses");
                            case OperatorType::RIGHT_BRACE:
                                throw std::runtime_error("Parser has produced a right brace token");
                            case OperatorType::INVERT: {
                                operands.push(std::make_shared<InvertOperation<T>>(operands.pop()));
                                break;
                            }
                            case OperatorType::PLUS: {
                                operands.push(std::make_shared<PlusOperation<T>>(operands.pop(), operands.pop()));
                                break;
                            }
                            case OperatorType::MINUS: {
                                operands.push(std::make_shared<MinusOperation<T>>(operands.pop(), operands.pop()));
                                break;
                            }
                            case OperatorType::MULTIPLY: {
                                operands.push(std::make_shared<MultiplyOperation<T>>(operands.pop(), operands.pop()));
                                break;
                            }
                            case OperatorType::DIVIDE: {
                                operands.push(std::make_shared<DivideOperation<T>>(operands.pop(), operands.pop()));
                                break;
                            }
                            case OperatorType::SQRT: {
                                operands.push(std::make_shared<PrimitiveSqrtOperation<T>>(operands.pop()));
                                break;
                            }
                            case OperatorType::EXP: {
                                operands.push(std::make_shared<PowOperation<T>>(std::make_shared<ConstEOperation<T>>(),
                                                                                operands.pop()));
                                break;
                            }
                            case OperatorType::POW: {
                                operands.push(std::make_shared<PowOperation<T>>(operands.pop(), operands.pop()));
                                break;
                            }
                            case OperatorType::SIN: {
                                operands.push(std::make_shared<SinOperation<T>>(operands.pop()));
                                break;
                            }
                            case OperatorType::COS: {
                                operands.push(std::make_shared<CosOperation<T>>(operands.pop()));
                                break;
                            }
                            case OperatorType::TG: {
                                operands.push(std::make_shared<TgOperation<T>>(operands.pop()));
                                break;
                            }
                            case OperatorType::CTG: {
                                operands.push(std::make_shared<CtgOperation<T>>(operands.pop()));
                                break;
                            }
                                // unsupported
                            case OperatorType::MODULO: {
                                case OperatorType::ASIN:
                                case OperatorType::ACOS:
                                case OperatorType::ATG:
                                case OperatorType::ACTG: throw new std::runtime_error("Unsupported operation");
                            }
                        }
                    }
                }

                return operands.pop();
            }
        };

        enum class PermittedToken : uint8_t {
            OPERAND = 1 << 0u,
            ALGEBRAIC = 1 << 1u,
            MULTIPLYING = 1 << 2u,
        };

        class Permissions final {
            uint8_t mask_;

        public:
            void clear() { mask_ = 0; }

            void permitAnything() {
                mask_ = static_cast<uint8_t>(PermittedToken::OPERAND) | static_cast<uint8_t>(PermittedToken::ALGEBRAIC)
                        | static_cast<uint8_t>(PermittedToken::MULTIPLYING);
            }

            void permit(PermittedToken token) { mask_ |= static_cast<uint8_t>(token); }

            inline bool isPermitted(PermittedToken token) { return (mask_ & static_cast<uint8_t>(token)) != 0; }

            void require(PermittedToken token) {
                if (!isPermitted(token)) { throw InvalidExpression("Invalid expression"); }
            }
        };

        static std::shared_ptr<Operation<T>> parseExpression(std::string_view expression) {
            Context context;
            Permissions permissions;
            permissions.permit(PermittedToken::OPERAND);

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
                        permissions.require(PermittedToken::OPERAND);

                        size_t numberLength = 1;
                        while (index < maxIndex) { // read numbers until non-digit symbol is found
                            if (std::isdigit(character = expression[index + 1])) {
                                ++numberLength;
                                ++index;
                            } else
                                break;
                        }
                        if (character == ',') {
                            ++numberLength;
                            ++index;
                            while (index < maxIndex) { // read numbers until non-digit symbol is found
                                if (std::isdigit(character = expression[index + 1])) {
                                    ++numberLength;
                                    ++index;
                                } else
                                    break;
                            }
                        }

                        // TODO from string
                        context.pushConstant(T(expression.substr(index + 1 - numberLength, numberLength)));
                        permissions.permitAnything();

                        break;
                    }
                    case ',': { // decimal part of the number: (\d+)
                        permissions.require(PermittedToken::OPERAND);
                        if (index == maxIndex) // no possible digits
                            throw InvalidExpression("Meaningless dot at index " + std::to_string(index));

                        size_t numberLength = 1;
                        while (index < maxIndex) { // read numbers until non-digit symbol is found
                            if (std::isdigit(character = expression[index + 1])) {
                                ++numberLength;
                                ++index;
                            } else
                                break;
                        }
                        if (numberLength == 1) // no digits
                            throw InvalidExpression("Meaningless dot at index " + std::to_string(index));

                        context.pushConstant(T(expression.substr(index + 1 - numberLength, numberLength)));

                        permissions.clear();
                        permissions.permitAnything();

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
                        permissions.require(PermittedToken::OPERAND);

                        if (index < maxIndex - 2) { // 2 extra symbols (`xp`) + at least one symbol after
                            // continue reading
                            if (((character = expression[index + 1]) == 'X' || character == 'x')
                                && ((character = expression[index + 2]) == 'P' || character == 'p')) {
                                index += 2;
                                // this is an exp function
                                context.pushFunction(OperatorType::EXP);
                                permissions.permit(PermittedToken::OPERAND);

                                break;
                            }
                        }

                        permissions.require(PermittedToken::OPERAND);
                        // this is an e constant
                        context.pushValue(std::make_shared<ConstEOperation<T>>());

                        permissions.permitAnything();

                        break;
                    }
                    case 'P':
                    case 'p': {
                        permissions.require(PermittedToken::OPERAND);
                        if (index < maxIndex) {
                            auto const char1 = expression[character + 1];
                            if (char1 == 'I' || char1 == 'i') {
                                // this is a PI const
                                context.pushValue(std::make_shared<ConstPiOperation<T>>());
                                break;
                            }
                        }
                        // this is a variable
                        context.pushVariable(character); // P or p
                        permissions.permitAnything();

                        break;
                    }
                    case 'S': // sin or sqrt
                    case 's': {
                        permissions.require(PermittedToken::OPERAND);
                        // 2 extra symbols (`in`) + at least one symbol after for `sin`
                        if (index < maxIndex - 2) {
                            auto const char1 = expression[index + 1], char2 = expression[index + 2];
                            if ((char1 == 'I' || char1 == 'i') && (char2 == 'N' || char2 == 'n')) {
                                index += 2;
                                // this is a sin function
                                context.pushFunction(OperatorType::SIN);
                                permissions.permit(PermittedToken::OPERAND);

                                break;
                            }

                            if (index < maxIndex - 3) {
                                if ((char1 == 'Q' || char1 == 'q') && (char2 == 'R' || char2 == 'r')) {
                                    auto const char3 = expression[index + 3];
                                    if (char3 == 'T' || char3 == 't') {
                                        index += 3;
                                        // this is a sqrt function
                                        context.pushFunction(OperatorType::SQRT);
                                        permissions.permit(PermittedToken::OPERAND);

                                        break;
                                    }
                                }
                            }
                        }
                        // this is a variable
                        context.pushVariable(character); // S or s
                        permissions.permitAnything();

                        break;
                    }
                    case 'C':
                    case 'c': {
                        permissions.require(PermittedToken::OPERAND);
                        // 2 extra symbols (`in`) + at least one symbol after for `cos` or `ctg`
                        if (index < maxIndex - 2) {
                            auto const char1 = expression[index + 1], char2 = expression[index + 2];
                            if ((char1 == 'O' || char1 == 'o') && (char2 == 'S' || char2 == 's')) {
                                index += 2;
                                // this is a cos function
                                context.pushFunction(OperatorType::COS);
                                permissions.permit(PermittedToken::OPERAND);

                                break;
                            }

                            if ((char1 == 'T' || char1 == 't') && (char2 == 'G' || char2 == 'g')) {
                                index += 2;
                                // this is a ctg function
                                context.pushFunction(OperatorType::CTG);
                                permissions.permit(PermittedToken::OPERAND);

                                break;
                            }
                        }
                        // this is a variable
                        context.pushVariable(character); // C or c
                        permissions.permitAnything();

                        break;
                    }
                    case 'T':
                    case 't': {
                        permissions.require(PermittedToken::OPERAND);
                        // 2 extra symbols (`in`) + at least one symbol after for `cos` or `ctg`
                        if (index < maxIndex - 2) {
                            auto const char1 = expression[index + 1], char2 = expression[index + 2];
                            if (char1 == 'G' || char1 == 'g') {
                                index += 1;
                                // this is a ctg function
                                context.pushFunction(OperatorType::TG);
                                permissions.permit(PermittedToken::OPERAND);

                                break;
                            }
                        }
                        // this is a variable
                        context.pushVariable(character); // C or c
                        permissions.permitAnything();

                        break;
                    }
                        // TODO add tg
                        // TODO arc-function
                    case '+': {
                        permissions.require(PermittedToken::ALGEBRAIC);
                        context.pushOperator({OperatorType::PLUS, 0, false});
                        permissions.permit(PermittedToken::OPERAND);

                        break;
                    }
                    case '-': {
                        if (permissions.isPermitted(PermittedToken::ALGEBRAIC)) {
                            // binary minus
                            context.pushOperator({OperatorType::MINUS, 0, false});
                            permissions.permit(PermittedToken::OPERAND);
                        } else {
                            // unary minus
                            permissions.require(PermittedToken::OPERAND);
                            context.pushOperator({OperatorType::INVERT, 0, false});
                            permissions.permit(PermittedToken::OPERAND);
                        }

                        break;
                    }
                    case '*': {
                        permissions.require(PermittedToken::MULTIPLYING);
                        context.pushOperator({OperatorType::MULTIPLY, 1, false});
                        permissions.permit(PermittedToken::OPERAND);

                        break;
                    }
                    case '/': {
                        permissions.require(PermittedToken::MULTIPLYING);
                        context.pushOperator({OperatorType::DIVIDE, 1, false});
                        permissions.permit(PermittedToken::OPERAND);

                        break;
                    }
                    case '%': {
                        permissions.require(PermittedToken::MULTIPLYING);
                        context.pushOperator({OperatorType::MODULO, 1, false});
                        permissions.permit(PermittedToken::OPERAND);

                        break;
                    }
                    case '^': {
                        permissions.require(PermittedToken::MULTIPLYING);
                        context.pushOperator({OperatorType::POW, 2, false});
                        permissions.permit(PermittedToken::OPERAND);

                        break;
                    }
                    case '(': {
                        permissions.require(PermittedToken::OPERAND);
                        context.pushLeftBracket();
                        permissions.permit(PermittedToken::OPERAND);

                        break;
                    }
                    case ')': {
                        // FIXME (?) permissions.require(PermittedToken::ALGEBRAIC);
                        context.pushRightBracket();
                        permissions.permit(PermittedToken::ALGEBRAIC);
                        permissions.permit(PermittedToken::MULTIPLYING);

                        break;
                    }
                    default: {
                        if (std::isalpha(character)) {
                            permissions.require(PermittedToken::OPERAND);
                            context.pushVariable(character);
                            permissions.permitAnything();
                        }
                    }
                }
            }

            context.flushRemaining();

            return context.createOperation();
        }

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

                if (!empty) {
                    // flush the last sub-expression
                    terms.push_back({expression.substr(startIndex, length - startIndex), negative});
                }
            }

            return terms;
        }

        // parses an expression with no possible reordering
        static OperationPointer parseSingleTermExpression(std::string_view expression) {
            return parseExpression(expression);
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
