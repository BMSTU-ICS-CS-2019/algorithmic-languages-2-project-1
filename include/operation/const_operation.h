#ifndef INCLUDE_CONST_OPERATION_H_
#define INCLUDE_CONST_OPERATION_H_

#include <cmath>
#include <operation/operation.h>
#include <utility>

namespace calculator {

    template<typename T>
    class ConstOperation final : public Operation<T> {
        T const result_;

    public:
        explicit ConstOperation(T const& result) : result_(result) {}

        explicit ConstOperation(T&& result) noexcept : result_(std::move(result)) {}

        T result(Variables<T> const& variables) const override { return result_; }
    };

    template<typename T>
    class ConstEOperation final : public Operation<T> {
        //static T const E_ = M_E;

    public:

        explicit ConstEOperation() {}

        T result(Variables<T> const& variables)const override { return M_E; }
    };

    template<typename T>
    class ConstPiOperation final : public Operation<T> {
        //static T const PI_ = M_PI;

    public:

        explicit ConstPiOperation() {}

        T result(Variables<T> const& variables)const override { return M_PI; }
    };
} // namespace calculator


#endif //INCLUDE_CONST_OPERATION_H_
