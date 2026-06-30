#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>

namespace csk {

// Lightweight span replacement for GCC 9 (no <span> support)
template <typename T>
class Span {
public:
    Span() : data_(nullptr), size_(0) {}
    Span(T *data, size_t size) : data_(data), size_(size) {}
    Span(std::vector<typename std::remove_const<T>::type> &vec)
        : data_(vec.data()), size_(vec.size()) {}
    Span(const std::vector<typename std::remove_const<T>::type> &vec)
        : data_(vec.data()), size_(vec.size()) {}
    template <size_t N>
    Span(T (&arr)[N]) : data_(arr), size_(N) {}

    // Allow implicit conversion from Span<U> to Span<const U>
    template <typename U,
              typename = typename std::enable_if<
                  std::is_same<T, const U>::value>::type>
    Span(Span<U> other) : data_(other.data()), size_(other.size()) {}

    T *data() const { return data_; }
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    T &operator[](size_t i) const { return data_[i]; }

    T *begin() const { return data_; }
    T *end() const { return data_ + size_; }

    Span subspan(size_t offset, size_t count = static_cast<size_t>(-1)) const {
        if (count == static_cast<size_t>(-1)) count = size_ - offset;
        return Span(data_ + offset, count);
    }

private:
    T *data_;
    size_t size_;
};

using SpanU8 = Span<const uint8_t>;
using MutSpanU8 = Span<uint8_t>;

class Buffer {
public:
    Buffer() = default;
    explicit Buffer(size_t size) : data_(size) {}
    Buffer(const uint8_t *data, size_t size) : data_(data, data + size) {}
    explicit Buffer(std::vector<uint8_t> data) : data_(std::move(data)) {}
    Buffer(SpanU8 span) : data_(span.begin(), span.end()) {}

    uint8_t *data() { return data_.data(); }
    const uint8_t *data() const { return data_.data(); }
    size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }

    MutSpanU8 span() { return {data_.data(), data_.size()}; }
    SpanU8 span() const { return {data_.data(), data_.size()}; }

    void resize(size_t size) { data_.resize(size); }
    void reserve(size_t size) { data_.reserve(size); }
    void clear() { data_.clear(); }

    void append(const uint8_t *data, size_t size) {
        data_.insert(data_.end(), data, data + size);
    }

    void append(SpanU8 span) {
        data_.insert(data_.end(), span.begin(), span.end());
    }

    uint8_t &operator[](size_t index) { return data_[index]; }
    const uint8_t &operator[](size_t index) const { return data_[index]; }

private:
    std::vector<uint8_t> data_;
};

using BufferPtr = std::shared_ptr<Buffer>;

inline BufferPtr make_buffer(size_t size) {
    return std::make_shared<Buffer>(size);
}

inline BufferPtr make_buffer(const uint8_t *data, size_t size) {
    return std::make_shared<Buffer>(data, size);
}

inline BufferPtr make_buffer(std::vector<uint8_t> data) {
    return std::make_shared<Buffer>(std::move(data));
}

class BufferReader {
public:
    explicit BufferReader(SpanU8 data) : data_(data) {}

    uint8_t read_u8() {
        check_remaining(1);
        return data_[pos_++];
    }

    uint16_t read_u16_be() {
        check_remaining(2);
        uint16_t val = (static_cast<uint16_t>(data_[pos_]) << 8) |
                       static_cast<uint16_t>(data_[pos_ + 1]);
        pos_ += 2;
        return val;
    }

    uint32_t read_u32_be() {
        check_remaining(4);
        uint32_t val = (static_cast<uint32_t>(data_[pos_]) << 24) |
                       (static_cast<uint32_t>(data_[pos_ + 1]) << 16) |
                       (static_cast<uint32_t>(data_[pos_ + 2]) << 8) |
                       static_cast<uint32_t>(data_[pos_ + 3]);
        pos_ += 4;
        return val;
    }

    SpanU8 read_bytes(size_t n) {
        check_remaining(n);
        auto result = data_.subspan(pos_, n);
        pos_ += n;
        return result;
    }

    void skip(size_t n) {
        check_remaining(n);
        pos_ += n;
    }

    size_t remaining() const { return data_.size() - pos_; }
    size_t position() const { return pos_; }
    bool has_remaining() const { return pos_ < data_.size(); }

private:
    void check_remaining(size_t n) const {
        if (pos_ + n > data_.size()) {
            throw std::out_of_range("BufferReader: not enough data");
        }
    }

    SpanU8 data_;
    size_t pos_ = 0;
};

class BufferWriter {
public:
    BufferWriter() = default;
    explicit BufferWriter(size_t reserve_size) { data_.reserve(reserve_size); }

    void write_u8(uint8_t val) { data_.push_back(val); }

    void write_u16_be(uint16_t val) {
        data_.push_back(static_cast<uint8_t>(val >> 8));
        data_.push_back(static_cast<uint8_t>(val & 0xFF));
    }

    void write_u32_be(uint32_t val) {
        data_.push_back(static_cast<uint8_t>(val >> 24));
        data_.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
        data_.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
        data_.push_back(static_cast<uint8_t>(val & 0xFF));
    }

    void write_bytes(SpanU8 bytes) {
        data_.insert(data_.end(), bytes.begin(), bytes.end());
    }

    void write_bytes(const uint8_t *data, size_t size) {
        data_.insert(data_.end(), data, data + size);
    }

    std::vector<uint8_t> take() { return std::move(data_); }
    const std::vector<uint8_t> &data() const { return data_; }
    size_t size() const { return data_.size(); }

private:
    std::vector<uint8_t> data_;
};

}  // namespace csk
