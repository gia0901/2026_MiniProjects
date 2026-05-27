#include <iostream>
#include <cerrno>
#include <optional>
#include <cstring>
#include <string>
#include <string_view>
using namespace std;

enum class ErrorCode {
    OpenFailed,
    ReadFailed,
    WriteFailed,
    InvalidArguments,
};

struct Error {
    ErrorCode code;      // machine-readable, dùng cho logic
    std::string message; // human-readable, dùng để logging
    // ko cần implement 'move', để compiler tự lo
};

//==================================
//  Kiểu dữ liệu Result
//==================================
// - transfer ownership (move) và chứa data (nếu ko có lỗi)
// - chứa error (nếu có lỗi xảy ra)
template<typename T>
class Result {
public:
    // success constructor
    Result(T data) : data_(std::move(data)) {}

    // failure constructor
    Result(Error error) : error_(std::move(error)) {}

    // nếu chứa data
    bool hasData() const { return data_.has_value(); }

    // access data (compiler lựa chọn 1 trong 3 hàm dưới)
    // 1. read-only
    const T& getData() const & {
        return data_.value();
    }

    // 2. move access, transfer data ra ngoài : kiểu data T cần support move
    T&& getData() && {
        return std::move(data_.value());
    }

    // 3. mutable access: có thể thay đổi data
    T& getData() & {
        return data_.value();
    }
private:
    std::optional<T> data_;
    std::optional<Error> error_;
};

//====================================
//  Helper functions
//====================================
// Tạo 'Error' chuẩn cho lỗi
Error makeSystemError(ErrorCode code, std::string_view whatHaveDone) {
    return Error {
        .code = code,
        .message = std::string(whatHaveDone) + " failed: " + std::strerror(errno)
    };
}