# IPC trên Linux (Embedded Linux / Interview-Oriented)

# Tổng Quan

IPC (Inter-Process Communication) là cơ chế cho phép:
- process ↔ process
- service ↔ service
- middleware ↔ daemon
- application ↔ hardware service

giao tiếp với nhau trên Linux.

Trong Embedded Linux:
- IPC là nền tảng của middleware architecture
- ảnh hưởng trực tiếp tới:
  - performance
  - scalability
  - maintainability
  - debugging complexity

---

# Bảng So Sánh IPC

```text
┌────────────────┬───────────────┬────────────┬────────────┬─────────────────────┬──────────────────────┐
│ IPC Mechanism  │ Persistence   │ 2 chiều    │ Cross-host │ Hiệu năng           │ Độ phức tạp          │
├────────────────┼───────────────┼────────────┼────────────┼─────────────────────┼──────────────────────┤
│ Pipe (anonymous)│ Không        │ Không      │ Không      │ Nhanh               │ Rất đơn giản         │
│ FIFO (Named Pipe)│ File system │ Không      │ Không      │ Nhanh               │ Đơn giản             │
│ Unix Domain Socket│ File system│ Có         │ Không      │ Nhanh, linh hoạt    │ Trung bình           │
│ Shared Memory  │ Tới khi unmap │ Thủ công   │ Không      │ Nhanh nhất          │ Khó nhất             │
│ Message Queue  │ Kernel        │ Không      │ Không      │ Trung bình          │ Trung bình           │
│ TCP Socket     │ Không         │ Có         │ Có         │ Overhead cao hơn    │ Trung bình → cao     │
└────────────────┴───────────────┴────────────┴────────────┴─────────────────────┴──────────────────────┘
```

---

# Tư duy chọn IPC trong Embedded Linux

Không có IPC nào "tốt nhất".

Cần chọn theo:
- throughput
- latency
- complexity
- scalability
- maintainability
- debugging difficulty
- data size
- communication pattern

---

# 1. Pipe (Anonymous Pipe)

## Đặc điểm

- IPC đơn giản nhất trên Linux
- thường dùng giữa:
  - parent process
  - child process
- unidirectional
- không có tên trong filesystem

---

## Khi nào nên dùng?

Phù hợp cho:
- process pipeline đơn giản
- redirect output
- shell-like architecture
- producer → consumer đơn giản

---

## Ví dụ thực tế

```text
camera_capture
    ↓ pipe
frame_encoder
```

Hoặc:

```text
stdout redirect
    ↓
log processor
```

---

## Không phù hợp khi:

- nhiều process
- cần bidirectional communication
- protocol phức tạp
- multi-client
- scalable architecture

---

## Ưu điểm

- cực đơn giản
- performance tốt
- kernel support mạnh
- phù hợp streaming nhỏ

---

## Nhược điểm

- một chiều
- khó scale
- không flexible
- khó maintain khi architecture lớn

---

## Linux APIs

```cpp
pipe()
fork()
read()
write()
close()
```

---

## Interview Insight

Pipe phù hợp khi:
- data flow đơn giản
- parent-child relationship rõ ràng
- lifecycle ngắn

---

# 2. FIFO (Named Pipe)

## Đặc điểm

- giống pipe nhưng có tên trong filesystem
- process không cần quan hệ cha-con

Ví dụ:

```text
/tmp/my_fifo
```

---

## Khi nào nên dùng?

Phù hợp cho:
- debug tools
- lightweight IPC
- simple command channel
- local scripting integration

---

## Ví dụ thực tế

```text
debug_console
    ↓
middleware daemon
```

Ví dụ:
- reload config
- trigger log dump
- debug command injection

---

## Không phù hợp khi:

- high throughput
- async event lớn
- nhiều client đồng thời
- binary protocol phức tạp

---

## Ưu điểm

- dễ debug
- dễ inspect
- shell-friendly
- đơn giản hơn socket

---

## Nhược điểm

- vẫn là one-way
- scalability thấp
- synchronization hạn chế

---

## Linux APIs

```cpp
mkfifo()
open()
read()
write()
```

---

## Interview Insight

FIFO:
- đơn giản
- phù hợp lightweight command IPC
- không phù hợp middleware lớn

