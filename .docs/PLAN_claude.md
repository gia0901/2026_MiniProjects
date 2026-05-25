# Embedded Linux Engineer — Interview-Oriented 2-Week Roadmap

> **Target Role:** Embedded Linux Engineer (Middle–Senior level)
> **Duration:** 14 days | **Style:** Production-grade, interview-focused
> **Stack:** Modern C++17/20 · Linux User-space · CMake · POSIX APIs

---

## Table of Contents

1. [Overview & Philosophy](#overview--philosophy)
2. [Weekly Schedule](#weekly-schedule)
3. [Core Skill Matrix](#core-skill-matrix)
4. [Week 1: Foundations & Core Patterns](#week-1-foundations--core-patterns)
   - Day 1–2: Modern C++17/20 Production Patterns
   - Day 3–4: Linux System Programming & IPC
   - Day 5–7: Project 1 — Sensor Monitoring Service
5. [Week 2: Concurrency, Event-driven & Integration](#week-2-concurrency-event-driven--integration)
   - Day 8–9: Concurrency & Event-driven Architecture
   - Day 10–11: Hardware Communication & HAL
   - Day 12–14: Project 2 — Async Device Event Dispatcher
6. [Project Catalog](#project-catalog)
   - Project A: Sensor Monitoring Service
   - Project B: UART Communication Daemon
   - Project C: Async Device Event Dispatcher
   - Project D: Plugin-based Middleware
7. [Interview Deep-dives](#interview-deep-dives)
8. [Tooling Quick-reference](#tooling-quick-reference)
9. [Daily Review Checklist](#daily-review-checklist)

---

## Overview & Philosophy

### Engineering Mindset (not tutorial mindset)

```
Tutorial mindset          Production mindset
─────────────────         ──────────────────────────────
"it compiles & runs"  →   "what happens under resource pressure?"
"simple example"      →   "clean API boundaries, clear ownership"
"no error handling"   →   "every syscall can fail — handle it"
"single-thread demo"  →   "what's the concurrency model?"
"works on my machine" →   "what does valgrind / sanitizer say?"
```

### Progression Logic (applied to all projects)

```
synchronous  ──►  asynchronous
single-thread ──►  multi-thread
blocking I/O  ──►  event-driven (epoll)
monolithic    ──►  modular / plugin-style
raw POSIX     ──►  RAII-wrapped Linux resources
```

---

## Weekly Schedule

```
WEEK 1 — Foundations
─────────────────────────────────────────────────────────────────
Mon  Tue  Wed  Thu  Fri  Sat  Sun
[D1] [D2] [D3] [D4] [D5] [D6] [D7]
C++  C++  IPC  IPC  P1:  P1:  P1:
17   20   fd   msg  Arc  Impl Rev
                    ↑ Sensor Monitoring Service

WEEK 2 — Concurrency & Integration
─────────────────────────────────────────────────────────────────
Mon  Tue  Wed  Thu  Fri  Sat  Sun
[D8] [D9] [D10][D11][D12][D13][D14]
Thr  epol UART HAL  P2:  P2:  Mock
dng  l/ev comm abs  Arc  Impl Intv
                    ↑ Async Device Dispatcher
```

---

## Core Skill Matrix

| Domain | Topics | Interview Weight |
|--------|--------|-----------------|
| Modern C++17/20 | RAII, smart ptr, move semantics, std::variant, if constexpr | ★★★★★ |
| Linux FDs & Resources | open/close/read/write, mmap, epoll, signalfd, timerfd | ★★★★★ |
| IPC | pipe, FIFO, Unix socket, shared memory, message queue | ★★★★☆ |
| Concurrency | std::thread, mutex, condition_variable, lock-free basics | ★★★★★ |
| Event-driven | epoll edge/level-triggered, event loop design, non-blocking I/O | ★★★★☆ |
| UART / Serial | termios, baud rate, framing, read strategies | ★★★☆☆ |
| CMake | modular structure, shared/static lib, interface includes | ★★★☆☆ |
| Debugging | gdb, valgrind, sanitizers, strace, perf | ★★★★☆ |

---

## Week 1: Foundations & Core Patterns

### Day 1–2 — Modern C++17/20 Production Patterns

#### RAII Pattern (Non-negotiable in interview)

```cpp
// ✅ Production-style file descriptor wrapper
class FileDescriptor {
public:
    explicit FileDescriptor(int fd) noexcept : fd_(fd) {}
    ~FileDescriptor() { if (fd_ >= 0) ::close(fd_); }

    // Non-copyable, movable
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;
    FileDescriptor(FileDescriptor&& o) noexcept : fd_(std::exchange(o.fd_, -1)) {}
    FileDescriptor& operator=(FileDescriptor&& o) noexcept {
        if (this != &o) { reset(); fd_ = std::exchange(o.fd_, -1); }
        return *this;
    }

    int get() const noexcept { return fd_; }
    bool valid() const noexcept { return fd_ >= 0; }
    void reset() noexcept { if (fd_ >= 0) { ::close(fd_); fd_ = -1; } }

private:
    int fd_{-1};
};
```

**Why this matters in interview:**
- Shows understanding of ownership semantics
- Shows understanding of exception safety
- Shows you know when to use `std::exchange`
- Demonstrates move-only resource handles

---

#### Smart Pointer Ownership Models

```
Ownership models
────────────────────────────────────────────────────
unique_ptr<T>  → sole owner, move-only
                 Use for: resources, subsystem handles

shared_ptr<T>  → shared ownership, ref-counted
                 Use for: callbacks that outlive caller,
                          shared config/state

weak_ptr<T>    → non-owning observer
                 Use for: cache, optional back-reference,
                          break circular refs

raw T*         → non-owning borrow (observe only)
                 Use for: function params that don't store
```

**Interview red flag:** `shared_ptr` everywhere = not thinking about ownership. Use `unique_ptr` by default.

---

#### std::variant & Error Handling (C++17)

```cpp
// Production error handling — no exceptions in embedded
using ReadResult = std::variant<std::vector<uint8_t>, std::error_code>;

ReadResult read_device(const FileDescriptor& fd, size_t len) {
    std::vector<uint8_t> buf(len);
    ssize_t n = ::read(fd.get(), buf.data(), len);
    if (n < 0) return std::make_error_code(static_cast<std::errc>(errno));
    buf.resize(static_cast<size_t>(n));
    return buf;
}

// Caller
auto result = read_device(fd, 64);
std::visit(overloaded{
    [](const std::vector<uint8_t>& data) { /* process */ },
    [](const std::error_code& ec) { /* log error */ }
}, result);
```

---

#### Move Semantics — Interview Essentials

```
Copy vs Move
─────────────────────────────────────────────────────
T obj2 = obj1;        → copy constructor (T copied)
T obj2 = std::move(obj1); → move constructor (resources transferred)
                           obj1 is valid but unspecified after move

Rule of Five (if custom destructor):
  ~T()                  → destructor
  T(const T&)           → copy constructor
  T& operator=(const T&)→ copy assignment
  T(T&&)                → move constructor
  T& operator=(T&&)     → move assignment

Rule of Zero (preferred):
  Let compiler generate all 5 — use RAII members
```

---

#### C++17 Key Features for Embedded

| Feature | Use Case |
|---------|----------|
| `std::optional<T>` | Return value that may not exist |
| `std::variant<Ts...>` | Tagged union, type-safe error/result |
| `if constexpr` | Compile-time branching without SFINAE |
| `std::string_view` | Non-owning string reference, no alloc |
| Structured bindings | `auto [fd, err] = open_device(...)` |
| `[[nodiscard]]` | Force caller to check return value |

---

### Day 3–4 — Linux System Programming & IPC

#### File Descriptor Lifecycle

```
┌─────────────────────────────────────────────────────┐
│                  FD Lifecycle                        │
│                                                      │
│  open() / socket() / pipe() / epoll_create()         │
│         │                                            │
│         ▼                                            │
│   fd = [3, 4, 5, ...]   ← per-process table         │
│         │                                            │
│         ├─ read() / write() / ioctl() / mmap()      │
│         │                                            │
│         ├─ epoll_ctl(epfd, EPOLL_CTL_ADD, fd, ...)  │
│         │                                            │
│         └─ close(fd)  ← MUST happen, or fd leak     │
│                                                      │
│  Check: /proc/<pid>/fd/                              │
└─────────────────────────────────────────────────────┘
```

**Interview Q:** What happens if you fork() and don't close fds in child?
**A:** Child inherits all open fds. Pipe read-end in child prevents writer's EOF signal. Always close unused ends after fork/exec.

---

#### IPC Comparison

```
IPC Mechanism Comparison
──────────────────────────────────────────────────────────────────
Mechanism      | Persistence | Bidirec | Cross-host | Perf
───────────────┼─────────────┼─────────┼────────────┼────────────
Pipe (anon)    | No          | No      | No         | Fast
Named FIFO     | Filesystem  | No      | No         | Fast
Unix Socket    | Filesystem  | Yes     | No         | Fast, flexible
Shared Memory  | Until unmap | Manual  | No         | Fastest
Msg Queue      | Kernel      | No      | No         | Medium
TCP Socket     | No          | Yes     | Yes        | Overhead
──────────────────────────────────────────────────────────────────
Embedded typical: Unix socket (flexibility) or shared mem (perf)
```

---

#### epoll vs poll vs select

```
epoll (preferred for embedded services)
────────────────────────────────────────────────────────
+ O(1) per event (not O(n) like poll/select)
+ Edge-triggered mode available
+ Can monitor thousands of fds efficiently
+ Works with signalfd, timerfd, eventfd
- Linux-only

poll
────────────────────────────────────────────────────────
+ Portable (POSIX)
+ No fd_set limit like select
- O(n) scan per call
- Pass entire array each call

select
────────────────────────────────────────────────────────
- Limited to FD_SETSIZE (usually 1024)
- O(n) scan
- Modifies fd_sets (must reset each call)
+ Highest portability

Decision: Use epoll for production Linux daemons.
          Use poll for portable/simpler code.
          Avoid select for new code.
```

---

#### epoll Edge-Triggered vs Level-Triggered

```
Level-Triggered (EPOLLIN, default)
───────────────────────────────────
Fires as long as data is available.
Safe: if you miss an event, next epoll_wait fires again.
Risk: can busy-loop if you don't drain the buffer.

Edge-Triggered (EPOLLIN | EPOLLET)
───────────────────────────────────
Fires only on state CHANGE (new data arrival).
MUST: read until EAGAIN/EWOULDBLOCK.
MUST: use non-blocking fd.
Risk: miss events if buffer not fully drained.
Perf: fewer epoll_wait wakeups = better for high-frequency events.

Rule of thumb:
  Default → Level-triggered (correctness first)
  High-throughput → Edge-triggered (with discipline)
```

---

### Day 5–7 — Project 1: Sensor Monitoring Service

See [Project A: Sensor Monitoring Service](#project-a-sensor-monitoring-service) for full detail.

**Day 5:** Architecture design, RAII wrappers, CMake setup
**Day 6:** Core implementation — reader thread, queue, logger
**Day 7:** Testing, debugging with valgrind/sanitizers, review questions

---

## Week 2: Concurrency, Event-driven & Integration

### Day 8–9 — Concurrency & Event-driven Architecture

#### Threading Model Choices

```
Threading Models
───────────────────────────────────────────────────────────────
Model             | Use When
──────────────────┼────────────────────────────────────────────
Thread-per-task   | Simple, low task count, blocking I/O OK
Thread pool       | Many short tasks, bounded parallelism
Single-thread +   | Many I/O events, low CPU, latency-sensitive
epoll event loop  |
Producer-Consumer | Pipeline stages, rate decoupling
Actor model       | Complex message-passing, isolation needed
───────────────────────────────────────────────────────────────
Embedded typical: Single event loop (epoll) OR
                  1 I/O thread + 1 worker thread
```

---

#### Producer-Consumer with Condition Variable

```cpp
// Thread-safe bounded queue (production pattern)
template<typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(size_t cap) : capacity_(cap) {}

    bool push(T item, std::chrono::milliseconds timeout = {}) {
        std::unique_lock lock(mutex_);
        if (!not_full_.wait_for(lock, timeout,
                [this]{ return queue_.size() < capacity_ || stopped_; }))
            return false;
        if (stopped_) return false;
        queue_.push(std::move(item));
        not_empty_.notify_one();
        return true;
    }

    std::optional<T> pop(std::chrono::milliseconds timeout = {}) {
        std::unique_lock lock(mutex_);
        if (!not_empty_.wait_for(lock, timeout,
                [this]{ return !queue_.empty() || stopped_; }))
            return std::nullopt;
        if (queue_.empty()) return std::nullopt;
        T item = std::move(queue_.front());
        queue_.pop();
        not_full_.notify_one();
        return item;
    }

    void stop() {
        { std::lock_guard lock(mutex_); stopped_ = true; }
        not_full_.notify_all();
        not_empty_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable not_full_, not_empty_;
    size_t capacity_;
    bool stopped_{false};
};
```

**Interview questions:**
- Why `not_full_.notify_all()` in stop()? (wake all blocked pushers to exit)
- What is spurious wakeup and why does the lambda predicate protect against it?
- What's the difference between `notify_one()` and `notify_all()` here?

---

#### Deadlock Prevention Rules

```
Rules (memorize these for interview)
─────────────────────────────────────────────────────
1. Lock ordering: Always acquire locks in same global order
2. Lock duration: Hold locks as short as possible
3. No lock while calling external code / callbacks
4. Prefer std::scoped_lock for multi-lock acquire
5. Avoid recursive locking with std::mutex

Deadlock detection:
  strace -p <pid>      → check if blocked on futex
  gdb: thread apply all bt  → see all thread stacks
  valgrind --tool=helgrind  → race + deadlock analysis
```

---

#### std::atomic and Memory Ordering

```cpp
// Common embedded pattern: atomic flag for shutdown
std::atomic<bool> running_{true};

// Thread 1 (main):
running_.store(false, std::memory_order_relaxed);

// Thread 2 (worker):
while (running_.load(std::memory_order_relaxed)) {
    // do work
}

// Memory ordering quick guide:
// relaxed  → no ordering, just atomicity (counter, flag)
// acquire  → see all writes before the release
// release  → publish writes to acquiring thread
// seq_cst  → total order, strongest, default
//
// Interview: when would you use acquire/release?
// → Producer sets data, then store(flag, release)
// → Consumer load(flag, acquire), then reads data safely
```

---

### Day 10–11 — Hardware Communication & HAL

#### UART / Serial Communication

```
UART Frame Structure
────────────────────────────────────────────────
[Start][D0][D1][D2][D3][D4][D5][D6][D7][Parity][Stop]
  1bit  ───── 8 data bits ─────  opt    1-2bit

termios settings (C++ wrapper):
  B115200       → baud rate
  CS8           → 8 data bits
  CLOCAL|CREAD  → enable receiver, local mode
  ~PARENB       → no parity
  ~CSTOPB       → 1 stop bit
  VMIN=1        → read blocks until 1 byte
  VTIME=0       → no timeout
```

```cpp
// RAII UART port wrapper
class UartPort {
public:
    explicit UartPort(const std::string& device, speed_t baud) {
        fd_ = FileDescriptor(::open(device.c_str(), O_RDWR | O_NOCTTY));
        if (!fd_.valid()) throw std::system_error(errno, std::system_category());
        configure(baud);
    }

    ssize_t write(std::span<const uint8_t> data) {
        return ::write(fd_.get(), data.data(), data.size());
    }

    ssize_t read(std::span<uint8_t> buf) {
        return ::read(fd_.get(), buf.data(), buf.size());
    }

private:
    void configure(speed_t baud) {
        struct termios tty{};
        tcgetattr(fd_.get(), &tty);
        cfsetispeed(&tty, baud);
        cfsetospeed(&tty, baud);
        cfmakeraw(&tty);  // raw mode
        tty.c_cc[VMIN] = 1;
        tty.c_cc[VTIME] = 0;
        tcsetattr(fd_.get(), TCSANOW, &tty);
    }
    FileDescriptor fd_;
};
```

---

#### Hardware Abstraction Layer (HAL) Design

```
HAL Architecture
─────────────────────────────────────────────────────────────
Application Layer
      │  (uses abstract interface only)
      ▼
┌─────────────────────────────────────────────────────┐
│              IDeviceDriver (pure virtual)            │
│  + open() + close() + read() + write() + ioctl()    │
└─────────────────────────────────────────────────────┘
      │
      ├── UartDriver (real hardware: /dev/ttyS0)
      ├── SpiDriver  (real hardware: /dev/spidev0.0)
      ├── MockDriver (unit testing, no hardware)
      └── FileDriver (integration: uses regular file)

Benefits:
  - Unit test without hardware
  - Swap driver without changing app logic
  - Clean ownership boundary
```

---

#### sysfs / devfs Interaction

```bash
# Read GPIO (sysfs classic — kernel < 4.8)
echo 17 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio17/direction
cat /sys/class/gpio/gpio17/value

# Modern: character device /dev/gpiochipN (libgpiod)
# SPI device: /dev/spidev0.0
# I2C device: /dev/i2c-1

# In C++: open as regular fd, use read/write/ioctl
# Embed in RAII class → same pattern as FileDescriptor above
```

**Interview Q:** Why prefer `/dev/gpiochip` over sysfs?
**A:** sysfs GPIO deprecated since kernel 4.8. chardev is faster, supports events (GPIO interrupts via poll), non-root capable with proper permissions, and doesn't pollute sysfs namespace.

---

### Day 12–14 — Project 2: Async Device Event Dispatcher

See [Project C: Async Device Event Dispatcher](#project-c-async-device-event-dispatcher) for full detail.

**Day 12:** Architecture — epoll event loop + dispatcher design
**Day 13:** Implementation — event registration, worker threads, IPC
**Day 14:** Integration test, mock interview Q&A

---

## Project Catalog

---

## Project A: Sensor Monitoring Service

### 1. Problem Scenario

You're building a middleware service that reads from multiple sensor files (simulated via `/dev/` or named files), validates readings, queues them, and dispatches to subscribers. Must handle sensor dropout gracefully.

### 2. Interview Focus

- RAII resource management
- Thread-safe queue design
- Error propagation strategy
- Graceful shutdown (signal handling)
- Logging architecture

### 3. Key Modern C++ Concepts

- `unique_ptr` for resource ownership
- `std::optional<T>` for nullable readings
- `std::variant<Reading, Error>` for result type
- Move semantics for queue elements
- `std::atomic<bool>` for shutdown flag
- `[[nodiscard]]` on read functions

### 4. Key Linux Concepts

- File descriptor lifecycle
- `signalfd` for async signal handling
- `timerfd` for periodic polling
- `/proc/self/fd` for fd leak check
- `syslog` / structured logging

### 5. Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                   Sensor Monitoring Service                      │
│                                                                  │
│  ┌──────────────┐   ┌──────────────────┐   ┌────────────────┐  │
│  │ SensorReader  │   │  BoundedQueue<T> │   │  Dispatcher   │  │
│  │              │──►│                  │──►│               │  │
│  │ timerfd poll │   │  thread-safe     │   │ subscriber    │  │
│  │ per sensor   │   │  bounded MPSC    │   │ callbacks     │  │
│  └──────────────┘   └──────────────────┘   └────────────────┘  │
│         │                                          │             │
│         ▼                                          ▼             │
│  ┌──────────────┐                      ┌────────────────────┐  │
│  │  HAL Layer   │                      │   Logger (async)   │  │
│  │  FileDriver  │                      │   ring buffer      │  │
│  │  MockDriver  │                      │   syslog / file    │  │
│  └──────────────┘                      └────────────────────┘  │
│                                                                  │
│  Signal handling: signalfd → SIGTERM/SIGINT → graceful stop     │
└─────────────────────────────────────────────────────────────────┘
```

### 6. Project Structure

```
sensor_monitor/
├── CMakeLists.txt
├── lib/
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── sensor_monitor/fd.hpp           ← FileDescriptor RAII
│   │   ├── sensor_monitor/sensor.hpp       ← SensorReading type
│   │   ├── sensor_monitor/queue.hpp        ← BoundedQueue<T>
│   │   ├── sensor_monitor/idriver.hpp      ← IDeviceDriver interface
│   │   └── sensor_monitor/logger.hpp       ← Logger interface
│   └── src/
│       ├── reader.cpp                      ← SensorReader impl
│       ├── dispatcher.cpp                  ← Dispatcher impl
│       ├── file_driver.cpp                 ← File-based HAL
│       └── logger.cpp                      ← Async logger impl
├── app/
│   ├── CMakeLists.txt
│   └── main.cpp                            ← entry, signal setup
└── test/
    ├── CMakeLists.txt
    ├── test_queue.cpp
    └── test_reader.cpp
```

### 7. Core Components

#### SensorReading type
```cpp
struct SensorReading {
    std::string sensor_id;
    double value;
    std::chrono::steady_clock::time_point timestamp;
    enum class Status { OK, OUT_OF_RANGE, TIMEOUT } status;
};

using ReadResult = std::variant<SensorReading, std::error_code>;
```

#### Graceful Shutdown (signalfd pattern)
```cpp
// In main: replace signal() with signalfd
sigset_t mask;
sigemptyset(&mask);
sigaddset(&mask, SIGTERM);
sigaddset(&mask, SIGINT);
sigprocmask(SIG_BLOCK, &mask, nullptr);  // block default handlers

int sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
// Add sfd to epoll — handle SIGTERM like a regular read event
// No async-signal-safety concerns!
```

### 8. Build Instructions

```cmake
# lib/CMakeLists.txt
add_library(sensor_monitor_lib STATIC
    src/reader.cpp src/dispatcher.cpp
    src/file_driver.cpp src/logger.cpp)

target_include_directories(sensor_monitor_lib
    PUBLIC include)

target_compile_features(sensor_monitor_lib PUBLIC cxx_std_17)

# Enable warnings + sanitizers in debug
target_compile_options(sensor_monitor_lib PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>)

target_link_options(sensor_monitor_lib PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
```

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

### 9. Suggested Implementation Steps

1. `FileDescriptor` RAII class + unit test
2. `BoundedQueue<T>` with stop() support
3. `IDeviceDriver` interface + `MockDriver`
4. `SensorReader` — reads via driver, pushes to queue
5. `Dispatcher` — pops from queue, calls subscribers
6. `Logger` — async, ring buffer backed
7. `main.cpp` — wire up with signalfd shutdown
8. Valgrind + ThreadSanitizer run

### 10. Testing Strategy

| Test Type | What to Test |
|-----------|-------------|
| Unit | BoundedQueue: push/pop/stop/timeout |
| Unit | FileDescriptor: move semantics, close on destroy |
| Unit | SensorReader: mock driver returns error → error propagated |
| Integration | Start service → simulate sensor data → verify dispatch |
| Race | ThreadSanitizer: concurrent push/pop |
| Failure | Inject read errors → service continues for other sensors |
| Shutdown | Send SIGTERM → all queued data flushed before exit |

### 11. Debugging Strategy

```
Symptom → Root Cause → Tool
─────────────────────────────────────────────────────────────────
Fd leak         → RAII not applied      → /proc/<pid>/fd, valgrind
Thread hangs    → Deadlock or missing   → gdb: thread apply all bt
                  notify                  helgrind
Data loss       → Queue full, drop      → Add queue stats counter
                  silently
High CPU        → Busy-poll loop,       → perf top, strace
                  missing sleep/epoll
SIGTERM ignored → Signal not blocked    → strace -e trace=signal
                  before signalfd
Memory error    → use-after-free,       → AddressSanitizer
                  buffer overflow
```

### 12. Performance Considerations

```
Bottleneck Analysis:
─────────────────────────────────────────────────────────────────
Component          | Bottleneck           | Optimization
───────────────────┼──────────────────────┼─────────────────────
SensorReader       | syscall overhead     | batch reads, timerfd
BoundedQueue       | lock contention      | per-sensor queues or
                   |                      | lock-free (later)
Dispatcher         | callback duration    | offload to worker pool
Logger             | write syscall        | async + ring buffer
                   | per log line         | batch flush
───────────────────────────────────────────────────────────────
Measure first with perf. Don't optimize speculatively.
```

### 13. Common Mistakes

- Forgetting `sigprocmask` before `signalfd` → default handler fires
- `shared_ptr` for everything → unclear ownership, ref-cycle risk
- Not handling `EINTR` on `read()` → spurious failures
- Locking too coarsely → serializes unrelated work
- Not draining queue before exit → data loss on shutdown
- `std::endl` in hot path → flushes on every log line (use `"\n"`)

### 14. Interview Questions

**Level: Middle**
- Explain RAII and why it matters for file descriptors
- What is a spurious wakeup? How do you protect against it?
- Why use `signalfd` instead of `signal()`?
- What is `EINTR` and when does it occur?

**Level: Senior**
- How would you make the queue lock-free? What are the trade-offs?
- How do you detect and prevent deadlock in a multi-subscriber dispatcher?
- How would you scale this to 100 sensors? 1000?
- Design a zero-copy path between reader and dispatcher

**Trade-off discussions:**
- MPSC vs SPSC queue: when to use each
- Bounded vs unbounded queue: back-pressure implications
- Shared queue vs per-sensor queue: latency vs simplicity

### 15. Optional Extensions

- Add metrics endpoint (Unix socket, JSON over POSIX IPC)
- Persistent ring buffer using `mmap` (survives process restart)
- Priority queue: critical sensors have low-latency path
- Watchdog integration: kick watchdog from dispatcher

### 16. Daily Review Checklist (Project A)

- [ ] Every fd is wrapped in RAII
- [ ] No raw `new`/`delete` in non-test code
- [ ] ThreadSanitizer: zero races
- [ ] AddressSanitizer: zero errors
- [ ] valgrind: zero leaks
- [ ] SIGTERM triggers graceful shutdown in < 1s
- [ ] Can explain BoundedQueue locking strategy
- [ ] Can explain signalfd vs signal()

---

## Project B: UART Communication Daemon

### 1. Problem Scenario

A Linux daemon that manages UART communication with an embedded MCU. Handles framed messages, retransmit on timeout, and exposes a Unix socket API to client applications.

### 2. Interview Focus

- UART/termios configuration
- Protocol framing (header + length + payload + CRC)
- Async I/O on serial device
- Unix socket server for client API
- Daemon lifecycle (fork, setsid, PID file)

### 3. Key Modern C++ Concepts

- `std::span<uint8_t>` for buffer views (zero-copy)
- `std::byte` for raw data
- Coroutine or state machine for framing
- `std::chrono` for timeout tracking

### 4. Key Linux Concepts

- `termios` API (cfmakeraw, cfsetspeed)
- Non-blocking serial read + epoll
- `VMIN`/`VTIME` vs non-blocking
- `AF_UNIX` / `SOCK_SEQPACKET` for local IPC
- `daemon()`, PID file, `/var/run/`

### 5. Architecture Overview

```
┌───────────────────────────────────────────────────────┐
│                  UART Daemon (uartd)                  │
│                                                       │
│  Client A         Client B                            │
│     │                │                               │
│     └────────┬───────┘                               │
│              ▼                                        │
│  ┌─────────────────────┐   ┌──────────────────────┐  │
│  │  Unix Socket Server  │   │   UART Port Driver   │  │
│  │  /var/run/uartd.sock │   │   /dev/ttyS0         │  │
│  │  SOCK_SEQPACKET      │   │   non-blocking       │  │
│  └──────────┬──────────┘   └──────────┬───────────┘  │
│             │                          │               │
│             └────────────┬─────────────┘              │
│                          ▼                             │
│              ┌───────────────────────┐                │
│              │     epoll Event Loop  │                │
│              │  signalfd | timerfd   │                │
│              └───────────────────────┘                │
└───────────────────────────────────────────────────────┘
```

### 6. Message Framing Protocol

```
Frame format:
┌────────┬────────┬──────────┬─────────────────┬────────┐
│ SYNC   │ CMD    │ LEN      │ PAYLOAD         │ CRC16  │
│ 0xAA55 │ 1 byte │ 2 bytes  │ N bytes (≤256)  │ 2 bytes│
└────────┴────────┴──────────┴─────────────────┴────────┘

Parser states:
  WAIT_SYNC → WAIT_CMD → WAIT_LEN → WAIT_PAYLOAD → WAIT_CRC → COMPLETE

Non-blocking read: accumulate in byte buffer, run state machine on each byte
```

### 7. Daemon Lifecycle

```cpp
// Proper daemon initialization
void daemonize(const char* pid_file) {
    // 1. Fork and exit parent
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);  // parent exits

    // 2. New session
    if (setsid() < 0) exit(EXIT_FAILURE);

    // 3. Fork again (prevent TTY re-acquisition)
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    // 4. Set file permissions, chdir
    umask(0);
    chdir("/");

    // 5. Close standard fds, redirect to /dev/null
    close(STDIN_FILENO); close(STDOUT_FILENO); close(STDERR_FILENO);
    open("/dev/null", O_RDONLY);  // stdin=0
    open("/dev/null", O_WRONLY);  // stdout=1
    open("/dev/null", O_WRONLY);  // stderr=2

    // 6. Write PID file
    // ...
}
```

### 8. Testing Strategy

| Test | Description |
|------|-------------|
| Framing | Feed partial frames → no crash, no stale state |
| CRC mismatch | Bad CRC → frame discarded, parser resets |
| Timeout | MCU no response → retransmit after N ms |
| Client connect | 2 clients simultaneously → both receive responses |
| Daemon | Process in ps, PID file correct, logs to syslog |

### 9. Interview Questions

- Explain VMIN=1 VTIME=0 vs VMIN=0 VTIME=10
- Why double-fork for daemon?
- How to ensure only one daemon instance runs?
- What happens if client disconnects mid-transfer on Unix socket?
- Explain `SOCK_SEQPACKET` vs `SOCK_STREAM`

### 10. Common Mistakes

- Forgetting `cfmakeraw()` → echo, canonical mode active
- Not flushing termios with `TCSANOW` vs `TCSADRAIN`
- Single-byte reads in a loop → excessive syscall overhead
- Not handling `EAGAIN` on non-blocking read

---

## Project C: Async Device Event Dispatcher

### 1. Problem Scenario

Middleware service that listens on multiple device event sources (simulated as fds: pipes, Unix sockets, timerfd, eventfd) and dispatches events to registered handlers. Must support dynamic handler registration at runtime.

### 2. Interview Focus

- epoll-based event loop (production quality)
- Dynamic handler registration (thread-safe)
- Worker thread pool for handler execution
- Backpressure and queue overflow handling
- Clean API design (callback / observer pattern)

### 3. Key Modern C++ Concepts

- `std::function<void(Event)>` as handler type
- `std::unordered_map<int, Handler>` fd → handler
- `std::shared_mutex` (readers-writer lock) for handler map
- `eventfd` for cross-thread wakeup
- `std::atomic<uint64_t>` for stats

### 4. Architecture Overview

```
┌──────────────────────────────────────────────────────────────┐
│              Async Device Event Dispatcher                    │
│                                                              │
│   Device Sources             Event Loop Thread               │
│   ┌──────────┐              ┌─────────────────────────────┐ │
│   │ timerfd  │──────────────►                             │ │
│   │ eventfd  │              │  epoll_wait(epfd, events)   │ │
│   │ pipe     │──────────────►                             │ │
│   │ unixsock │              │  for each event:            │ │
│   └──────────┘              │    lookup handler by fd     │ │
│                              │    post to worker queue    │ │
│   Control (other threads)   └────────────┬────────────────┘ │
│   ┌──────────────────┐                   │                   │
│   │ register_handler │──► eventfd wake   │                   │
│   │ unregister       │                   ▼                   │
│   └──────────────────┘      ┌─────────────────────────────┐ │
│                              │   Worker Thread Pool        │ │
│                              │   (N threads)               │ │
│                              │   BoundedQueue<Task>        │ │
│                              └─────────────────────────────┘ │
└──────────────────────────────────────────────────────────────┘
```

### 5. Event Loop Core

```cpp
class EventDispatcher {
public:
    using Handler = std::function<void(int fd, uint32_t events)>;

    void register_handler(int fd, uint32_t epoll_events, Handler handler) {
        {
            std::unique_lock lock(handler_mutex_);
            handlers_[fd] = std::move(handler);
        }
        struct epoll_event ev{};
        ev.events = epoll_events;
        ev.data.fd = fd;
        epoll_ctl(epoll_fd_.get(), EPOLL_CTL_ADD, fd, &ev);
        // Wake up event loop if blocked in epoll_wait
        uint64_t val = 1;
        write(wakeup_fd_.get(), &val, sizeof(val));
    }

    void run() {
        std::array<epoll_event, 64> events;
        while (running_) {
            int n = epoll_wait(epoll_fd_.get(), events.data(), events.size(), -1);
            if (n < 0) { if (errno == EINTR) continue; break; }
            for (int i = 0; i < n; ++i) {
                int fd = events[i].data.fd;
                if (fd == wakeup_fd_.get()) {
                    uint64_t val; read(fd, &val, sizeof(val));  // drain
                    continue;
                }
                Handler h;
                {
                    std::shared_lock lock(handler_mutex_);
                    auto it = handlers_.find(fd);
                    if (it != handlers_.end()) h = it->second;
                }
                if (h) worker_queue_.push(Task{fd, events[i].events, std::move(h)});
            }
        }
    }

private:
    FileDescriptor epoll_fd_;
    FileDescriptor wakeup_fd_;  // eventfd
    std::unordered_map<int, Handler> handlers_;
    std::shared_mutex handler_mutex_;
    BoundedQueue<Task> worker_queue_;
    std::atomic<bool> running_{true};
};
```

### 6. Threading Model

```
Threads:
─────────────────────────────────────────────
1x epoll thread   : event detection only, NO blocking
Nx worker threads : handler execution (may be slow)
1x control thread : register/unregister handlers

Synchronization:
  epoll thread → worker queue: push (non-blocking preferred)
  control thread → handler map: std::shared_mutex (R/W lock)
  control thread → epoll thread: eventfd wakeup

Key insight: epoll thread must NEVER call handler directly
             if handler can block → starves all other events
```

### 7. Testing Strategy

| Test | Setup |
|------|-------|
| Basic dispatch | Create timerfd, register handler, verify fires |
| Dynamic register | Register handler from worker thread while loop running |
| Unregister | Remove fd → verify no more events |
| Overload | 1000 events/s → verify no dropped events |
| Slow handler | 100ms handler → verify other fds not starved |
| Shutdown | stop() → all workers drain and exit |

### 8. Debugging Strategy

```
Symptom → Debug approach
──────────────────────────────────────────────────────
Handler never fires   → strace: is epoll_wait returning?
                        gdb: is fd added to epoll?
Race in handler map   → ThreadSanitizer
                        Check shared_mutex usage
eventfd not draining  → epoll busy-loop (100% CPU)
                        Always read eventfd on wakeup event
Worker queue full     → Slow handlers blocking queue
                        Add queue depth metric
```

### 9. Interview Questions

**Core:**
- Why separate epoll thread from worker threads?
- What's the risk of calling handler directly in epoll loop?
- Why use `eventfd` for wakeup instead of just pipe?
- Explain `std::shared_mutex` — when does it help vs hurt?

**Advanced:**
- How would you implement priority-based event handling?
- Design a timeout mechanism: handler must complete in N ms
- How to handle fd that closes while handler is running?
- Compare this design to `libuv` or `Boost.Asio`

### 10. Optional Extensions

- Add event filtering / routing by event type
- Implement backpressure: slow consumer notifies producer
- Add `io_uring` backend as drop-in epoll replacement
- Plugin-loadable handlers via `dlopen`/`dlsym`

---

## Project D: Plugin-based Middleware

### 1. Problem Scenario

A middleware host that loads driver plugins at runtime via `dlopen`. Each plugin implements a common `IPlugin` interface. Enables adding support for new hardware without recompiling the host.

### 2. Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│                  Plugin Host Process                     │
│                                                         │
│  PluginManager                                          │
│  ┌─────────────────────────────────────────────────┐   │
│  │  dlopen("libtemp_plugin.so")                    │   │
│  │  dlsym("create_plugin") → IPlugin* (factory)   │   │
│  │  plugins_[name] = unique_ptr<IPlugin>(factory())│   │
│  └─────────────────────────────────────────────────┘   │
│       │                                                 │
│       ▼                                                 │
│  IPlugin interface                                      │
│  ┌──────────────────┐  ┌──────────────────┐            │
│  │  TempPlugin.so   │  │  UartPlugin.so   │  ...       │
│  │  + init()        │  │  + init()        │            │
│  │  + read()        │  │  + read()        │            │
│  │  + shutdown()    │  │  + shutdown()    │            │
│  └──────────────────┘  └──────────────────┘            │
└─────────────────────────────────────────────────────────┘
```

### 3. Plugin Interface Design

```cpp
// include/iplugin.hpp — shared between host and plugins
struct PluginData {
    std::string id;
    double value;
    int64_t timestamp_us;
};

class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual bool init(const std::string& config) = 0;
    virtual std::optional<PluginData> read() = 0;
    virtual void shutdown() = 0;
    virtual std::string_view name() const = 0;
};

// Each .so must export this C function (extern "C" for no mangling)
// IPlugin* create_plugin();
// void destroy_plugin(IPlugin*);
```

### 4. CMake for Shared Libraries

```cmake
# Plugin CMakeLists.txt
add_library(temp_plugin SHARED src/temp_plugin.cpp)
target_link_libraries(temp_plugin PRIVATE plugin_iface)

# IMPORTANT: control symbol visibility
set_target_properties(temp_plugin PROPERTIES
    CXX_VISIBILITY_PRESET hidden        # hide all symbols by default
    VISIBILITY_INLINES_HIDDEN YES)

# Only export factory functions
# In source: __attribute__((visibility("default"))) extern "C" IPlugin* create_plugin() {...}
```

### 5. Key Interview Topics

- `dlopen` flags: `RTLD_LAZY` vs `RTLD_NOW`
- Why `extern "C"` for plugin factory functions?
- Symbol visibility: why hide by default?
- ABI stability: what breaks plugin compatibility?
- Memory ownership: who owns the `IPlugin*`?

---

## Interview Deep-dives

### Topic: Memory Model & Ordering

```
Happens-before relationship
────────────────────────────────────────────────
Thread A:                     Thread B:
  data = 42;                    if (flag.load(acquire))
  flag.store(true, release);      use(data);  // safe: sees data=42
                                              // guaranteed by acq/rel
```

**Interview:** Explain why `std::memory_order_relaxed` is insufficient for the producer-consumer flag pattern.

---

### Topic: Linux Resource Limits

```bash
ulimit -n        # max open files (default 1024)
ulimit -Hn       # hard limit

# In daemon: raise for production
struct rlimit rl = {65536, 65536};
setrlimit(RLIMIT_NOFILE, &rl);

# Check at runtime
/proc/sys/fs/file-max          # system-wide
/proc/<pid>/limits             # per-process
```

---

### Topic: mmap for Zero-copy Shared Memory

```
mmap workflow
─────────────────────────────────────────────────────────
Process A (writer)              Process B (reader)
  shm_open("myshm", O_CREAT)     shm_open("myshm", O_RDONLY)
  ftruncate(fd, size)             fstat(fd, &st)
  mmap(NULL, size,                mmap(NULL, st.st_size,
       PROT_READ|PROT_WRITE,           PROT_READ,
       MAP_SHARED, fd, 0)              MAP_SHARED, fd, 0)
       → void* ptr                     → const void* ptr
  memcpy(ptr, data, len)          memcpy(local, ptr, len)
  munmap(ptr, size)               munmap(ptr, size)
  shm_unlink("myshm")

Sync: use semaphore or futex for producer-consumer sync
      (shared memory has no built-in synchronization)
```

---

### Topic: C++ ABI & extern "C"

```
C++ name mangling
─────────────────────────────────────────────────────────
C++ function:  void foo(int)    → mangled: _Z3fooi
C function:    void foo(int)    → unmangled: foo

extern "C" void foo(int);      → disables mangling → foo

Why it matters for .so plugins:
  dlsym("create_plugin")   ← needs exact symbol name
  C++ mangling: _ZN...     ← impossible to predict
  extern "C":  create_plugin ← predictable, stable
```

---

## Tooling Quick-reference

### GDB Essential Commands

```bash
gdb ./sensor_monitor
(gdb) run
(gdb) break reader.cpp:45
(gdb) thread apply all bt   # all thread stack traces
(gdb) info threads
(gdb) watch variable_name   # hardware watchpoint
(gdb) set follow-fork-mode child  # debug child process
```

### Valgrind

```bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         ./sensor_monitor

valgrind --tool=helgrind ./sensor_monitor   # race/deadlock
valgrind --tool=callgrind ./sensor_monitor  # profiling
```

### Sanitizers (build-time, faster than valgrind)

```bash
# CMake: add to compile/link options
-fsanitize=address          # heap/stack/global overflow, use-after-free
-fsanitize=undefined        # UB: null deref, signed overflow, etc
-fsanitize=thread           # data races
# Note: address + thread cannot be combined

# Run: sanitizer output printed to stderr automatically
```

### strace / ltrace

```bash
strace -p <pid>                      # trace running process
strace -e trace=read,write ./app     # filter syscalls
strace -c ./app                      # summary: count, time
ltrace ./app                         # library calls

# Useful for:
# - Verifying epoll_wait is called
# - Checking fd open/close balance
# - Seeing signal delivery
```

### perf

```bash
perf stat ./app              # CPU counter summary
perf top -p <pid>            # live function profiling
perf record ./app            # record samples
perf report                  # interactive report
```

### /proc Inspection

```bash
/proc/<pid>/fd/          # open file descriptors
/proc/<pid>/maps         # memory mappings
/proc/<pid>/status       # memory, threads
/proc/<pid>/limits       # resource limits
/proc/<pid>/threads/     # per-thread info
```

---

## Daily Review Checklist

### Every Day

- [ ] Read and understood today's topic without notes
- [ ] Can explain the key concept verbally in 2 minutes
- [ ] Wrote at least 20 lines of production-style code
- [ ] Ran sanitizers (ASan/TSan) on written code
- [ ] Reviewed 3 interview questions from today's topic

### Every Project

- [ ] Clear ownership model (who owns what resource)
- [ ] All fds/sockets/memory in RAII wrappers
- [ ] No raw `new`/`delete` outside RAII classes
- [ ] ThreadSanitizer: zero races
- [ ] AddressSanitizer: zero errors
- [ ] Graceful shutdown: all resources freed
- [ ] CMake: builds cleanly with `-Wall -Wextra`
- [ ] Can explain architecture in ASCII diagram form
- [ ] Can answer: "what breaks under resource pressure?"
- [ ] Can answer: "what are the concurrency risks?"

### Interview Day Readiness

- [ ] RAII: write FileDescriptor from memory
- [ ] Queue: write BoundedQueue with condition_variable from memory
- [ ] epoll: explain edge vs level triggered, write event loop skeleton
- [ ] IPC: compare Unix socket vs pipe vs shared memory
- [ ] Concurrency: explain happens-before, memory ordering
- [ ] Debugging: describe workflow for deadlock, fd leak, race condition
- [ ] CMake: write CMakeLists.txt for shared library with interface includes
- [ ] Signal handling: explain signalfd pattern + why not signal()
- [ ] UART: explain termios raw mode configuration
- [ ] Plugin: explain dlopen/dlsym + extern "C" + symbol visibility

---

*Generated for interview preparation — Middle/Senior Embedded Linux Engineer*
*Focus: production mindset, not tutorial code*