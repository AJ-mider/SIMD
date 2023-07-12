#include <cstring>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include "../src/NBSimdBooster.h"
#include "../src/udf.h"
#include <random>
#include <cstddef> 
#include <vector>

int main() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(1, 100);  // 范围为1到100的随机数

    const size_t size = 700000;
    std::vector<uint64_t> a_64(size);
    std::vector<uint64_t> c_64(size, 0);
    // 生成随机数据填充数组
    for (size_t i = 0; i < size; ++i) {
        a_64[i] = (dis(gen));
        // std::cout << a_64[i] << " ";
    }
    // std::cout << "Using divideImpl<uint64_t, uint64_t, uint64_t>:" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    NBSimdBooster::divideImpl<uint64_t, uint64_t, uint64_t>(a_64.data(), 1, c_64.data(), size);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = (end - start);
    std::cout <<  "search_Elapsed time: " << elapsed_seconds.count()*100000 << " us\n";
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_64[i] << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Using divideImpl<uint64_t, uint32_t, uint64_t>:" << std::endl;
    // NBSimdBooster::divideImpl<uint64_t, uint32_t, uint64_t>(a_64, 3, c_64, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_64[i] << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Using divideImpl<uint64_t, uint16_t, uint64_t>:" << std::endl;
    // NBSimdBooster::divideImpl<uint64_t, uint16_t, uint64_t>(a_64, 2, c_64, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_64[i] << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Using divideImpl<uint64_t, char, uint64_t>:" << std::endl;
    // NBSimdBooster::divideImpl<uint64_t, char, uint64_t>(a_64, 'A', c_64, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_64[i] << " ";
    // }
    // std::cout << std::endl;


    // uint32_t a_32[size] = {10, 20, 30, 40, 50};
    // uint32_t c_32[size] = {0};
    // std::cout << "Using divideImpl<uint32_t, uint64_t, uint32_t>:" << std::endl;
    // NBSimdBooster::divideImpl<uint32_t, uint64_t, uint32_t>(a_32, 5, c_32, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_32[i] << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Using divideImpl<uint32_t, uint32_t, uint32_t>:" << std::endl;
    // NBSimdBooster::divideImpl<uint32_t, uint32_t, uint32_t>(a_32, 3, c_32, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_32[i] << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Using divideImpl<uint32_t, uint16_t, uint32_t>:" << std::endl;
    // NBSimdBooster::divideImpl<uint32_t, uint16_t, uint32_t>(a_32, 2, c_32, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_32[i] << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Using divideImpl<uint32_t, char, uint32_t>:" << std::endl;
    // NBSimdBooster::divideImpl<uint32_t, char, uint32_t>(a_32, 'A', c_32, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_32[i] << " ";
    // }
    // std::cout << std::endl;


    // int64_t a_int64[size] = {10, 20, 30, 40, 50};
    // int64_t c_int64[size] = {0};

    // std::cout << "Using divideImpl<int64_t, int64_t, int64_t>:" << std::endl;
    // NBSimdBooster::divideImpl<int64_t, int64_t, int64_t>(a_int64, 5, c_int64, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_int64[i] << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Using divideImpl<int64_t, int32_t, int64_t>:" << std::endl;
    // NBSimdBooster::divideImpl<int64_t, int32_t, int64_t>(a_int64, 3, c_int64, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_int64[i] << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Using divideImpl<int64_t, int16_t, int64_t>:" << std::endl;
    // NBSimdBooster::divideImpl<int64_t, int16_t, int64_t>(a_int64, 2, c_int64, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_int64[i] << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Using divideImpl<int64_t, int8_t, int64_t>:" << std::endl;
    // NBSimdBooster::divideImpl<int64_t, int8_t, int64_t>(a_int64, 1, c_int64, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_int64[i] << " ";
    // }
    // std::cout << std::endl;


    // int32_t a_int32[size] = {10, 20, 30, 40, 50};
    // int32_t c_int32[size] = {0};

    // std::cout << "Using divideImpl<int32_t, int64_t, int32_t>:" << std::endl;
    // NBSimdBooster::divideImpl<int32_t, int64_t, int32_t>(a_int32, 5, c_int32, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_int32[i] << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Using divideImpl<int32_t, int32_t, int32_t>:" << std::endl;
    // NBSimdBooster::divideImpl<int32_t, int32_t, int32_t>(a_int32, 3, c_int32, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_int32[i] << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Using divideImpl<int32_t, int16_t, int32_t>:" << std::endl;
    // NBSimdBooster::divideImpl<int32_t, int16_t, int32_t>(a_int32, 2, c_int32, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_int32[i] << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Using divideImpl<int32_t, int8_t, int32_t>:" << std::endl;
    // NBSimdBooster::divideImpl<int32_t, int8_t, int32_t>(a_int32, 1, c_int32, size);
    // for (size_t i = 0; i < size; ++i) {
    //     std::cout << c_int32[i] << " ";
    // }
    // std::cout << std::endl;




    return 0;
}