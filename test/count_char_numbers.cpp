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
#include <cstdio>
int main() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> charDist(97, 122);  // ASCII码范围为65到122（大写字母A到小写字母z之间）
    std::uniform_int_distribution<uint64_t> dis(1, 100);  // 范围为1到100的随机数

    const size_t size = 1000;
    std::vector<uint8_t> a_64(size);
    const uint8_t needle = 'b';

    for (size_t i = 0; i < size; ++i) {
        a_64[i] = static_cast<uint8_t>(charDist(gen));
        // std::cout << a_64[i] << std::endl;
    }
    auto start = std::chrono::high_resolution_clock::now();
    int res = NBSimdBooster::count_char(a_64.data(), size, needle);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = (end - start);
    std::cout <<  "search_Elapsed time: " << elapsed_seconds.count()*100000 << " us\n";



    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<int> charDist(97, 122);  // ASCII码范围为65到122（大写字母A到小写字母z之间）
    // std::uniform_int_distribution<uint64_t> dis(1, 100);  // 范围为1到100的随机数

    // const size_t size = 16;
    // std::vector<int32_t> a_64(size)
    // const int32_t divider = 2;
    // // 生成随机数据填充数组
    // for (size_t i = 0; i < size; ++i) {
    //     // a_64[i] = static_cast<uint8_t>(charDist(gen));
    //     std::cout << a_64[i] << std::endl;
    // }
    // auto start = std::chrono::high_resolution_clock::now();
    // int res = NBSimdBooster::count_numbers(a_64.data(), size, divider);
    // auto end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> elapsed_seconds = (end - start);
    // std::cout <<  "search_Elapsed time: " << elapsed_seconds.count()*100000 << " us\n";
    // std::cout << res << std::endl;

}