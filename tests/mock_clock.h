#pragma once

#include <chrono>
#include <memory>

class MockClock {
 public:
  using time_point = std::chrono::steady_clock::time_point;
  using duration = std::chrono::steady_clock::duration;

  MockClock()
      : time_ptr_(
            std::make_shared<time_point>(std::chrono::steady_clock::now())) {}

  static time_point now() { return *instance_time_ptr_; }

  void advance(std::chrono::seconds seconds) {
    *time_ptr_ += seconds;
    *instance_time_ptr_ = *time_ptr_;
  }

  void set_time(time_point time) {
    *time_ptr_ = time;
    *instance_time_ptr_ = *time_ptr_;
  }

  void setup_static() { instance_time_ptr_ = time_ptr_; }

 private:
  std::shared_ptr<time_point> time_ptr_;
  static std::shared_ptr<time_point> instance_time_ptr_;
};
