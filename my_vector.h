#include <cstdlib>
#include <sys/mman.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <stdexcept>

using std::string;
using std::runtime_error;

template<typename T>
class MyVector {
public:
    explicit MyVector(size_t max_capacity = 32768);

    ~MyVector();

    MyVector(const MyVector&) = delete;

    MyVector(MyVector&& other) noexcept;

    MyVector& operator = (const MyVector&) = delete;

    MyVector& operator = (MyVector&& other)  noexcept;

    T& operator [] (size_t ind);

    const T& operator [] (size_t ind) const;

    void pop();

    void push_back(const T& element);

    template<typename ...Args>
    void emplace_back(Args ...args);

private:
    static const int PROT = PROT_WRITE | PROT_READ;
    static int DEFAULT_FLAGS;

    static size_t PAGE_SIZE;

    void clear();

    bool need_increasing();

    void increase_capacity();

    void fallback_allocate();

    char* raw_data;
    size_t pages_allocated;
    size_t size;
};

#include "my_vector.tpp"