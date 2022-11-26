//
// Created by dima on 26.11.22.
//
#include <sys/mman.h>
#include <vector>
#include <iostream>

#include "my_vector.h"

using namespace std;

namespace {
    int moves = 0;
    int copies = 0;

    class Int {
    private:
        int value;

    public:
        Int() = delete;

        explicit Int(int a): value(a) {}

        Int(const Int& other) {
            value = other.value;
            copies++;
        }

        Int(Int&& other) noexcept {
            value = other.value;
            other.value = 0;
            moves++;
        }

        Int& operator = (const Int& other) = delete;

        Int& operator = (Int&& other) = delete;
    };
}

int old_main() {
    moves = 0;
    copies = 0;
    vector<Int> vec;
    for (int i = 0; i < (1 << 20); i++) {
        vec.emplace_back(i + 1);
    }
    cout << "copies " << copies << " moves " << moves << "\n";
    return 0;
}

const int PAGE_SIZE = 4096;

int experiment() {
    int initial_pages = 32768;
    char* start_addr = (char* ) mmap(nullptr, PAGE_SIZE * initial_pages, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (start_addr == MAP_FAILED) {
        cerr << "initial mmap failed\n";
        return 0;
    }
    munmap(start_addr, PAGE_SIZE * initial_pages);
    char* addr = (char* )mmap(start_addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    int total_alloc = 1;
    auto next_addr = addr + PAGE_SIZE * total_alloc;
    for (int i = 0; i < 15; i++) {
        char* new_addr = (char *)mmap(next_addr, PAGE_SIZE * total_alloc, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED_NOREPLACE, -1, 0);
        if (new_addr == MAP_FAILED) {
            cerr << "MAP_FAILED\n" << total_alloc;
            return 0;
        }
        next_addr = new_addr + PAGE_SIZE * total_alloc;
        total_alloc *= 2;
    }
    cerr << "returning 0" << endl;
    cout << "total_alloc: " << total_alloc << endl;
    return 0;
}

void nop() {
    MyVector<Int> vec;
}