---

# 3. Unix Domain Socket (UDS)

# IPC quan trọng nhất trong Embedded Linux middleware

---

## Đặc điểm

- bidirectional
- local IPC
- socket API quen thuộc
- hỗ trợ:
  - stream
  - datagram
  - credential passing
  - file descriptor passing

---

## Khi nào nên dùng?

Đây là lựa chọn mặc định cho:
- daemon/service communication
- middleware architecture
- command/control channel
- local microservice-style communication
- multi-client IPC

---

## Ví dụ thực tế

```text
UI process
    ↕
media service
    ↕
audio daemon
```

---

## Vì sao Embedded Linux rất thích UDS?

Vì:
- local-only → nhanh hơn TCP
- API chuẩn socket
- scalable hơn pipe
- dễ event-driven với epoll
- dễ multi-client
- maintainability tốt

---

## Ví dụ thực tế nổi tiếng

- DBus
- Android native services
- multimedia middleware
- logging service
- telemetry daemon

---

## Không phù hợp khi:

- throughput cực lớn
- cần zero-copy
- truyền video/audio frame size lớn

---

## Ưu điểm

- flexible
- scalable
- event-driven friendly
- dễ integrate epoll
- dễ monitor/debug

---

## Nhược điểm

- overhead hơn shared memory
- protocol parsing complexity
- copy cost vẫn tồn tại

---

## Linux APIs

```cpp
socket(AF_UNIX)
bind()
connect()
listen()
accept()
send()
recv()
epoll()
```

---

## Interview Insight

Nếu interviewer hỏi:

> IPC mặc định cho middleware Linux?

=> thường câu trả lời mạnh là:

```text
Unix Domain Socket
```

vì balance tốt giữa:
- performance
- maintainability
- flexibility

---

# 4. Shared Memory

# IPC hiệu năng cao nhất

---

## Đặc điểm

- processes cùng map chung memory
- gần như zero-copy
- kernel không copy data nhiều lần

---

## Khi nào nên dùng?

Rất phù hợp cho:
- video frame
- audio buffer
- sensor stream tốc độ cao
- telemetry throughput lớn
- image processing pipeline

---

## Ví dụ thực tế

```text
camera process
    ↓ shared memory
video renderer
```

Hoặc:

```text
sensor collector
    ↓ shared memory
analytics service
```

---

## Vấn đề lớn nhất

Shared memory:
- KHÔNG có synchronization

Bạn phải tự xử lý:
- mutex
- semaphore
- lock-free
- ring buffer

---

## Các lỗi thường gặp

- race condition
- stale data
- partial write
- deadlock
- cache contention

---

## Ưu điểm

- fastest IPC
- low latency
- high throughput
- minimal copy

---

## Nhược điểm

- synchronization cực khó
- debugging khó
- ownership khó
- maintainability thấp hơn
- complexity cao

---

## Linux APIs

```cpp
shm_open()
mmap()
munmap()
sem_open()
pthread_mutexattr_setpshared()
```

---

## Interview Insight

Shared memory:
- fastest
- hardest

Interviewer thường hỏi:
- synchronization strategy
- producer-consumer model
- lock-free ring buffer
- cache alignment
- consistency guarantee

---

# 5. Message Queue

## Đặc điểm

Kernel quản lý:
- queue
- message boundary
- ordering

---

## Khi nào nên dùng?

Phù hợp cho:
- asynchronous command
- event notification
- lightweight task dispatch
- watchdog event system

---

## Ví dụ thực tế

```text
watchdog manager
    ↓
health event queue
```

Hoặc:

```text
event producer
    ↓
task dispatcher
```

---

## Ưu điểm

- message boundary rõ ràng
- async dễ hơn pipe
- decoupling tốt
- ordering rõ ràng

---

## Nhược điểm

- throughput không quá cao
- kernel overhead
- không phù hợp large streaming

---

## Linux APIs

```cpp
mq_open()
mq_send()
mq_receive()
mq_close()
```

---

## Interview Insight

Message queue phù hợp khi:
- event-oriented
- command-oriented
- moderate throughput
- asynchronous workflow

---

# 6. TCP Socket

## Đặc điểm

- network-capable
- cross-machine
- distributed architecture

