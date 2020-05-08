#ifndef INCLUDE_ALGEBRAIC_OPERATIONS_H_
#define INCLUDE_ALGEBRAIC_OPERATIONS_H_

#include <operation.h>

namespace calculator {

    template<typename T>
    class PlusOperation : public Operation<T> {
        std::shared_ptr<Operation<T>> const left_, right_;

    public:
        using typename Operation<T>::Result;

        PlusOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : left_(left), right_(right) {}

        Result result() const noexcept override {
            const auto left = left_->result();
            BOOST_OUTCOME_TRY(left);

            const auto right = right_->result();
            BOOST_OUTCOME_TRY(right);

            return outcome::success(left.value() + right.value());
        };
    };
} // namespace calculator

#endif //INCLUDE_ALGEBRAIC_OPERATIONS_H_
