#ifndef INCLUDE_ALGEBRAIC_OPERATIONS_H_
#define INCLUDE_ALGEBRAIC_OPERATIONS_H_

#include <cmath>
#include <memory>
#include <operation/operation.h>
#include <type_traits>
#include <vector>

namespace calculator {

    template<typename T>
    class VariableOperation final : public Operation<T> {
        char name_;

    public:
        VariableOperation(char const name) noexcept : name_(name) {}

        T result(Variables<T> const& variables) const final override {
            auto const variable = variables.get(name_);
            if (variable) return variable.value();
            throw new OperationError("Unknown variable: " + std::to_string(name_));
        };
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

    template<typename T>
    class DivideOperation final : public BinaryOperation<T> {
        T const ZERO{};

    public:
        DivideOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        T apply(T&& leftValue, T&& rightValue) const override {
            if (rightValue == ZERO) throw OperationError("Division by zero");

            return leftValue / rightValue;
        }
    };

    template<typename T>
    class InvertOperation final : public UnaryOperation<T> {
    public:
        InvertOperation(std::shared_ptr<Operation<T>> operand) noexcept : UnaryOperation<T>(operand) {}

        T apply(T&& value) const override { return -value; }
    };

    template<typename T>
    class PowOperation final : public BinaryOperation<T> {
    public:
        PowOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        T apply(T&& leftValue, T&& rightValue) const override {
            return static_cast<T>(pow(static_cast<double>(leftValue), static_cast<double>(rightValue)));
        }
    };

    template<typename T>
    class PrimitiveSqrtOperation final : public UnaryOperation<T> {
    public:
        PrimitiveSqrtOperation(std::shared_ptr<Operation<T>> operand) noexcept : UnaryOperation<T>(operand) {}

        T apply(T&& value) const override { return static_cast<T>(sqrt(static_cast<double>(value))); }
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

    /*
     * Trigonometry TODO more strict
     */
    template<typename T>
    class SinOperation final : public UnaryOperation<T> {
    public:
        SinOperation(std::shared_ptr<Operation<T>> operand) noexcept : UnaryOperation<T>(operand) {}

        T apply(T&& value) const override { return static_cast<T>(sin(static_cast<double>(value))); }
    };

    template<typename T>
    class CosOperation final : public UnaryOperation<T> {
    public:
        CosOperation(std::shared_ptr<Operation<T>> operand) noexcept : UnaryOperation<T>(operand) {}

        T apply(T&& value) const override { return static_cast<T>(cos(static_cast<double>(value))); }
    };

    template<typename T>
    class TgOperation final : public UnaryOperation<T> {
    public:
        TgOperation(std::shared_ptr<Operation<T>> operand) noexcept : UnaryOperation<T>(operand) {}

        T apply(T&& value) const override { return static_cast<T>(tan(static_cast<double>(value))); }
    };

    template<typename T>
    class CtgOperation final : public UnaryOperation<T> {
    public:
        CtgOperation(std::shared_ptr<Operation<T>> operand) noexcept : UnaryOperation<T>(operand) {}

        T apply(T&& value) const override { return static_cast<T>(1. / tan(static_cast<double>(value))); }
    };
} // namespace calculator

#endif //INCLUDE_ALGEBRAIC_OPERATIONS_H_
