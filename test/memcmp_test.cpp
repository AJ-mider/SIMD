#include <cstring>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include "../src/NBSimdBooster.h"
#include "../src/udf.h"
#include <random>
#include <cstddef> 

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

std::string generateRandomString(int length) {
    static const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, characters.length() - 1);

    std::string result;
    for (int i = 0; i < length; ++i) {
        result += characters[dis(gen)];
    }
    return result;
}
int main() {

    std::vector<std::string> firsts;
    std::vector<std::string> seconds;
    // std::vector<std::string, AlignedAllocator<std::string>> firsts;
    // std::vector<std::string, AlignedAllocator<std::string>> seconds;
    int numStrings = 16;
    int stringLength = 16;
    // int stringLength_first =ldd 
    // int stringLength_second = 1;
    for (int i = 0; i < numStrings; ++i) {
        std::string random = generateRandomString(stringLength);
        // std::string first = random+generateRandomString(stringLength);
        // std::string second = random+generateRandomString(stringLength);
        firsts.push_back(random);
        seconds.push_back(random);
    }
    
    double total_elapsed_time = 0.0;
    for (int i = 0; i < numStrings; ++i) {
        const char* first_ptr = firsts[i].c_str();
        const char* second_ptr = seconds[i].c_str();

        size_t f1_len = firsts[i].size();
        size_t f2_len = seconds[i].size();
        auto start = std::chrono::high_resolution_clock::now();
        bool res = NBSimdBooster::memcmp_small_allow_overflow15(first_ptr,f1_len,second_ptr,f2_len);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = (end - start);
        double elapsed_time = elapsed_seconds.count();
        total_elapsed_time += elapsed_time;
        // std::cout << "firsts[i]:" << firsts[i] << std::endl;
        // std::cout << "seconds[i]:" << seconds[i] << std::endl;
        // std::cout << "res:" << res << std::endl;
    }
    
    std::cout <<  "search_Elapsed time: " << total_elapsed_time << " s\n";

    return 0;
}
// 