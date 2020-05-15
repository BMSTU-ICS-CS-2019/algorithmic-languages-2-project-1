#include <iostream>

#include <boost/multiprecision/cpp_int.hpp>
#include <parser/simple_expression_parser.h>

using BigDecimal = boost::multiprecision::cpp_rational;

static const size_t MAX_VARIABLES = 10;

Variables<BigDecimal> readVariables();

int main() { // x^2 + cos 3.1415926536
    calculator::SimpleOperationParser<BigDecimal> parser;

    std::string input;
    do {
        auto const variables = readVariables();
        try {
            std::cout << "Enter the expression to calculate" << std::endl;
            std::cin.ignore();
            auto const parsed = parser.parse(std::cin);
            try {
                auto const result = parsed->result(variables);
                std::cout << "\t=\t" << result << std::endl;
            } catch (calculator::OperationError const& e) {
                std::cerr << "Operation error: " << e.what() << std::endl;
            }
        } catch (calculator::InvalidExpression const& e) {
            std::cerr << "Invalid expression: " << e.what() << std::endl;
        }
        std::cout << "Enter `end`, `exit` or `stop` to end this session" << std::endl;
        getline(std::cin, input);
    } while (input != "end" && input != "exit" && input != "stop");
    return 0;
}

Variables<BigDecimal> readVariables() {
    size_t count;
    do {
        std::cout << "Enter the number of variables you would like to use (max: " << MAX_VARIABLES << ')' << std::endl;
        std::cin >> count;
    } while (count > MAX_VARIABLES);

    std::map<char, BigDecimal> variables;
    for (size_t i = 0; i < count; ++i) {
        char name;
        BigDecimal value;
        do {
            std::cout << "Enter the name of the variable and its value" << std::endl;
            std::cin >> name >> value;
        } while (variables.contains(name));
        variables.insert(std::pair<char, BigDecimal>(name, value));
    }

    return Variables(variables);
}
