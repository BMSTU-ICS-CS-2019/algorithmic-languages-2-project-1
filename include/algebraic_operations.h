#ifndef INCLUDE_ALGEBRAIC_OPERATIONS_H_
#define INCLUDE_ALGEBRAIC_OPERATIONS_H_

#include <cmath>
#include <memory>
#include <type_traits>
#include <operation.h>

namespace calculator {

    template<typename T>
    class PlusOperation final : public BinaryOperation<T> {
    public:
        PlusOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        T apply(T&& leftValue, T&& rightValue) const noexcept override {
            return leftValue + rightValue;
        }
    };

    template<typename T>
    class MinusOperation final : public BinaryOperation<T> {
    public:
        MinusOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        T apply(T&& leftValue, T&& rightValue) const noexcept override {
            return leftValue - rightValue;
        }
    };

    template<typename T>
    class MultiplyOperation final : public BinaryOperation<T> {
    public:
        MultiplyOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        T apply(T&& leftValue, T&& rightValue) const noexcept override {
            return leftValue * rightValue;
        }
    };

    template<typename T, T ZERO>
    class DivideOperation final : public BinaryOperation<T> {
    public:
        DivideOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        T apply(T&& leftValue, T&& rightValue) const noexcept override {
            if (rightValue == ZERO) throw OperationError("Division by zero");

            return leftValue / rightValue;
        }
    };

    template<typename T, T ZERO>
    class ModuloOperation final : public BinaryOperation<T> {
    public:
        ModuloOperation(std::shared_ptr<Operation<T>> left, std::shared_ptr<Operation<T>> right) noexcept
            : BinaryOperation<T>(left, right) {}

        T apply(T&& leftValue, T&& rightValue) const noexcept override {
            if (rightValue == ZERO) throw OperationError("Division by zero");

            return leftValue % rightValue;
        }
    };

    template<typename T>
    class PrimitiveSqrtOperation final : public UnaryOperation<T> {
        static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value);
    public:
        PrimitiveSqrtOperation(std::shared_ptr<Operation<T>> operand) noexcept
            : UnaryOperation<T>(operand) {}

        T apply(T&& value) const noexcept override {
            return sqrt(value);
        }
    };
} // namespace calculator

#endif //INCLUDE_ALGEBRAIC_OPERATIONS_H_
