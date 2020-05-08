#ifndef INCLUDE_OPERATION_H_
#define INCLUDE_OPERATION_H_

#include <boost/outcome.hpp>
#include <boost/outcome/result.hpp>
#include <boost/multiprecision/cpp_int.hpp>

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
} // namespace calculator

#endif //INCLUDE_OPERATION_H_
