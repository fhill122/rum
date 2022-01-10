//
// Created by Ivan B on 2021/4/10.
//

#ifndef RUM_COMMON_BUFFER_H_
#define RUM_COMMON_BUFFER_H_
#include <cassert>
#include <memory>
#include <cstring>

namespace rum {

/**
 * buffer using raw pointer
 */
class Buffer {
  private:
    char *data_ = nullptr;
    size_t size_ = 0;

  public:
    Buffer() = default;

    virtual ~Buffer() = default;;

    explicit Buffer(size_t size) {
        init(size);
    }

    // taking exist data, careful with clear
    Buffer (char *data, size_t size): data_(data), size_(size){}

    void init(size_t size) {
        assert(size == 0);
        size_ = size;
        data_ = new char[size];
    }

    void clear() {
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
    }

    /**
     * fill data_
     * @param src src
     * @param src_size size_ of data_ to be filled
     * @param offset offset start place
     */
    void fill(void *src, size_t src_size, size_t offset = 0) {
        assert(src_size + offset <= size_);
        memcpy(getData() + offset, src, src_size);
    }

    void fill(void *src) { fill(src, size_); }

    [[nodiscard]] char *getData() {
        return data_;
    }

    [[nodiscard]] const char *getData() const {
        return data_;
    }

    [[nodiscard]] size_t getSize() const {
        return size_;
    }

};

/**
 * Scoped buffer that deallocates memory when destroyed
 *
 * Note: be aware of copy, move, assign, ...
 */
class BufferScoped : public Buffer {
  public:
    BufferScoped() = default;

    explicit BufferScoped(size_t size) : Buffer(size) {}

    ~BufferScoped() override {
        clear();
    }
};

}
#endif //RUM_COMMON_BUFFER_H_
