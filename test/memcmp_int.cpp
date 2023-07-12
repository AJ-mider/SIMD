#include <cstring>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <cstddef>
#include "../src/NBSimdBooster.h"
#include "../src/udf.h"

template<typename T>
class AlignedAllocator {
public:
    using value_type = T;

    T* allocate(std::size_t n) {
        void* ptr = nullptr;
        if (posix_memalign(&ptr, 16, n * sizeof(T)) != 0) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(ptr);
    }

    void deallocate(T* ptr, std::size_t) {
        free(ptr);
    }
};

std::vector<int8_t> generateRandomInt8Array(int length) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(-128, 127);

    std::vector<int8_t> result(length);
    for (int i = 0; i < length; ++i) {
        result[i] = static_cast<int8_t>(dis(gen));
    }
    return result;
}

int main() {
    std::vector<std::vector<int8_t>> firsts;
    std::vector<std::vector<int8_t>> seconds;
    int numArrays = 1000000;
    int arrayLength = 32;

    for (int i = 0; i < numArrays; ++i) {
        std::vector<int8_t> randomArray = generateRandomInt8Array(arrayLength);
        std::vector<int8_t> first = randomArray;
        std::vector<int8_t> second = randomArray;
        firsts.push_back(first);
        seconds.push_back(second);
    }

    double total_elapsed_time = 0.0;
    for (int i = 0; i < numArrays; ++i) {
        const int8_t* first_ptr = firsts[i].data();
        const int8_t* second_ptr = seconds[i].data();

        size_t f1_len = firsts[i].size();
        size_t f2_len = seconds[i].size();

        auto start = std::chrono::high_resolution_clock::now();
        bool res = NBSimdBooster::memcmp_small_allow_overflow15(first_ptr, f1_len, second_ptr, f2_len);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = (end - start);
        double elapsed_time = elapsed_seconds.count();
        total_elapsed_time += elapsed_time;
    }

    std::cout << "search_Elapsed time: " << total_elapsed_time << " s\n";

    return 0;
}
