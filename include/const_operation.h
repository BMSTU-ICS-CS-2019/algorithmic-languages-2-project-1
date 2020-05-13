#ifndef INCLUDE_CONST_OPERATION_H_
#define INCLUDE_CONST_OPERATION_H_

#include <operation.h>
#include <utility>

namespace calculator {

    template<typename T>
    class ConstOperation final : public Operation<T> {
        T const result_;

    public:
        explicit ConstOperation(T const& result) : result_(result) {}

        explicit ConstOperation(T&& result) noexcept : result_(std::move(result)) {}

        T result() const override { return result_; }
    };
} // namespace calculator


#endif //INCLUDE_CONST_OPERATION_H_
