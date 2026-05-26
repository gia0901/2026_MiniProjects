#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <iostream>

// File descriptor wrapper
class Fd {
public:
    explicit Fd(int fd) noexcept : fd_(fd) {}
    ~Fd() {
        if (fd_ >= 0) ::close(fd_);
    }

    // non-copyable: giảm tối đa nguy cơ fd bị tác động bởi bên ngoài
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    
    // movable: cho phép chuyển giao ownership
    Fd(Fd&& other) noexcept : fd_(other.fd_)  {
        other.fd_ = -1;
    } 

    Fd& operator=(Fd&& other) noexcept {
        if (this != &other) {
            reset();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    int get() const noexcept    { return fd_; }
    
    bool valid() const noexcept { return fd_ >= 0; }
    
    void reset() noexcept {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }

private:
    int fd_{-1};
};