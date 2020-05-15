#ifndef INCLUDE_ALGEBRAIC_OPERATIONS_H_
#define INCLUDE_ALGEBRAIC_OPERATIONS_H_

#include <cmath>
#include <memory>
#include <operation/operation.h>
#include <type_traits>
#include <vector>

namespace calculator {

    template<typename T>
    class VariableOperation : public Operation<T> {
        char name_;

    public:
        VariableOperation(char const name) noexcept : name_(name) {}

        T result(Variables<T> const& variables) const final override {
            auto const variable = variables.get(name_);
            if (variable) return variable.value();
            throw new OperationError("Unknown variable: " + std::to_string(variable));
        };

    protected:
        virtual T apply(T&& value) const = 0;
    };

    template<typename T>
    class PlusOperation final : public BinaryOperation<T> {
    public:
        PlusOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        T apply(T&& leftValue, T&& rightValue) const override { return leftValue + rightValue; }
    };

    template<typename T>
    class NegativeOperation final : public UnaryOperation<T> {
    public:
        NegativeOperation(std::shared_ptr<Operation<T>> operand) noexcept : UnaryOperation<T>(operand) {}

        T apply(T&& value) const override { return -value; }
    };

    template<typename T>
    class MinusOperation final : public BinaryOperation<T> {
    public:
        MinusOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        T apply(T&& leftValue, T&& rightValue) const override { return leftValue - rightValue; }
    };

    template<typename T>
    class MultiplyOperation final : public BinaryOperation<T> {
    public:
        MultiplyOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        T apply(T&& leftValue, T&& rightValue) const override { return leftValue * rightValue; }
    };

    template<typename T, T ZERO>
    class DivideOperation final : public BinaryOperation<T> {
    public:
        DivideOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        T apply(T&& leftValue, T&& rightValue) const override {
            if (rightValue == ZERO) throw OperationError("Division by zero");

            return leftValue / rightValue;
        }
    };

    template<typename T, T ZERO>
    class ModuloOperation final : public BinaryOperation<T> {
    public:
        ModuloOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        T apply(T&& leftValue, T&& rightValue) const override {
            if (rightValue == ZERO) throw OperationError("Division by zero");

            return leftValue % rightValue;
        }
    };

    template<typename T>
    class PrimitiveSqrtOperation final : public UnaryOperation<T> {
        static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value);

    public:
        PrimitiveSqrtOperation(std::shared_ptr<Operation<T>> operand) noexcept : UnaryOperation<T>(operand) {}

        T apply(T&& value) const override { return sqrt(value); }
    };

    template<typename T>
    class ObjectSqrtOperation final : public UnaryOperation<T> {
        static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value);

    public:
        ObjectSqrtOperation(std::shared_ptr<Operation<T>> operand) noexcept : UnaryOperation<T>(operand) {}

        T apply(T&& value) const override { return value.sqrt(); }
    };

    template<typename T>
    class VectorSumOperation final : public Operation<T> {
        std::vector<std::shared_ptr<Operation<T>>> operands_;

    public:
        VectorSumOperation(std::vector<std::shared_ptr<Operation<T>>> operands) noexcept : operands_(operands) {}

        T result(Variables<T> const& variables) const final override {
            T sum{};
            for (auto const element : operands_) sum += element->result(variables);

            return sum;
        };
    };
} // namespace calculator

#endif //INCLUDE_ALGEBRAIC_OPERATIONS_H_
