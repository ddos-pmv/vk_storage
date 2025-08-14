#include "mock_clock.h"

std::shared_ptr<MockClock::time_point> MockClock::instance_time_ptr_ =
    std::make_shared<MockClock::time_point>(std::chrono::steady_clock::now());
