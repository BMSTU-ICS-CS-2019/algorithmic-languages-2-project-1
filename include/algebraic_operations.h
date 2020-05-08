#ifndef INCLUDE_ALGEBRAIC_OPERATIONS_H_
#define INCLUDE_ALGEBRAIC_OPERATIONS_H_

#include <operation.h>

namespace calculator {

    template<typename T>
    class PlusOperation final : public BinaryOperation<T> {

    public:
        using typename Operation<T>::Result;

        PlusOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        Result apply(T const leftValue, T const rightValue) const noexcept override {
            return outcome::success(leftValue + rightValue);
        }
    };
} // namespace calculator

#endif //INCLUDE_ALGEBRAIC_OPERATIONS_H_
