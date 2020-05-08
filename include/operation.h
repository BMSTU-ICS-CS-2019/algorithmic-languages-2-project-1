#ifndef INCLUDE_OPERATION_H_
#define INCLUDE_OPERATION_H_

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/outcome.hpp>
#include <boost/outcome/result.hpp>

namespace calculator {

    namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;

    class OperationError final {
        std::string cause_;

    public:
        OperationError(std::string cause) : cause_(cause) {}

        // required for result
        OperationError() : OperationError("") {}

        std::string cause() { return cause_; }
    };

    /**
     * @brief An operation providing some result.
     *
     * @tparam T result og the operation
     */
    template<typename T>
    class Operation {
    public:
        typedef outcome::result<T> Result;

        virtual ~Operation() noexcept = default;
        //typedef BOOST_OUTCOME_V2_NAMESPACE::result<T> Result;
        /**
         * @brief Gets the result of this operation.
         *
         * @return result of this operation
         */
        virtual Result result() const noexcept = 0;
    };

    template<typename T>
    class UnaryOperation : public Operation<T> {
        std::shared_ptr<Operation<T>> const operand_;

    public:
        using typename Operation<T>::Result;

        UnaryOperation(std::shared_ptr<Operation<T>> const operand) noexcept : operand_(operand) {}

        Result result() const noexcept final override {
            const auto operand = operand_->result();
            BOOST_OUTCOME_TRY(operand);

            return apply(operand.value());
        };

    protected:
        virtual Result apply(T const value) const noexcept = 0;
    };

    template<typename T>
    class BinaryOperation : public Operation<T> {
        std::shared_ptr<Operation<T>> const leftOperand_, rightOperand_;

    public:
        using typename Operation<T>::Result;

        BinaryOperation(std::shared_ptr<Operation<T>> const leftOperand,
                        std::shared_ptr<Operation<T>> const rightOperand) noexcept
            : leftOperand_(leftOperand), rightOperand_(rightOperand) {}

        Result result() const noexcept final override {
            const auto left = leftOperand_->result();
            BOOST_OUTCOME_TRY(left);
            const auto right = rightOperand_->result();
            BOOST_OUTCOME_TRY(right);

            return apply(left.value(), right.value());
        };

    protected:
        virtual Result apply(T const leftValue, T const rightValue) const noexcept = 0;
    };
} // namespace calculator

#endif //INCLUDE_OPERATION_H_
