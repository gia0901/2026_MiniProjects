#pragma once
#include <vector>
#include "result.h"
#include "unique_fd.h"


// ==========================
// UART Device
// ==========================
//
// Middleware-style abstraction.
// Business logic không gọi open/read trực tiếp.
//
class UartDevice {
public:
    // factory function
    // return:  Result<T> có constructor để 'bắt' Error, hoặc kiểu dữ liệu T bất kỳ
    static Result<UartDevice> create(const std::string& devicePath) {
        // invalid check
        if (devicePath.empty()) {
            return Error{ErrorCode::InvalidArgument, "device path is empty"};
        }

        // runtime/system failure
        int fd = ::open(devicePath.c_str(), O_RDONLY);
        if (fd < 0) {
            return makeSystemError(ErrorCode::OpenFailed, "open");
        }

        return UartDevice(UniqueFd(fd)); // tạo unique fd và chuyển giao cho local fd_
    }

    // read data từ UART
    Result<std::vector<char>> readData() {
        constexpr size_t BUFFER_SIZE = 64;
        std::vector<char> buffer(BUFFER_SIZE);

        // POSIX read (partial read có thể xảy ra, cần kiểm tra sau)
        ssize_t bytesRead = ::read(fd_.get(), buffer.data(), buffer.size());
        if (bytesRead < 0) {
            return makeSystemError(ErrorCode::ReadFailed, "read");
        }

        // Resize lại vừa đủ kích thước đọc được
        buffer.resize(bytesRead);
        return buffer;  // Result<T> có constructor để 'bắt' kiểu T, ở đây là vector<char>
    }

private:
    // private constructor
    // chuyển giao UniqueFd cho fd_ của object
    explicit UartDevice(UniqueFd fd) : fd_(std::move(fd)) {}
private:
    UniqueFd fd_;
};