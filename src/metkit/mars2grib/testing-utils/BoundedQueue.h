/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <utility>

namespace metkit::mars2grib::testing_utils::run_tests {

/// Multi-producer/multi-consumer queue with a fixed capacity.
///
/// `close()` lets consumers drain the remaining items and then receive `std::nullopt`.
/// `cancel()` drops queued items and wakes every blocked producer and consumer.
template <typename T>
class BoundedQueue {
public:

    explicit BoundedQueue(std::size_t capacity) : capacity_{capacity} {}

    bool push(T value) {
        std::unique_lock<std::mutex> lock{mutex_};
        notFull_.wait(lock, [&]() { return queue_.size() < capacity_ || cancelled_; });
        if (cancelled_) {
            return false;
        }
        queue_.push_back(std::move(value));
        lock.unlock();
        notEmpty_.notify_one();
        return true;
    }

    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock{mutex_};
        notEmpty_.wait(lock, [&]() { return !queue_.empty() || closed_ || cancelled_; });
        if (queue_.empty() || cancelled_) {
            return std::nullopt;
        }
        T value = std::move(queue_.front());
        queue_.pop_front();
        lock.unlock();
        notFull_.notify_one();
        return value;
    }

    void close() {
        std::lock_guard<std::mutex> lock{mutex_};
        closed_ = true;
        notEmpty_.notify_all();
    }

    void cancel() {
        std::lock_guard<std::mutex> lock{mutex_};
        cancelled_ = true;
        queue_.clear();
        notEmpty_.notify_all();
        notFull_.notify_all();
    }

private:

    const std::size_t capacity_;
    std::deque<T> queue_;
    std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    bool closed_{false};
    bool cancelled_{false};
};

}  // namespace metkit::mars2grib::testing_utils::run_tests
