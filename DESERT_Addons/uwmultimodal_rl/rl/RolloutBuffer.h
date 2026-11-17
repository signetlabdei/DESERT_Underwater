#pragma once

#include <cstddef>
#include <vector>

#include "rl/Types.h"

namespace rl {

class RolloutBuffer {
public:
    explicit RolloutBuffer(std::size_t capacity)
        : capacity_(capacity) {}

    void push(Transition transition) {
        if (capacity_ > 0 && buffer_.size() == capacity_) {
            buffer_.erase(buffer_.begin());
        }

        buffer_.push_back(std::move(transition));
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return buffer_.size();
    }

    [[nodiscard]] const std::vector<Transition>& data() const noexcept {
        return buffer_;
    }

    void clear() noexcept {
        buffer_.clear();
    }

private:
    std::size_t capacity_;
    std::vector<Transition> buffer_;
};

} // namespace rl
