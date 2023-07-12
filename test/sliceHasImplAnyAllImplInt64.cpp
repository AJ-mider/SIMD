#include <cstring>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include "../src/NBSimdBooster.h"
#include "../src/udf.h"
#include "../src/types.h"
#include <random>
#include <cstddef> 

//SSE4.2 Int64, UInt64 specialization
void sliceHasImplAnyAllImplInt64(){

    NBSimdBooster::NumericArraySlice<Int64> first;
    first.data = new Int64[4]{1, 2, 3, 4};
    first.size = 4;

    NBSimdBooster::NumericArraySlice<Int64> second;
    second.data = new Int64[2]{1,2};
    second.size = 2;

    UInt8* first_null_map = nullptr;
    UInt8* second_null_map = nullptr;

    // Call the function
    bool result = sliceHasImplAnyAllImplInt64(first, second, first_null_map, second_null_map);

    // Output the result
    if (result) {
        std::cout << "All elements in 'second' are found in 'first'." << std::endl;
    } else {
        std::cout << "Not all elements in 'second' are found in 'first'." << std::endl;
    }

    // Clean up the test data
    delete[] first.data;
    delete[] second.data;

}

void sliceHasImplAnyAllImplInt32(){
    NBSimdBooster::NumericArraySlice<Int32> first;
    first.data = new Int32[3]{1, 2, 3};
    first.size = 3;

    NBSimdBooster::NumericArraySlice<Int32> second;
    second.data = new Int32[3]{1, 2, 3};
    second.size = 3;

    UInt8* first_null_map = nullptr;
    UInt8* second_null_map = nullptr;

    // Call the Int64 function
    // bool result1 = sliceHasImplAnyAllImplInt64(first, second, first_null_map, second_null_map);

    // Call the Int32 function
    bool result2 = sliceHasImplAnyAllImplInt32(first, second, first_null_map, second_null_map);

    // Output the results
    // if (result1) {
    //     std::cout << "All elements in 'second' (Int64) are found in 'first' (Int64)." << std::endl;
    // } else {
    //     std::cout << "Not all elements in 'second' (Int64) are found in 'first' (Int64)." << std::endl;
    // }

    if (result2) {
        std::cout << "All elements in 'second' (Int32) are found in 'first' (Int32)." << std::endl;
    } else {
        std::cout << "Not all elements in 'second' (Int32) are found in 'first' (Int32)." << std::endl;
    }

    // Clean up the test data
    delete[] first.data;
    delete[] second.data;


}

void sliceHasImplAnyAllImplInt16(){
    NBSimdBooster::NumericArraySlice<Int16> first;
    first.data = new Int16[3]{1, 2, 3};
    first.size = 3;

    NBSimdBooster::NumericArraySlice<Int16> second;
    second.data = new Int16[3]{1, 2, 3};
    second.size = 3;

    UInt8* first_null_map = nullptr;
    UInt8* second_null_map = nullptr;


    // Call the Int16 function
    bool result3 = sliceHasImplAnyAllImplInt16(first, second, first_null_map, second_null_map);

    if (result3) {
        std::cout << "All elements in 'second' (Int16) are found in 'first' (Int16)." << std::endl;
    } else {
        std::cout << "Not all elements in 'second' (Int16) are found in 'first' (Int16)." << std::endl;
    }

    // Clean up the test data
    delete[] first.data;
    delete[] second.data;
}

void sliceHasImplAnyAllImplInt8(){
    NBSimdBooster::NumericArraySlice<Int8> first;
    first.data = new Int8[3]{1, 2, 3};
    first.size = 3;

    NBSimdBooster::NumericArraySlice<Int8> second;
    second.data = new Int8[3]{1, 2, 3};
    second.size = 3;

    UInt8 first_null_map[] = {0, 0, 0};
    UInt8 second_null_map[] = {0, 0, 0};


    // Call the Int16 function
    bool result3 = sliceHasImplAnyAllImplInt8(first, second, first_null_map, second_null_map);

    if (result3) {
        std::cout << "All elements in 'second' (Int8) are found in 'first' (Int8)." << std::endl;
    } else {
        std::cout << "Not all elements in 'second' (Int8) are found in 'first' (Int8)." << std::endl;
    }

    // Clean up the test data
    delete[] first.data;
    delete[] second.data;
}


int main()
{

    sliceHasImplAnyAllImplInt64();
    // sliceHasImplAnyAllImplInt32();
    // sliceHasImplAnyAllImplInt16();
    // sliceHasImplAnyAllImplInt8();
    return 0;
}