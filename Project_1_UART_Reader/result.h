#pragma once
#include <cerrno>
#include <cstring>
#include <string>
#include <string_view>
#include <optional>

enum class ErrorCode {
    OpenFailed,
    ReadFailed,
    InvalidArgument
};

struct Error {
    ErrorCode code;         // mã code cho máy hiểu (machine-readable)
    std::string message;    // string cho người hiểu
};

// Tạo instance chuẩn cho lỗi (struct Error)
Error makeSystemError(ErrorCode code, std::string_view operation) {
    return {
        code,
        std::string(operation) +
        " failed: " +
        std::strerror(errno)
    };
}

// Result<T> pattern: quản lý kết quả trả về
template<typename T>
class Result {
public:
    // success constructor
    Result(T value) : value_(std::move(value)) {}

    // failure constructor: khi đó value_ trống => check hasValue() là biết kết quả fail hay success
    Result(Error error) : error_(std::move(error)) {}

    bool hasValue() const {
        return value_.has_value();
    }

    // read-only access
    const T& value() const & {
        return value_.value();
    }

    // mutable lvalue
    T& value() & {
        return value_.value();
    }

    // move access
    // && thứ hai: ref-qualifier
    //  “Chỉ được move value ra ngoài nếu chính container cũng là temporary/rvalue.”
    T&& value() && {
        return std::move(value_.value()); // std::move cast thành '&&' => cần return T&&
    }

    const Error& error() const {
        return error_.value();
    }

private:
    std::optional<T> value_;        // chứa giá trị trả về hoặc std::nullopt
    std::optional<Error> error_;    // chứa lỗi trả về hoặc std::nullopt
};