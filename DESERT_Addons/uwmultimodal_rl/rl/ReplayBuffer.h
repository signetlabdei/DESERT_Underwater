#pragma once

#include <algorithm>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <vector>

#include "rl/Types.h"

namespace rl {

class ReplayBuffer {
public:
    explicit ReplayBuffer(std::size_t capacity)
        : capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("replay capacity must be greater than zero");
        }
    }

    void push(Transition transition) {
        if (buffer_.size() < capacity_) {
            buffer_.push_back(std::move(transition));
        } else {
            buffer_[head_] = std::move(transition);
        }

        head_ = (head_ + 1) % capacity_;
    }

    std::vector<Transition> sample(std::size_t count, std::mt19937& rng) const {
        if (count > buffer_.size()) {
            throw std::invalid_argument("cannot sample more elements than present in buffer");
        }

        std::vector<std::size_t> indices(buffer_.size());
        for (std::size_t i = 0; i < indices.size(); ++i) {
            indices[i] = i;
        }

        std::shuffle(indices.begin(), indices.end(), rng);

        std::vector<Transition> batch;
        batch.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            batch.push_back(buffer_[indices[i]]);
        }

        return batch;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return buffer_.size();
    }

    void clear() noexcept {
        buffer_.clear();
        head_ = 0;
    }

private:
    std::vector<Transition> buffer_;
    std::size_t capacity_;
    std::size_t head_ = 0;
};

} // namespace rl
