#include <iostream>
#include <vector>
#include <sys/mman.h>
#include <cstring>

#include "int.h"

using namespace std;

const int PAGE_SIZE = 4096;
int Int::constructed = 0;
int Int::moves = 0;
int Int::copies = 0;

template<typename T>
class MyVector {
public:
    explicit MyVector(size_t max_capacity = 32768) {
        char* initial_address = (char* ) mmap(nullptr, (max_capacity * sizeof(T) / PAGE_SIZE + 1) * PAGE_SIZE, PROT,
                                             DEFAULT_FLAGS, -1, 0);
        if (initial_address == MAP_FAILED) {
            throw runtime_error(string("MAP_FAILED1 ") + strerror(errno));
        }
        munmap(initial_address, (max_capacity * sizeof(T) / PAGE_SIZE + 1) * PAGE_SIZE);
        raw_data = (char* ) mmap(initial_address, PAGE_SIZE, PROT, DEFAULT_FLAGS | MAP_FIXED_NOREPLACE, -1, 0);
        if (raw_data == MAP_FAILED) {
            throw runtime_error(string("MAP_FAILED2 ") + strerror(errno));
        }
        pages_allocated = 1;
        size = 0;
    }

    ~MyVector() {
        clear();
    }

    MyVector(const MyVector&) = delete;

    MyVector(MyVector&& other)  noexcept {
        *this = other;
    }

    MyVector& operator = (const MyVector&) = delete;

    MyVector& operator = (MyVector&& other)  noexcept {
        clear();
        raw_data = other.raw_data;
        size = other.size;
        pages_allocated = other.pages_allocated;
        other.raw_data = nullptr;
        other.size = 0;
        other.pages_allocated = 0;
        return *this;
    }

    T& operator [] (size_t ind) {
        T* data = (T*) raw_data;
        return data[ind];
    }

    const T& operator [] (size_t ind) const {
        T* data = (T*) raw_data;
        return data[ind];
    }

    void pop() {
        T* data = (T*) raw_data;
        data[size - 1].~T();
        size--;
    }

    void push_back(const T& element) {
        if (need_increasing()) {
            increase_capacity();
        }
        T* data = (T*) raw_data;
        new (&data[size]) T(element);
        size++;
    }

    template<typename ...Args>
    void emplace_back(Args ...args) {
        if (need_increasing()) {
            increase_capacity();
        }
        T* data = (T*) raw_data;
        new (&data[size]) T(args...);
        size++;
    }

private:
    static const auto PROT = PROT_WRITE | PROT_READ;
    static const auto DEFAULT_FLAGS = MAP_PRIVATE | MAP_ANONYMOUS;

    void clear() {
        T* data = (T*) raw_data;
        for (size_t i = 0; i < size; i++) {
            data[i].~T();
        }
        if (raw_data) {
            munmap(raw_data, pages_allocated * PAGE_SIZE);
        }
        pages_allocated = 0;
        size = 0;
        raw_data = nullptr;
    }

    bool need_increasing() {
        return (size + 1) * sizeof(T) > pages_allocated * PAGE_SIZE;
    }

    void increase_capacity() {
        char* new_mem = (char* ) mmap(raw_data + pages_allocated * PAGE_SIZE, pages_allocated * PAGE_SIZE, PROT,
                             DEFAULT_FLAGS | MAP_FIXED_NOREPLACE, -1, 0);
        if (new_mem == MAP_FAILED) {
            fallback_allocate();
            return;
        }
        pages_allocated *= 2;
    }

    void fallback_allocate() {
        //cerr << "fallback_allocate called " << pages_allocated << endl;
        T* data = (T*) raw_data;
        T* new_memory = (T *) mmap(nullptr, pages_allocated * 2 * PAGE_SIZE, PROT, DEFAULT_FLAGS, -1, 0);
        if (new_memory == MAP_FAILED) {
            throw runtime_error(string("MAP_FAILED in fallback_allocate ") + strerror(errno));
        }
        for (size_t i = 0; i < size; i++) {
            new (&new_memory[i]) T(std::move(data[i]));
            data[i].~T();
        }
        munmap(raw_data, pages_allocated * PAGE_SIZE);
        pages_allocated *= 2;
        raw_data = (char *) new_memory;
    }

    char* raw_data;
    size_t pages_allocated;
    size_t size;
};

int main() {
    {
        int initial_size = 100000000;
        // auto my_vec = MyVector<Int>(PAGE_SIZE / sizeof(Int));
        auto my_vec = MyVector<Int>(initial_size * 2);
        //auto my_vec = vector<Int>();
        //my_vec.reserve(PAGE_SIZE / sizeof(Int));
        // my_vec.reserve(initial_size);
        for (size_t i = 0; i < initial_size; i++) {
            my_vec.emplace_back(i);
        }
    }
    printf("copied: %d, moved: %d, left: %d\n", Int::copies, Int::moves, Int::constructed);
}