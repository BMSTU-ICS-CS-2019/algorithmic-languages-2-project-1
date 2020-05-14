#include <iostream>

#include <boost/multiprecision/cpp_int.hpp>
#include <operation/algebraic_operations.h>
#include <operation/const_operation.h>
#include <parser/simple_expression_parser.h>

using BigDecimal = boost::multiprecision::cpp_rational;
// TODO remove

int main() {
    calculator::PrimitiveSqrtOperation<int> expr(std::make_shared<calculator::PlusOperation<int>>(
            std::make_shared<calculator::DivideOperation<int, 0>>(
                    std::make_shared<calculator::ConstOperation<int>>(4),
                    std::make_shared<calculator::ConstOperation<int>>(2)
            ),
            std::make_shared<calculator::ConstOperation<int>>(7)
    ));
    std::cout << "OperationResult = " << expr.result() << std::endl;
    BigDecimal a;
    calculator::SimpleOperationParser<BigDecimal> parser;

    auto in = std::stringstream("1+2+3434+34*4*22-32434-(12*3+4-5)-2*3+4");
    //std::string expression;
    //getline(std::cin, expression);
    parser.parse(in);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
