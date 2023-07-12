#include <cstring>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include "../src/NBSimdBooster.h"

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


void compare(){
    std::vector<std::string> needles;
    std::vector<std::string> haystacks;
    int numStrings = 1;
    int stringLength_needle = 1;
    int stringLength_haystack = 128;
    // int stringLength_suiji = 1024;
    for (int i = 0; i < numStrings; ++i) {
        std::string needle = generateRandomString(stringLength_needle);
        std::string haystack = generateRandomString(stringLength_haystack)+ needle;
        needles.push_back(needle);
        haystacks.push_back(haystack);
    }

    double total_elapsed_time = 0.0;
    for (int i = 0; i < numStrings; ++i) {
        const char* needle = needles[i].c_str();
        const char* haystack = haystacks[i].c_str();
        NBSimdBooster::StringSearcher<true, true> searcher(needle, strlen(needle)+1);

        auto start = std::chrono::high_resolution_clock::now();
        bool result = searcher.compare(haystack, haystack + strlen(haystack),haystack);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        double elapsed_time = elapsed_seconds.count();
        total_elapsed_time += elapsed_time;
        // if(result) std::cout << "true" << std::endl;
        // else std::cout << "false" << std::endl;

    }
    std::cout <<  "search_Elapsed time: " << total_elapsed_time << " s\n";
}

void Case_sensitive_ASCII_UTF8_search(){
    std::vector<std::string> needles;
    std::vector<std::string> haystacks;
    int numStrings = 100;
    int stringLength_needle = 256;
    int stringLength_haystack = 256;
    // int stringLength_suiji = 1024;
    for (int i = 0; i < numStrings; ++i) {
        std::string suiji = generateRandomString(stringLength_needle);
        std::string needle = generateRandomString(stringLength_needle);
        std::string haystack = generateRandomString(stringLength_haystack)+ needle;
        needles.push_back(needle);
        haystacks.push_back(haystack);
    }

    double total_elapsed_time = 0.0;
    for (int i = 0; i < numStrings; ++i) {
        const char* needle = needles[i].c_str();
        const char* haystack = haystacks[i].c_str();
        NBSimdBooster::StringSearcher<true, true> searcher(needle, strlen(needle));

        auto start = std::chrono::high_resolution_clock::now();
        const char* result = searcher.search(haystack, haystack + strlen(haystack));
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        double elapsed_time = elapsed_seconds.count();
        total_elapsed_time += elapsed_time;
    }
    
    std::cout <<  "search_Elapsed time: " << total_elapsed_time << " s\n";
}

void Case_insensitive_ASCII__UTF8_search(){
    std::vector<std::string> needles;
    std::vector<std::string> haystacks;
    int numStrings = 100000;
    int stringLength_needle = 256;
    int stringLength_haystack = 1024;
    // int stringLength_suiji = 1024;
    for (int i = 0; i < numStrings; ++i) {
        std::string suiji = generateRandomString(stringLength_needle);
        std::string needle = generateRandomString(stringLength_needle);
        std::string haystack = generateRandomString(stringLength_haystack)+ needle;
        needles.push_back(needle);
        haystacks.push_back(haystack);
    }

    double total_elapsed_time = 0.0;
    for (int i = 0; i < numStrings; ++i) {
        const char* needle = needles[i].c_str();
        const char* haystack = haystacks[i].c_str();
        NBSimdBooster::ASCIICaseSensitiveStringSearcher searcher(needle, strlen(needle));

        auto start = std::chrono::high_resolution_clock::now();
        const char* result = searcher.search(haystack, haystack + strlen(haystack));
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        double elapsed_time = elapsed_seconds.count();
        total_elapsed_time += elapsed_time;
    }
    
    std::cout <<  "search_Elapsed time: " << total_elapsed_time << " s\n";
}

void Token_search(){
}


int main() {

    // compare();
    // Case_sensitive_ASCII_UTF8_search();
    Case_insensitive_ASCII__UTF8_search();
    // Token_search();
    return 0;
}