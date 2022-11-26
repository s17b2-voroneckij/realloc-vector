#include <iostream>
#include <vector>
#include <sys/mman.h>
#include <cstring>

#include "int.h"
#include "my_vector.h"

using namespace std;

const int PAGE_SIZE = 4096;
int Int::constructed = 0;
int Int::moves = 0;
int Int::copies = 0;

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
        my_vec.push_back(Int(1));
        my_vec.pop();
    }
    printf("copied: %d, moved: %d, left: %d\n", Int::copies, Int::moves, Int::constructed);
}