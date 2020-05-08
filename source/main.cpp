#include <iostream>

#include <operation.h>
#include <const_operation.h>

using BigInteger = boost::multiprecision::cpp_rational;

int main() {
    calculator::ConstOperation<BigInteger> op(124);
    std::cout << typeid(op.result().value()).name() << std::endl;

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
