#include <cstddef>
#include <stdexcept>
#include <vector>

namespace rl {

template <typename T>
class CircularArray {
public:
    explicit CircularArray(std::size_t capacity);
    void push_back(const T& element);
    const T& get_nth_last(std::size_t n) const;
    std::size_t size() const;
    std::size_t capacity() const;

private:
    std::size_t capacity_;
    std::size_t size_;
    std::size_t start_index_;
    std::vector<T> data_;
};

template <typename T>
CircularArray<T>::CircularArray(std::size_t capacity)
    : capacity_(capacity), size_(0), start_index_(0), data_(capacity) {
    if (capacity_ == 0) {
        throw std::invalid_argument("CircularArray capacity must be greater than zero");
    }
}

template <typename T>
void CircularArray<T>::push_back(const T& element) {
    data_[(start_index_ + size_) % capacity_] = element;
    if (size_ < capacity_) {
        ++size_;
    } else {
        start_index_ = (start_index_ + 1) % capacity_;
    }
}

template <typename T>
const T& CircularArray<T>::get_nth_last(std::size_t n) const {
    if (n >= size_) {
        throw std::out_of_range("n is greater than the number of elements in the buffer");
    }

    std::size_t index = (start_index_ + size_ - 1 - n) % capacity_;
    return data_[index];
}

template <typename T>
std::size_t CircularArray<T>::size() const {
    return size_;
}

template <typename T>
std::size_t CircularArray<T>::capacity() const {
    return capacity_;
}

} // namespace rl