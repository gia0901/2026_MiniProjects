#include <iostream>
#include <cerrno>
#include <optional>
#include <cstring>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <cassert>

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
    explicit Result(T data) : data_(std::move(data)) {}

    // failure constructor
    explicit Result(Error error) : error_(std::move(error)) {}

    // nếu chứa data
    bool hasData() const { return data_.has_value(); }

    // nếu chứa lỗi
    bool hasError() const { return error_.has_value(); }

    // truy xuất lỗi
    const Error& getError() const & { return error_.value(); }
    Error&& getError() && { return std::move(error_.value()); }

    // access data (compiler lựa chọn 1 trong 3 hàm dưới)
    // 1. read-only
    const T& getData() const & {
        assert(hasData());
        return data_.value();
    }

    // 2. move access, transfer data ra ngoài : kiểu data T cần support move
    // KHÔNG trả T&& từ getter trừ khi bạn thực sự trả tham chiếu đến một object có lifetime chắc chắn dài hơn caller (hiếm). 
    // Thay vào đó trả T (by value, moved) khi bạn muốn "di chuyển" dữ liệu ra khỏi Result.

    // Ngắn gọn: trả T (by value) sẽ move dữ liệu nội bộ vào một prvalue trả về, 
    // rồi prvalue đó được move (hoặc elided) vào biến của caller — ownership được chuyển an toàn, không tạo dangling reference.
    T getData() && {
        assert(hasData());
        return std::move(data_.value());
    }

    // 3. mutable access: có thể thay đổi data
    T& getData() & {
        assert(hasData());
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

int main() {
    
}