#include <iostream>

#include <algebraic_operations.h>
#include <const_operation.h>
#include <operation.h>

using BigDecimal = boost::multiprecision::cpp_rational;

int main() {
    calculator::PlusOperation<int> sum(
            std::make_shared<calculator::ConstOperation<int>>(1),
            std::make_shared<calculator::ConstOperation<int>>(2)
    );
    std::cout << "Result = " << sum.result().value() << std::endl;

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
