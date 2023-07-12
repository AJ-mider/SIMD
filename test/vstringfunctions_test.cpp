#include <cstring>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include "../src/NBSimdBooster.h"
#include "../src/udf.h"
using namespace NBSimdBooster;

std::string generateRandomString_trim(int length,int maxSpaces) {
    static const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, characters.length() - 1);
    std::uniform_int_distribution<> spacesDis(16, 33);

    std::string result;
    int numSpaces = spacesDis(gen);
    for (int i = 0; i < maxSpaces; ++i) {
        result += ' ';
    }
    for (int i = 0; i < length; ++i) {
        result += characters[dis(gen)];
    }
    return result;
    for (int i = 0; i < maxSpaces; ++i) {
        result += ' ';
    }

}

std::string generateRandomString_transfer(int length) {
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

void runTests_trim() {
    std::vector<std::string> needles;
    int numStrings = 3000000;
    int stringLength = 200;
    int maxSpaces = 128;

    for (int i = 0; i < numStrings; ++i) {
        std::string needle = generateRandomString_trim(stringLength,maxSpaces);
        needles.push_back(needle);
    }

    double total_elapsed_time = 0.0;
    for (int i = 0; i < numStrings; ++i) {
        doris_udf::StringVal input_str(needles[i].c_str(), needles[i].size());
        auto start = std::chrono::high_resolution_clock::now();
        doris_udf::StringVal result_str = VStringFunctions::trim(input_str);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        double elapsed_time = elapsed_seconds.count();
        total_elapsed_time += elapsed_time;
    }
    std::cout <<  "Trim_Elapsed time: " << total_elapsed_time << " s\n";

}

void runTests_tolower() {
    std::vector<std::string> needles;
    int numStrings = 3000000;
    int stringLength = 1;

    for (int i = 0; i < numStrings; ++i) {
        std::string needle = generateRandomString_transfer(stringLength);
        needles.push_back(needle);
    }

    double total_elapsed_time = 0.0;
    for (int i = 0; i < numStrings; ++i) {
        std::string un_transfer = needles[i];
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<char> transfered(needles[i].size());
        NBSimdBooster::VStringFunctions::to_lower(reinterpret_cast<const uint8_t*>(un_transfer.c_str()),
                                                        reinterpret_cast<const uint8_t*>(un_transfer.c_str() + un_transfer.size()),
                                                        reinterpret_cast<uint8_t*>(transfered.data()));

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        double elapsed_time = elapsed_seconds.count();
        total_elapsed_time += elapsed_time;

        // std::cout << "nums: " << i << "\n";
        // std::cout << "un_transfer:" <<un_transfer<< "\n";
        // std::string transferedString(transfered.begin(), transfered.end());
        // std::cout << "transfered: " << transferedString << "\n";
    }
    std::cout <<  "Trim_Elapsed time: " << total_elapsed_time << " s\n";

}

void runTests_toupper() {
    std::vector<std::string> needles;
    int numStrings = 3000000;
    int stringLength = 512;

    for (int i = 0; i < numStrings; ++i) {
        std::string needle = generateRandomString_transfer(stringLength);
        needles.push_back(needle);
    }

    double total_elapsed_time = 0.0;
    for (int i = 0; i < numStrings; ++i) {
        std::string un_transfer = needles[i];
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<char> transfered(needles[i].size());
        NBSimdBooster::VStringFunctions::to_upper(reinterpret_cast<const uint8_t*>(un_transfer.c_str()),
                                                  static_cast<int64_t>(un_transfer.size()),
                                                  reinterpret_cast<uint8_t*>(transfered.data()));

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        double elapsed_time = elapsed_seconds.count();
        total_elapsed_time += elapsed_time;

        // std::cout << "nums: " << i << "\n";
        // std::cout << "un_transfer:" <<un_transfer<< "\n";
        // std::string transferedString(transfered.begin(), transfered.end());
        // std::cout << "transfered: " << transferedString << "\n";
    }
    std::cout <<  "Trim_Elapsed time: " << total_elapsed_time << " s\n";
    
}

void runTests_repeat() {

    std::vector<std::string> needles;
    int numStrings = 3000000;
    int stringLength =  512;
    doris_udf::IntVal n=5;
    for (int i = 0; i < numStrings; ++i) {
        std::string needle = generateRandomString_transfer(stringLength);
        needles.push_back(needle);
    }   
    NBSimdBooster::VStringFunctions vStringFunctions;
    double total_elapsed_time = 0.0;
    for (int i = 0; i < numStrings; ++i) {
        doris_udf::StringVal un_repeat(needles[i].c_str(), needles[i].size());
        std::vector<char> repeated(needles[i].size()*n.val+1);
        auto start = std::chrono::high_resolution_clock::now();
        vStringFunctions.repeat(reinterpret_cast<uint8_t*>(repeated.data()),un_repeat,n);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        double elapsed_time = elapsed_seconds.count();
        total_elapsed_time += elapsed_time;

        // std::cout << "un_repeat:" <<un_repeat<< "\n";
        // std::string transferedString(repeated.begin(), repeated.end());
        // std::cout << "repeated: " << transferedString << "\n";

    }
    std::cout <<  "To_repeat_Elapsed time: " << total_elapsed_time << " s\n";
    
}


int main() {

    /** trim的测试,改回了sse2，但是用sse4或sse2编译的时候不知道为什么性能不稳定，这里低长度sse4.1好，高长度sse2好
        g++ -O0 vstringfunctions_test.cpp -o pj1 -msse4.1
        g++ -O0 vstringfunctions_test.cpp -o pj2
        
    */
    // runTests_trim();


    /** tolwer转小写测试,改回了sse2，但是用sse4或sse2编译的时候不知道为什么性能不稳定，这里sse2好
        g++ -O0 vstringfunctions_test.cpp -o pj1 -msse4.1
        g++ -O0 vstringfunctions_test.cpp -o pj2
        256个字符，优化前:4.30307 优化后：1.03169s
        32个空格，优化前:1.971 优化后：0.853402s
    */
    // runTests_tolower();


    /** tolwer转d大写测试,改回了sse2，但是用sse4或sse2编译的时候不知道为什么性能不稳定，这里sse2好
        g++ -O0 vstringfunctions_test.cpp -o pj1 -msse4.1
        g++ -O0 vstringfunctions_test.cpp -o pj2
        256个字符，优化前:4.30307 优化后：1.03169s
        32个空格，优化前:0.969091 优化后：0.559087s
    */
    // runTests_toupper();


    /** Repeat测试,有bug，已解决，但感觉不完善,具体是结尾字符的处理
        g++ -O0 vstringfunctions_test.cpp -o pj1  sse_memcpy()
        g++ -O0 vstringfunctions_test.cpp -o pj1  std::memcpy()
        1024个字符，优化前:1.093e-06 s 优化后：1.03169e-06 s
        内置函数memcpys在使用的时候就会默认进行优化，而自定义函数如果不启用优化，则性能会特别的差，启用后和内置的差不多
    */    
    runTests_repeat();



    return 0;

}

//g++ vstringfunctions_test.cpp -o ob1 -msse4.1