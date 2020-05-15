#ifndef INCLUDE_OPERATION_H_
#define INCLUDE_OPERATION_H_

#include <operation/variables.h>
#include <stdexcept>
#include <system_error>

namespace calculator {

    class OperationError final : public std::invalid_argument {
    public:
        OperationError(const std::string& arg) : invalid_argument(arg) {}

        OperationError(const char* string) : invalid_argument(string) {}

        OperationError(const OperationError& argument) : invalid_argument(argument) {}

        OperationError(OperationError&& argument) : invalid_argument(argument) {}
    };

    /**
     * @brief An operation providing some result.
     *
     * @tparam T result of the operation
     */
    template<typename T>
    class Operation {
    public:
        virtual ~Operation() noexcept = default;

        /**
         * @brief Gets the result of this operation.
         *
         * @return result of this operation
         */
        virtual T result(Variables<T> const& variables) const = 0;
    };

    template<typename T>
    class UnaryOperation : public Operation<T> {
        std::shared_ptr<Operation<T>> const operand_;

    public:
        UnaryOperation(std::shared_ptr<Operation<T>> const operand) noexcept : operand_(operand) {}

        T result(Variables<T> const& variables) const final override { return apply(operand_->result(variables)); };

    protected:
        virtual T apply(T&& value) const = 0;
    };

    template<typename T>
    class BinaryOperation : public Operation<T> {
        std::shared_ptr<Operation<T>> const leftOperand_, rightOperand_;

    public:
        BinaryOperation(std::shared_ptr<Operation<T>> const leftOperand,
                        std::shared_ptr<Operation<T>> const rightOperand) noexcept
            : leftOperand_(leftOperand), rightOperand_(rightOperand) {}

        T result(Variables<T> const& variables) const final override {
            return apply(leftOperand_->result(variables), rightOperand_->result(variables));
        };

    protected:
        virtual T apply(T&& leftValue, T&& rightValue) const = 0;
    };
} // namespace calculator

#endif //INCLUDE_OPERATION_H_
