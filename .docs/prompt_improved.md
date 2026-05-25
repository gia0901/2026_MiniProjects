# Thiết kế roadmap interview-oriented production-style cho vị trí Embedded Linux Engineer

Mục tiêu:
Thiết kế roadmap ôn tập toàn diện trong 2 tuần cho vị trí Embedded Linux Engineer (Middle-level trở lên), tập trung vào:
- Modern C++17/20
- Embedded Linux user-space
- Linux system programming
- IPC
- Performance và concurrency
- Hardware communication
- Middleware/service-oriented architecture
- Production-style engineering mindset

Roadmap phải phản ánh tư duy engineering thực tế của Embedded Linux middleware/application engineer:
- maintainability
- debugging mindset
- performance awareness
- concurrency correctness
- ownership/lifetime management
- Linux resource management
- API/interface design
- production-style project organization

Không tập trung vào:
- thuật toán học thuật
- web/backend thông thường
- enterprise architecture phức tạp
- GUI-heavy application
- project toy/demo
- tutorial-style code

---

# Output Requirements

- Toàn bộ kế hoạch phải nằm trong duy nhất 1 file markdown tên `PLAN.md`
- File phải có cấu trúc rõ ràng, dễ đọc và có thể tải về được
- Kế hoạch phải đầy đủ và toàn diện, hạn chế tối đa việc cần trao đổi bổ sung
- Tổng thời lượng roadmap: 2 tuần
- Các project phải ngắn gọn, có thể hoàn thành trong vài giờ
- Ưu tiên breadth + practical interview review thay vì project quá lớn

---

# Background và Technical Focus

## Embedded Linux User-space Focus
Tập trung các chủ đề phổ biến trong Embedded Linux user-space:
- middleware
- dynamic library
- IPC
- Linux daemon/service
- device-driver interaction
- hardware communication
- asynchronous/event-driven architecture
- concurrency
- resource management
- performance awareness

Không tập trung:
- kernel internals chuyên sâu
- BSP development
- low-level kernel hacking
- desktop application development
- enterprise backend patterns

---

# Project Direction

Ưu tiên các project gần với middleware/application thực tế:

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

Project phải có progression logic:
- synchronous → asynchronous
- single-thread → multi-thread
- blocking IO → event-driven
- simple architecture → modular architecture

---

# Coding Style và Engineering Style

## Modern C++
Sử dụng Modern C++17/20 theo hướng production-level Linux application:
- RAII
- smart pointer
- move semantics
- thread/concurrency primitives
- STL containers/algorithms
- ownership management
- resource safety
- modular design

## Engineering Principles
- Không over-engineering
- Không phức tạp hóa vấn đề
- Không viết code kiểu tutorial/demo
- Ưu tiên maintainability
- Clean architecture vừa đủ
- Logging/error handling hợp lý
- Abstraction vừa đủ, tránh over-abstraction

---

# Architecture Requirements

## Module Boundaries
Ưu tiên module boundaries rõ ràng:
- application layer
- middleware/service layer
- hardware abstraction layer (HAL) nếu phù hợp
- Linux abstraction wrapper nếu cần

## Resource Management
Ưu tiên RAII wrapper cho:
- file descriptor
- socket
- thread
- mutex
- mmap
- epoll fd

Giải thích:
- ownership
- lifetime management
- resource cleanup strategy

Hạn chế:
- business logic phụ thuộc trực tiếp vào POSIX APIs
- raw pointer không cần thiết
- hidden lifetime issue

---

# API Design

Public interfaces cần:
- rõ ownership
- rõ thread-safety
- naming convention nhất quán
- tránh raw pointer nếu không cần
- tránh hidden lifetime issue

Nếu phù hợp:
- callback design
- async interface
- plugin-style interface

---

# Concurrency và Event-driven Design

## Multi-threading
Với project multi-thread:
- giải thích concurrency model
- thread ownership
- synchronization strategy
- queue/message-passing strategy
- cách tránh race-condition/deadlock

## Event-driven Architecture
Nếu sử dụng event-driven:
- giải thích lý do chọn:
    - epoll
    - poll
    - select
- mô tả event-loop flow
- blocking vs non-blocking trade-off

---

# Performance Mindset

Với mỗi project:
- phân tích bottleneck tiềm năng
- syscall overhead
- unnecessary copy
- lock contention
- memory allocation frequency
- blocking behavior

Đồng thời:
- chỉ ra hướng optimize thực tế
- phân tích scalability cơ bản
- giải thích trade-off performance vs maintainability

---

# Build System và Packaging

Sử dụng CMake cho tất cả project.

Khuyến khích:
- modular CMake structure
- reusable modules
- shared/static library separation
- interface include management

Nếu phù hợp:
- dlopen/dlsym
- plugin-style architecture
- symbol visibility cơ bản

---

# Testing Requirements

Không cần implement test hoàn chỉnh, nhưng phải đề xuất:
- unit test cases
- integration test cases
- boundary cases
- race-condition scenarios
- failure injection cases
- IPC validation
- synchronization validation
- integration behaviors cần verify

---

# Debugging và Troubleshooting

Với mỗi project:
- đề xuất các bug thực tế có thể xảy ra
- symptom → root cause mapping
- debugging workflow

Kết hợp tooling:
- gdb
- valgrind
- sanitizers
- strace/ltrace
- perf cơ bản
- clang-tidy
- clang-format

---

# Environment

## Linux VM
Các project thông thường:
- chạy trên Linux VM

## Hardware Projects
Nếu cần hardware:
- sử dụng BeagleBone Black

---

# Interview-oriented Requirements

Mỗi project phải liên kết với:
- interview topics thường gặp
- follow-up questions level Middle/Senior
- engineering trade-offs
- common design mistakes
- debugging discussion
- scalability discussion
- performance discussion
- alternative approaches

---

# Project Format

Mỗi project phải có format thống nhất:

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
12. Performance Considerations
13. Common Mistakes
14. Interview Questions
15. Optional Extensions
16. Daily Review Checklist

---

# Presentation Style

- Trình bày trực quan tối đa:
    - diagram
    - architecture flow
    - module relationship
    - event flow
    - threading model
    - IPC flow

- Hạn chế lý thuyết dài dòng
- Ưu tiên practical engineering explanation
- Ưu tiên interview-oriented explanation