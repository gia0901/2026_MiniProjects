# Thiết kế roadmap interview-oriented production-style cho vị trí Embedded Linux Engineer

## Về background công việc:
- Level công việc: kỳ vọng middle-level trở lên
- Embedded Linux user-space: dynamic library, middleware, tương tác device driver
- Linux system programming
- IPC
- Performance và Concurrency
- Hardware communication.

## Ưu tiên các project gần với Embedded Linux middleware/application thực tế, ví dụ:
- sensor monitoring service
- UART communication daemon
- device event dispatcher
- plugin-based middleware
- framebuffer/display control utility
- IPC-based service
- logging daemon
- watchdog service
- telemetry collector
- asynchronous device manager

## Những điều cần tránh
- Không tạo project thiên về web/backend/server thông thường.
- Không tạo project toy/demo thiếu tính thực tế.
- Không over-engineering bằng design pattern không cần thiết.
- Không tạo architecture enterprise phức tạp.
- Không tập trung GUI-heavy application.
- Không biến roadmap thành tài liệu lý thuyết dài dòng.

## Yêu cầu chung
- Kế hoạch phải được viết vào 1 file markdown duy nhất tên là PLAN.md và có thể tải về được.
- Kế hoạch đã đầy đủ và toàn diện, không cần phải trao đổi cuộc hội thoại này nhiều lần.
- Đối với Linux system programming. chỉ tập trung vào các phần thường gặp trong Embedded Linux user-space.
- Không tập trung vào các phần Linux system programming không chuyên cho Embedded engineer.
- Kế hoạch không nên thuần lý thuyết, thay vào đó hãy bắt đầu bằng tình huống qua các project nhỏ thực tế (ngắn gọn, có thể làm nhanh chóng trong vài giờ), khai thác triệt để Modern C++ và các kiến thức của Linux system programming.
- Kế hoạch kéo dài trong 2 tuần, số lượng project bạn hãy thiết kế cho phù hợp.

## Yêu cầu về coding style:
- Tập trung Modern C++17/20 cho production-level Linux application (hướng Embedded Linux, Linux system application)
- Không over-engineering, không phức tạp hóa vấn đề.
- Không làm demo code kiểu tutorial
- Ưu tiên clean architecture, maintainability
- logging/error handling vừa đủ, tránh gây phức tạp không cần thiết

## Yêu cầu cho Project:
Format cho mỗi project, phải có cấu trúc thống nhất:
    1. Problem Scenario
    2. Interview Focus
    3. Key Modern C++ Concepts
    4. Key Linux Concepts
    5. Architecture Overview
    6. Project Structure
    7. Core Components
    8. Build Instructions
    9. Suggested Implementation Steps
    10. Testing Strategy
    11. Debugging Strategy
    12. Common Mistakes
    13. Interview Questions
    14. Optional Extensions
    15. Daily Review Checklist

- Project nên trình bày trực quan với sơ đồ, lược đồ, hình ảnh hóa nhất có thể, tránh lý thuyết quá dài dòng.
- Project chỉ nên ngắn gọn và có thể thực hiện trong một vài giờ, vì mục tiêu là ôn tập và nắm bắt nhanh chóng các nội dung để chuẩn bị cho interview.
- Khai thác Hardware communication nếu có thể.
- Với test case:
    - liệt kê các test case quan trọng
    - boundary cases
    - race condition scenarios
    - failure injection cases
    - integration behavior cần verify

- Mỗi project phải liên kết với:
    - các chủ đề interview thường gặp
    - các câu hỏi follow-up ở level Middle/Senior
    - các trade-off engineering thực tế
    - lỗi thiết kế phổ biến

- Các project phải có progression logic:
    - từ synchronous → asynchronous
    - single-thread → multi-thread
    - blocking IO → event-driven
    - simple architecture → modular architecture

- Cuối mỗi ngày hoặc mỗi project cần:
    - checklist kiến thức
    - self-review
    - câu hỏi interview nhanh
    - technical summary ngắn



## Yêu cầu về Architecture
- Ưu tiên thiết kế module boundaries rõ ràng:
    - application layer
    - middleware/service layer
    - hardware abstraction layer (HAL) nếu phù hợp
    - Linux abstraction wrapper nếu cần

- Hạn chế để business logic phụ thuộc trực tiếp vào POSIX APIs.
- Ưu tiên RAII wrapper cho resource Linux:
    - file descriptor
    - thread
    - mutex
    - socket
    - mmap
    - epoll fd
- Giải thích ownership và lifetime management trong các thành phần quan trọng.
- Chỉ abstraction khi thực sự cần thiết, tránh over-abstraction.

## API Design
- Các interface public cần:
    - rõ ownership
    - rõ thread-safety
    - tránh raw pointer nếu không cần
    - hạn chế hidden lifetime issue
    - có naming convention nhất quán

## Concurrency và Event-driven Design
- Với các project multi-thread:
    - giải thích concurrency model được chọn
    - thread ownership
    - synchronization strategy
    - queue/message passing strategy
    - tránh race-condition và deadlock như thế nào

- Nếu sử dụng event-driven architecture:
    - giải thích lý do chọn epoll/select/poll
    - mô tả event loop flow

## Performance Mindset
- Với mỗi project:
    - phân tích bottleneck tiềm năng
    - syscall overhead
    - unnecessary copy
    - lock contention
    - memory allocation frequency
    - blocking behavior
- Chỉ ra các hướng optimize thực tế nếu scale lớn hơn.

## Build System và Packaging
- Sử dụng CMake cho tất cả project.
- Khuyến khích:
    - modular CMake structure
    - shared/static library separation
    - reusable modules
    - interface include management
- Nếu phù hợp:
    - plugin-style architecture
    - dynamic loading với dlopen/dlsym
    - symbol visibility cơ bản

## Environment
- Đối với các project không cần harware: chạy trực tiếp trên máy ảo Linux.
- Đối với các project cần board: sử dụng Beaglebone Black.

## Tooling
Kết hợp tooling thực tế:
    - gdb
    - valgrind
    - sanitizers
    - strace/ltrace
    - perf cơ bản
    - clang-tidy
    - clang-format

## Debugging và Troubleshooting
- Với mỗi project:
    - đề xuất các bug thực tế có thể xảy ra
    - cách debug bằng:
        - gdb
        - strace
        - valgrind
        - sanitizers
        - perf
    - mô tả symptom → root cause mapping

## Interview focus:
Mỗi project cần có:
- interview discussion section:
    - design trade-offs
    - alternative approaches
    - scalability discussion
    - debugging discussion
    - performance discussion