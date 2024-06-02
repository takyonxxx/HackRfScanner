#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <vector>
#include <mutex>
#include <condition_variable>

class CircularBuffer {
public:
    CircularBuffer(size_t size) : buffer_(size), head_(0), tail_(0), full_(false) {}

    void write(const float* data) {
        std::unique_lock<std::mutex> lock(mutex_);
        // Assuming the buffer_ size is equal to the number of samples to be written
        for (size_t i = 0; i < buffer_.size(); ++i) {
            buffer_[head_] = data[i];
            head_ = (head_ + 1) % buffer_.size();
            if (full_) {
                tail_ = (tail_ + 1) % buffer_.size();
            }
            full_ = head_ == tail_;
        }
        data_available_.notify_all();
    }

    void read(std::vector<float>& data) {
        std::unique_lock<std::mutex> lock(mutex_);
        data_available_.wait(lock, [&] {
            return size() >= data.size();
        });

        data.resize(buffer_.size());
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = buffer_[(tail_ + i) % buffer_.size()];
        }

        tail_ = (tail_ + data.size()) % buffer_.size();
        full_ = false;
    }

    size_t size() const {
        std::unique_lock<std::mutex> lock(mutex_);
        if (full_) {
            return buffer_.size();
        }
        if (head_ >= tail_) {
            return head_ - tail_;
        } else {
            return buffer_.size() - tail_ + head_;
        }
    }

    size_t capacity() const {
        return buffer_.size();
    }

public:
    mutable std::mutex mutex_;
    std::condition_variable data_available_;

    std::vector<float> buffer_;
    size_t head_;
    size_t tail_;
    bool full_;
};

#endif // CIRCULAR_BUFFER_H
