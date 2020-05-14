#ifndef INCLUDE_SIMPLE_EXPRESSION_PARSER_H_
#define INCLUDE_SIMPLE_EXPRESSION_PARSER_H_

#include <map>
#include <operation/algebraic_operations.h>
#include <parser/expression_parser.h>
#include <queue>
#include <vector>
#include <string_view>
#include <iostream> // TODO remove

namespace calculator {

    template<typename T>
    class SimpleOperationParser final : public ExpressionParser<T> {
        typedef std::shared_ptr<Operation<T>> OperationPointer;
        class Operator {
        public:
            virtual OperationPointer createOperation(std::queue<T>& operandQueue) = 0;
        };

        struct Term {
            std::string_view const expression;
            bool const negative;
        };

        /*
         * 1.) split by pluses and minuses treating minuses as pluses of inverted values
         * 2.) parse each term separately (performing inversion if needed) using Shunting-yard algorithm
         * 3.) Sum terms
         */


        // true for not EOF
        // false for EOF
        static std::vector<Term> parseTerms(std::string_view expression) {
            if (expression.empty()) return std::vector<Term>();

            std::vector<Term> terms;

            {
                size_t depth = 0, startIndex = 0;
                auto negative = false, empty = true;
                auto const length = expression.length();
                for (size_t index = 0; index < length; ++index) {
                    auto const token = expression[index];
                    if (depth == 0) {
                        if (token == ')') throw InvalidExpression(
                                "Imbalanced brackets: a closing bracket was found at index " + std::to_string(index)
                        );

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
                                ++startIndex; // omit this symbol
                            }
                            continue;
                        }

                        // regular token => not empty
                        empty = false;
                    } else {
                        // in case of increasing depth, simply not forget about checking depth
                        if (token == ')' && --depth == 0) {
                            // flush sub-expression (omitting brackets)
                            terms.push_back({expression.substr(startIndex + 1, index - startIndex), negative});

                            empty = true;
                            startIndex = index + 1;
                            negative = false;
                        }
                        else if (token == '(') ++depth;
                        continue;
                    }
                }

                if (empty) throw InvalidExpression(
                        "Missing operand: no right operand provided for " + std::to_string(expression[length - 1])
                );

                // flush the last sub-expression
                terms.push_back({expression.substr(startIndex, length - startIndex), negative});
            }

            return terms;
        }

    public:
        /*
         * The idea of the parser is the following:
         * - at first, tokenize the stream
         * - read tokens one by one
         * - store operations in the order in which they appear in a stack
         * - once reaching parentheses:
         *   - push stack entry for new level
         *   - push normal operation for parameters
         * - once the stream is over and there are no exceptions, build an operation tree respecting ordering
         */
        virtual OperationPointer parse(std::istream& input) override {
            // FIXME
            std::string expr;
            getline(input, expr);
            auto const terms = parseTerms(expr);
            for (auto const& term : terms) {
                std::cout << "SubExpr: {" << term.expression << "} ("<< (term.negative ? '-' : '+') << ')' << std::endl;
            }
            // note: while usage of goto is considered bad practice,
            // it is the most effective and readable way of ending tokenization

            /*
            int token;
            while ((token = input.get()) != EOF) {
                if ('0' <= token && token <= '9') { // numeric token
                    if (skipEmptyCharacters(input)) goto tokenizationEnd;
                }
                std::cout << "Token = " << token << std::endl;
            }

            // here the process of tokenization is considered over
             */
            return std::make_shared<ConstOperation<T>>(0); // FIXME
        }
    };
} // namespace calculator


#endif //INCLUDE_SIMPLE_EXPRESSION_PARSER_H_
