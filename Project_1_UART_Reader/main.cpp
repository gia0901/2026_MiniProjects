#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <iostream>
#include "uart_device.h"

//=========================================
// UART Reader project
//=========================================
// Skills: 
//   - Error handling
//   - fd RAII wrapper
//   - file open/read

int main() {
    // tạo uart device (=> Result<UartDevice>)
    auto uartResult = UartDevice::create("/dev/ttyS1");
    if (!uartResult.hasValue()) {
        std::cerr << "[UART ERROR] " << uartResult.error().message << std::endl;
        return 1;
    }

    // std::move ở đây không move giá trị uartResult ngay lập tức.
    // Nó chỉ chuyển uartResult thành rvalue để gọi overload Result<T>::value() &&
    // tức là chỉ lấy giá trị khi Result là temporary.
    // Trong value() && mới thực sự move UartDevice (UniqueFd) ra khỏi Result.
    auto uartDevice = std::move(uartResult).value();

                                                    
    // đọc data từ UART (=> Result<std::vector<char>>)
    auto readResult = uartDevice.readData();
    
    // fail: in lỗi
    if (!readResult.hasValue()) {
        std::cerr << "[UART ERROR] " << readResult.error().message << std::endl;
        return 1;
    }

    // success: in ra data đọc được
    std::cout << "Data: ";
    for (char c : readResult.value()) {
        std::cout << c << ' ';
    }
    std::cout << std::endl;
    std::cout << "Received bytes: " << readResult.value().size() << std::endl;
}
