#include <iostream>

#include <boost/multiprecision/cpp_int.hpp>
#include <operation/algebraic_operations.h>
#include <operation/const_operation.h>

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

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
