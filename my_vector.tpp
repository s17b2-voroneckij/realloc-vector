template<typename T>
size_t MyVector<T>::PAGE_SIZE = 4096;

template<typename T>
int MyVector<T>::DEFAULT_FLAGS = MAP_ANONYMOUS | MAP_PRIVATE;

template<typename T>
MyVector<T>::MyVector(size_t max_capacity) {
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

template<typename T>
MyVector<T>::~MyVector() {
    clear();
}

template<typename T>
MyVector<T>::MyVector(MyVector&& other)  noexcept {
    *this = std::move(other);
}

template<typename T>
MyVector<T>& MyVector<T>::operator = (MyVector<T>&& other)  noexcept {
    clear();
    raw_data = other.raw_data;
    size = other.size;
    pages_allocated = other.pages_allocated;
    other.raw_data = nullptr;
    other.size = 0;
    other.pages_allocated = 0;
    return *this;
}

template<typename T>
T& MyVector<T>::operator [] (size_t ind) {
    T* data = (T*) raw_data;
    return data[ind];
}

template<typename T>
const T& MyVector<T>::operator [] (size_t ind) const {
    T* data = (T*) raw_data;
    return data[ind];
}

template<typename T>
void MyVector<T>::pop() {
    T* data = (T*) raw_data;
    data[size - 1].~T();
    size--;
}

template<typename T>
void MyVector<T>::push_back(const T& element) {
    if (need_increasing()) {
        increase_capacity();
    }
    T* data = (T*) raw_data;
    new (&data[size]) T(element);
    size++;
}

template<typename T>
template<typename ...Args>
void MyVector<T>::emplace_back(Args ...args) {
    if (need_increasing()) {
        increase_capacity();
    }
    T* data = (T*) raw_data;
    new (&data[size]) T(args...);
    size++;
}

template<typename T>
void MyVector<T>::clear() {
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

template<typename T>
bool MyVector<T>::need_increasing() {
    return (size + 1) * sizeof(T) > pages_allocated * PAGE_SIZE;
}

template<typename T>
void MyVector<T>::increase_capacity() {
    char* new_mem = (char* ) mmap(raw_data + pages_allocated * PAGE_SIZE, pages_allocated * PAGE_SIZE, PROT,
                                  DEFAULT_FLAGS | MAP_FIXED_NOREPLACE, -1, 0);
    if (new_mem == MAP_FAILED) {
        fallback_allocate();
        return;
    }
    pages_allocated *= 2;
}

template<typename T>
void MyVector<T>::fallback_allocate() {
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