---

## Khi nào nên dùng?

Phù hợp cho:
- remote telemetry
- distributed embedded systems
- cloud communication
- remote control
- OTA update service

---

## Ví dụ thực tế

```text
embedded device
    ↕ TCP
cloud server
```

---

## Không nên dùng cho local IPC nếu:

- chỉ local machine
- low latency requirement
- không cần networking

Vì:
- TCP stack overhead
- networking layer unnecessary

---

## Ưu điểm

- scalable
- cross-device
- standardized
- network ecosystem mạnh

---

## Nhược điểm

- overhead cao hơn local IPC
- latency cao hơn
- protocol complexity
- reconnect/retry complexity

---

## Linux APIs

```cpp
socket(AF_INET)
bind()
connect()
listen()
accept()
send()
recv()
```

---

## Interview Insight

Nếu chỉ local IPC:

```text
Unix Domain Socket
```

thường tốt hơn TCP.

---

# So sánh theo Use Case Thực Tế

```text
┌────────────────────────────┬────────────────────────────┐
│ Use Case                   │ IPC phù hợp                │
├────────────────────────────┼────────────────────────────┤
│ Parent-child pipeline      │ Pipe                       │
│ Simple local command       │ FIFO                       │
│ Middleware service IPC     │ Unix Domain Socket         │
│ High-throughput streaming  │ Shared Memory              │
│ Event/task queue           │ Message Queue              │
│ Cross-device communication │ TCP Socket                 │
│ Video/audio frame sharing  │ Shared Memory              │
│ Multi-client local daemon  │ Unix Domain Socket         │
│ Remote telemetry           │ TCP Socket                 │
│ Sensor event dispatch      │ UDS hoặc Message Queue     │
└────────────────────────────┴────────────────────────────┘
```

---

# Architecture Production Thực Tế

## Hybrid IPC Model (rất phổ biến)

```text
Shared Memory
    +
Unix Domain Socket
```

Ví dụ:

```text
UDS:
control + notification

Shared memory:
large data
```

---

## Tại sao hybrid model phổ biến?

Vì:
- UDS:
  - flexible
  - maintainable
  - control path tốt

- Shared memory:
  - throughput cực cao
  - data path tốt

---

## Xuất hiện trong:

- multimedia pipeline
- camera system
- display stack
- telemetry platform
- sensor framework

---

# IPC Recommendation cho Embedded Linux

## IPC mặc định nên nghĩ tới đầu tiên

### 1. Unix Domain Socket

Khi:
- middleware
- daemon
- service communication
- local IPC
- event-driven system

---

### 2. Shared Memory

Khi:
- performance critical
- video/audio frame
- telemetry throughput lớn
- large streaming

---

### 3. Message Queue

Khi:
- async event
- command dispatch
- moderate throughput

---

### 4. TCP Socket

Khi:
- cross-machine communication
- cloud connectivity
- distributed embedded system

---

# Góc nhìn Middle/Senior Interview

Bạn nên trả lời được:

---

## 1. Tại sao shared memory nhanh?

Vì:
- tránh copy qua kernel nhiều lần
- processes cùng map memory

---

## 2. Tại sao UDS phổ biến?

Vì:
- API quen thuộc
- dễ maintain
- hỗ trợ multi-client
- dễ integrate epoll
- local-only nên nhanh hơn TCP

---

## 3. Khi nào KHÔNG nên dùng shared memory?

Khi:
- synchronization complexity quá cao
- throughput không thực sự cần
- maintainability quan trọng hơn raw performance

---

## 4. Architecture production thường dùng gì?

Rất thường là:

```text
Control path:
Unix Domain Socket

Data path:
Shared Memory
```

Đây là câu trả lời rất mạnh trong interview Embedded Linux Middleware.

---

# Checklist Ôn Tập IPC

Bạn nên đạt được khả năng:

- hiểu trade-off từng IPC
- chọn IPC phù hợp theo use case
- hiểu synchronization issue
- giải thích throughput vs maintainability
- hiểu event-driven IPC
- dùng epoll với UDS
- hiểu shared memory synchronization
- phân biệt control path vs data path
- giải thích architecture production thực tế
- trả lời được trade-off Middle/Senior interview
