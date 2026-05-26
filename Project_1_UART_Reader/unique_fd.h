#pragma once
#include <unistd.h>
#include <fcntl.h>

// RAII wrapper cho file-descriptor
class UniqueFd {
public:
    explicit UniqueFd(int fd = -1) : fd_(fd) {}
    ~UniqueFd() { reset(); }

    // cấm copy => tránh double-close
    UniqueFd(const UniqueFd&) = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;

    // cho phép move => transfer ownership
    UniqueFd(UniqueFd&& other) noexcept
        : fd_(other.fd_) {
        other.fd_ = -1;
    }

    UniqueFd& operator=(UniqueFd&& other) noexcept {
        if (this != &other) {
            reset();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    void reset() noexcept {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }

    int  get() const { return fd_; }
    bool valid() const { return fd_ >= 0; }

private:
    int fd_;
};