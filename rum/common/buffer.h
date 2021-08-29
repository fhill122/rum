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
    unsigned int size_ = 0;

  public:
    Buffer() = default;

    virtual ~Buffer() = default;;

    explicit Buffer(unsigned int size) {
        init(size);
    }

    Buffer (char *data, unsigned int size): data_(data), size_(size){}

    void init(unsigned int size) {
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
    void fill(void *src, unsigned int src_size, unsigned int offset = 0) {
        assert(src_size + offset <= size_);
        memcpy(getDataP() + offset, src, src_size);
    }

    void fill(void *src) { fill(src, size_); }

    char *getDataP() {
        return data_;
    }

    const char *getDataP() const {
        return data_;
    }

    size_t getSize() const {
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

    explicit BufferScoped(unsigned int size) : Buffer(size) {}

    ~BufferScoped() override {
        clear();
    }
};

}
#endif //RUM_COMMON_BUFFER_H_
