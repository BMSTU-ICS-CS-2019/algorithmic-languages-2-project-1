#include <iostream>

#include <algebraic_operations.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <const_operation.h>

using BigDecimal = boost::multiprecision::cpp_rational;
// TODO remove

int main() {
    calculator::PlusOperation<int> sum(
            std::make_shared<calculator::DivideOperation<int, 0>>(
                    std::make_shared<calculator::ConstOperation<int>>(4),
                    std::make_shared<calculator::ConstOperation<int>>(2)
            ),
            std::make_shared<calculator::ConstOperation<int>>(7)
    );
    std::cout << "OperationResult = " << sum.result() << std::endl;

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
