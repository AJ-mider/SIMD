#pragma once
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <stdint.h>
#include <string.h>      
#include <unistd.h>
#include <cstdlib>
#include "udf.h"
#include "types.h"
#define ALWAYS_INLINE __attribute__((__always_inline__))
// #define LIKELY(x)   __builtin_expect(!!(x), 1)
// #define UNLIKELY(x) __builtin_expect(!!(x), 0)

#include <nmmintrin.h> //SSE4.2
#include <smmintrin.h> //SSE4.1
#include <emmintrin.h> //SSE2
#define REG_SIZE 16

#ifdef __SSE4_2__
    #include <nmmintrin.h>
#endif

#ifdef __SSE4_1__
    #include <smmintrin.h>
#endif

#ifdef __SSE2__
    #include <emmintrin.h>
    // #define REG_SIZE 16
#endif

namespace NBSimdBooster{

// x_86 sse_memcpy
static inline void * sse_memcpy(void * __restrict dst_, const void * __restrict src_, size_t size)
{
    char * __restrict dst = reinterpret_cast<char * __restrict>(dst_);
    const char * __restrict src = reinterpret_cast<const char * __restrict>(src_);
    void * ret = dst;
tail:
    if (size <= 16)
    {
        if (size >= 8)
        {
            __builtin_memcpy(dst + size - 8, src + size - 8, 8);
            __builtin_memcpy(dst, src, 8);
        }
        else if (size >= 4)
        {
            __builtin_memcpy(dst + size - 4, src + size - 4, 4);
            __builtin_memcpy(dst, src, 4);
        }
        else if (size >= 2)
        {
            __builtin_memcpy(dst + size - 2, src + size - 2, 2);
            __builtin_memcpy(dst, src, 2);
        }
        else if (size >= 1)
        {
            *dst = *src;
        }
    }
    else
    {
        if (size <= 128)
        {
            _mm_storeu_si128(reinterpret_cast<__m128i *>(dst + size - 16), _mm_loadu_si128(reinterpret_cast<const __m128i *>(src + size - 16)));

            while (size > 16)
            {
                _mm_storeu_si128(reinterpret_cast<__m128i *>(dst), _mm_loadu_si128(reinterpret_cast<const __m128i *>(src)));
                dst += 16;
                src += 16;
                size -= 16;
            }
        }
        else
        {
            size_t padding = (16 - (reinterpret_cast<size_t>(dst) & 15)) & 15;

            if (padding > 0)
            {
                __m128i head = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
                _mm_storeu_si128(reinterpret_cast<__m128i*>(dst), head);
                dst += padding;
                src += padding;
                size -= padding;
            }

            __m128i c0, c1, c2, c3, c4, c5, c6, c7;

            while (size >= 128)
            {
                c0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src) + 0);
                c1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src) + 1);
                c2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src) + 2);
                c3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src) + 3);
                c4 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src) + 4);
                c5 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src) + 5);
                c6 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src) + 6);
                c7 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src) + 7);
                src += 128;
                _mm_store_si128((reinterpret_cast<__m128i*>(dst) + 0), c0);
                _mm_store_si128((reinterpret_cast<__m128i*>(dst) + 1), c1);
                _mm_store_si128((reinterpret_cast<__m128i*>(dst) + 2), c2);
                _mm_store_si128((reinterpret_cast<__m128i*>(dst) + 3), c3);
                _mm_store_si128((reinterpret_cast<__m128i*>(dst) + 4), c4);
                _mm_store_si128((reinterpret_cast<__m128i*>(dst) + 5), c5);
                _mm_store_si128((reinterpret_cast<__m128i*>(dst) + 6), c6);
                _mm_store_si128((reinterpret_cast<__m128i*>(dst) + 7), c7);
                dst += 128;
                size -= 128;
            }
            goto tail;
        }
    }
    return ret;
} // x_86 sse_memcpy
// extern "C" void * sse_memcpy(void * __restrict dst, const void * __restrict src, size_t size)
// {
//     return inline_sse_memcpy(dst, src, size);
// }


// Function for case conversion
template <char not_case_lower_bound, char not_case_upper_bound>
class LowerUpperImpl {
public:
    LowerUpperImpl() = default;

    static void transfer(const uint8_t* src, const uint8_t* src_end, uint8_t* dst) {
        const auto flip_case_mask = 'A' ^ 'a';
#if defined(__SSE2__) || defined(__aarch64__)
        const auto bytes_sse = sizeof(__m128i);
        const auto src_end_sse = src_end - (src_end - src) % bytes_sse;
        const auto v_not_case_lower_bound = _mm_set1_epi8(not_case_lower_bound - 1);
        const auto v_not_case_upper_bound = _mm_set1_epi8(not_case_upper_bound + 1);
        const auto v_flip_case_mask = _mm_set1_epi8(flip_case_mask);

        for (; src < src_end_sse; src += bytes_sse, dst += bytes_sse) {
            const auto chars = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
            const auto is_not_case = _mm_and_si128(_mm_cmpgt_epi8(chars, v_not_case_lower_bound),
                                                   _mm_cmplt_epi8(chars, v_not_case_upper_bound));
            const auto xor_mask = _mm_and_si128(v_flip_case_mask, is_not_case);
            const auto cased_chars = _mm_xor_si128(chars, xor_mask);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(dst), cased_chars);
        }
#endif
        for (; src < src_end; ++src, ++dst)
            if (*src >= not_case_lower_bound && *src <= not_case_upper_bound)
                *dst = *src ^ flip_case_mask;
            else
                *dst = *src;
    }
};// LowerUpperImpl


// Class for string,such as rtrim,ltrim,trim,to_lower,to_upper,initcap,concat,repeat
class VStringFunctions{
public:
#if defined(__SSE2__) || defined(__aarch64__)
    // REGISTER_SIZE = 16
    static constexpr auto REGISTER_SIZE = sizeof(__m128i);
#endif

    // The function removes the space to the right of the string
    static StringVal rtrim(const StringVal& str) {
        if (str.is_null || str.len == 0) {
            return str;
        }
        auto begin = 0;
        auto end = str.len - 1;
#if defined(__SSE2__) || defined(__aarch64__)
        char blank = ' ';
        const auto pattern = _mm_set1_epi8(blank);
        while (end - begin + 1 >= REGISTER_SIZE) {
            const auto v_haystack = _mm_loadu_si128(
                    reinterpret_cast<const __m128i*>(str.ptr + end + 1 - REGISTER_SIZE));
            const auto v_against_pattern = _mm_cmpeq_epi8(v_haystack, pattern);
            const auto mask = _mm_movemask_epi8(v_against_pattern);
            int offset = __builtin_clz(~(mask << REGISTER_SIZE));
            /// means not found
            if (offset == 0) {
                return StringVal(str.ptr + begin, end - begin + 1);
            } else {
                end -= offset;
            }
        }
#endif
        while (end >= begin && str.ptr[end] == ' ') {
            --end;
        }
        if (end < 0) {
            return StringVal("");
        }
        return StringVal(str.ptr + begin, end - begin + 1);
    }

    // The function removes the space to the left of the string
    static StringVal ltrim(const StringVal& str) {
        if (str.is_null || str.len == 0) {
            return str;
        }
        auto begin = 0;
        auto end = str.len - 1;
#if defined(__SSE2__) || defined(__aarch64__)
        char blank = ' ';
        const auto pattern = _mm_set1_epi8(blank);
        while (end - begin + 1 >= REGISTER_SIZE) {
            const auto v_haystack =
                    _mm_loadu_si128(reinterpret_cast<const __m128i*>(str.ptr + begin));
            const auto v_against_pattern = _mm_cmpeq_epi8(v_haystack, pattern);
            const auto mask = _mm_movemask_epi8(v_against_pattern) ^ 0xffff;
            if (mask == 0) {
                begin += REGISTER_SIZE;
            } else {
                const auto offset = __builtin_ctz(mask);
                begin += offset;
                return StringVal(str.ptr + begin, end - begin + 1);
            }
        }
#endif
        while (begin <= end && str.ptr[begin] == ' ') {
            ++begin;
        }
        return StringVal(str.ptr + begin, end - begin + 1);
    }

    // The function removes the space of the string
    static StringVal trim(const StringVal& str) {
        if (str.is_null || str.len == 0) {
            return str;
        }
        return rtrim(ltrim(str));
    }

    // The function implements the conversion string to lowercase
    static void to_lower(const uint8_t* src, const uint8_t* src_end, uint8_t* dst) {
        if (src ==  src_end) {
            return;
        }
        LowerUpperImpl<'A', 'Z'> lowerUpper;
        lowerUpper.transfer(src, src_end, dst);
    }

    // The function implements the conversion string to uppercase
    static void to_upper(const uint8_t* src, int64_t len, uint8_t* dst) {
        if (len <= 0) {
            return;
        }
        LowerUpperImpl<'a', 'z'> lowerUpper;
        lowerUpper.transfer(src, src + len, dst);
    }



    // todo
    static void initcap(FunctionContext* context, const StringVal& str) {
        // if (str.is_null) {
        //     return StringVal::null();
        // }
        // StringVal result(context, str.len);

        // simd::VStringFunctions::to_lower(str.ptr, str.len, result.ptr);

        // bool need_capitalize = true;
        // for (int64_t i = 0; i < str.len; ++i) {
        //     if (!::isalnum(result.ptr[i])) {
        //         need_capitalize = true;
        //     } else if (need_capitalize) {
        //         result.ptr[i] = ::toupper(result.ptr[i]);
        //         need_capitalize = false;
        //     }
        // }

        return;
    }



    // The function implements concatenation of two strings
    void concat(uint8_t* dst,int num_children,const StringVal* strs) {
        if (num_children == 1) {
            return;
        }

        uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);
        for (int32_t i = 0; i < num_children; ++i) {
            memcpy(ptr, strs[i].ptr, strs[i].len);
            ptr += strs[i].len;
        }
    }

    // The function implementation copies the string n times
    void repeat(uint8_t* dst, const StringVal& str, const IntVal& n) {
        if (str.is_null || n.is_null) {
            return ;
        }
        if (str.len == 0 || n.val <= 0) {
            return ;
        }

        uint8_t* ptr = dst;
        for (int64_t i = 0; i < n.val; ++i) {
            // std::cout <<"进入for循环,ptr: " << ptr << std::endl;
            // sse_memcpy(ptr, str.ptr, str.len);
            std::memcpy(ptr, str.ptr, str.len);
            ptr += str.len;
        }
        // 给字符串加上结束符，否则在打印输出等地方会出现错乱,或者在memcpy的时候在长度str.len+1可以解决
        *ptr = '\0';
    }

};// VStringFunctions

class StringSearcherBase
{
public:
    bool force_fallback = false;

#ifdef __SSE2__
protected:
    static constexpr size_t N = sizeof(__m128i);

    bool isPageSafe(const void * const ptr) const
    {
        return ((page_size - 1) & reinterpret_cast<std::uintptr_t>(ptr)) <= page_size - N;
    }

    Int64 getPageSize()
    {
        Int64 page_size = sysconf(_SC_PAGESIZE);
        if (page_size < 0)
            abort();
        return page_size;
    }

private:
    const Int64 page_size = getPageSize();
#endif
}; //StringSearcherBase


/// Performs case-sensitive or case-insensitive search of ASCII or UTF-8 strings
template <bool CaseSensitive, bool ASCII> class StringSearcher;

/// Case-sensitive ASCII and UTF8 searcher
template <bool ASCII>
class StringSearcher<true, ASCII> : public StringSearcherBase
{
private:
    /// string to be searched for
    const uint8_t * const needle;
    const uint8_t * const needle_end;
    /// first character in `needle`
    uint8_t first_needle_character = 0;

#ifdef __SSE4_1__
    /// second character of "needle" (if its length is > 1)
    uint8_t second_needle_character = 0;
    /// first/second needle character broadcasted into a 16 bytes vector
    __m128i first_needle_character_vec;
    __m128i second_needle_character_vec;
    /// vector of first 16 characters of `needle`
    __m128i cache = _mm_setzero_si128();
    uint16_t cachemask = 0;
#endif

public:
    template <typename CharT>
    StringSearcher(const CharT * needle_, size_t needle_size)
        : needle(reinterpret_cast<const uint8_t *>(needle_))
        , needle_end(needle + needle_size)
    {
        if (needle_size == 0)
            return;

        first_needle_character = *needle;

#ifdef __SSE4_1__
        first_needle_character_vec = _mm_set1_epi8(first_needle_character);
        if (needle_size > 1)
        {
            second_needle_character = *(needle + 1);
            second_needle_character_vec = _mm_set1_epi8(second_needle_character);
        }
        const auto * needle_pos = needle;

        for (uint8_t i = 0; i < N; ++i)
        {
            cache = _mm_srli_si128(cache, 1);

            if (needle_pos != needle_end)
            {
                cache = _mm_insert_epi8(cache, *needle_pos, N - 1);
                cachemask |= 1 << i;
                ++needle_pos;
            }
        }
#endif
    }

    template <typename CharT>
    ALWAYS_INLINE bool compare(const CharT * /*haystack*/, const CharT * /*haystack_end*/, const CharT * pos) const
    {
#ifdef __SSE4_1__
        if (isPageSafe(pos))
        {
            const __m128i haystack_characters = _mm_loadu_si128(reinterpret_cast<const __m128i *>(pos));
            const __m128i comparison_result = _mm_cmpeq_epi8(haystack_characters, cache);
            const uint16_t comparison_result_mask = _mm_movemask_epi8(comparison_result);

            if (0xffff == cachemask)
            {
                if (comparison_result_mask == cachemask)
                {
                    pos += N;
                    const auto * needle_pos = needle + N;

                    while (needle_pos < needle_end && *pos == *needle_pos)
                        ++pos, ++needle_pos;

                    if (needle_pos == needle_end)
                        return true;
                }
            }
            else if ((comparison_result_mask & cachemask) == cachemask)
                return true;

            return false;
        }
#endif

        if (*pos == first_needle_character)
        {
            ++pos;
            const auto * needle_pos = needle + 1;

            while (needle_pos < needle_end && *pos == *needle_pos)
                ++pos, ++needle_pos;

            if (needle_pos == needle_end)
                return true;
        }

        return false;
    }


    template <typename CharT>
    const CharT * search(const CharT * haystack, const CharT * const haystack_end) const
    {
        const auto needle_size = needle_end - needle;

        if (needle == needle_end)
            return haystack;

#ifdef __SSE4_1__
        /// Fast path for single-character needles. Compare 16 characters of the haystack against the needle character at once.
        if (needle_size == 1)
        {
            while (haystack < haystack_end)
            {
                if (haystack + N <= haystack_end && isPageSafe(haystack))
                {
                    const __m128i haystack_characters = _mm_loadu_si128(reinterpret_cast<const __m128i *>(haystack));
                    const __m128i comparison_result = _mm_cmpeq_epi8(haystack_characters, first_needle_character_vec);
                    const uint16_t comparison_result_mask = _mm_movemask_epi8(comparison_result);
                    if (comparison_result_mask == 0)
                    {
                        haystack += N;
                        continue;
                    }

                    const int offset = __builtin_clz(comparison_result_mask);
                    haystack += offset;

                    return haystack;
                }

                if (haystack == haystack_end)
                    return haystack_end;

                if (*haystack == first_needle_character)
                    return haystack;

                ++haystack;
            }

            return haystack_end;
        }
#endif

        while (haystack < haystack_end && haystack_end - haystack >= needle_size)
        {
#ifdef __SSE4_1__
            /// Compare the [0:15] bytes from haystack and broadcasted 16 bytes vector from first character of needle.
            /// Compare the [1:16] bytes from haystack and broadcasted 16 bytes vector from second character of needle.
            /// Bit AND the results of above two comparisons and get the mask.
            if ((haystack + 1 + N) <= haystack_end && isPageSafe(haystack + 1))
            {
                const __m128i haystack_characters_from_1st = _mm_loadu_si128(reinterpret_cast<const __m128i *>(haystack));
                const __m128i haystack_characters_from_2nd = _mm_loadu_si128(reinterpret_cast<const __m128i *>(haystack + 1));
                const __m128i comparison_result_1st = _mm_cmpeq_epi8(haystack_characters_from_1st, first_needle_character_vec);
                const __m128i comparison_result_2nd = _mm_cmpeq_epi8(haystack_characters_from_2nd, second_needle_character_vec);
                const __m128i comparison_result_combined = _mm_and_si128(comparison_result_1st, comparison_result_2nd);
                const uint16_t comparison_result_mask = _mm_movemask_epi8(comparison_result_combined);
                /// If the mask = 0, then first two characters [0:1] from needle are not in the [0:17] bytes of haystack.
                if (comparison_result_mask == 0)
                {
                    haystack += N;
                    continue;
                }

                const int offset = __builtin_clz(comparison_result_mask);
                haystack += offset;

                if (haystack + N <= haystack_end && isPageSafe(haystack))
                {
                    /// Already find the haystack position where the [pos:pos + 1] two characters exactly match the first two characters of needle.
                    /// Compare the 16 bytes from needle (cache) and the first 16 bytes from haystack at once if the haystack size >= 16 bytes.
                    const __m128i haystack_characters = _mm_loadu_si128(reinterpret_cast<const __m128i *>(haystack));
                    const __m128i comparison_result_cache = _mm_cmpeq_epi8(haystack_characters, cache);
                    const uint16_t mask_offset = _mm_movemask_epi8(comparison_result_cache);

                    if (0xffff == cachemask)
                    {
                        if (mask_offset == cachemask)
                        {
                            const auto * haystack_pos = haystack + N;
                            const auto * needle_pos = needle + N;

                            while (haystack_pos < haystack_end && needle_pos < needle_end &&
                                   *haystack_pos == *needle_pos)
                                ++haystack_pos, ++needle_pos;

                            if (needle_pos == needle_end)
                                return haystack;
                        }
                    }
                    else if ((mask_offset & cachemask) == cachemask)
                        return haystack;

                    ++haystack;
                    continue;
                }
            }
#endif

            if (haystack == haystack_end)
                return haystack_end;

            if (*haystack == first_needle_character)
            {
                const auto * haystack_pos = haystack + 1;
                const auto * needle_pos = needle + 1;

                while (haystack_pos < haystack_end && needle_pos < needle_end &&
                       *haystack_pos == *needle_pos)
                    ++haystack_pos, ++needle_pos;

                if (needle_pos == needle_end)
                    return haystack;
            }

            ++haystack;
        }

        return haystack_end;
    }

    template <typename CharT>
    const CharT * search(const CharT * haystack, size_t haystack_size) const
    {
        return search(haystack, haystack + haystack_size);
    }

}; //StringSearcher<true, ASCII>

/// Case-insensitive ASCII searcher
template <>
class StringSearcher<false, true> : public StringSearcherBase
{
private:
    /// string to be searched for
    const uint8_t * const needle;
    const uint8_t * const needle_end;
    /// lower and uppercase variants of the first character in `needle`
    uint8_t l = 0;
    uint8_t u = 0;

#ifdef __SSE4_1__
    /// vectors filled with `l` and `u`, for determining leftmost position of the first symbol
    __m128i patl, patu;
    /// lower and uppercase vectors of first 16 characters of `needle`
    __m128i cachel = _mm_setzero_si128(), cacheu = _mm_setzero_si128();
    int cachemask = 0;
#endif

public:
    template <typename CharT>
    StringSearcher(const CharT * needle_, size_t needle_size)
        : needle(reinterpret_cast<const uint8_t *>(needle_))
        , needle_end(needle + needle_size)
    {
        if (needle_size == 0)
            return;

        l = static_cast<uint8_t>(std::tolower(*needle));
        u = static_cast<uint8_t>(std::toupper(*needle));

#ifdef __SSE4_1__
        patl = _mm_set1_epi8(l);
        patu = _mm_set1_epi8(u);

        const auto * needle_pos = needle;

        for (size_t i = 0; i < N; ++i)
        {
            cachel = _mm_srli_si128(cachel, 1);
            cacheu = _mm_srli_si128(cacheu, 1);

            if (needle_pos != needle_end)
            {
                cachel = _mm_insert_epi8(cachel, std::tolower(*needle_pos), N - 1);
                cacheu = _mm_insert_epi8(cacheu, std::toupper(*needle_pos), N - 1);
                cachemask |= 1 << i;
                ++needle_pos;
            }
        }
#endif
    }

    template <typename CharT>
    ALWAYS_INLINE bool compare(const CharT * /*haystack*/, const CharT * /*haystack_end*/, const CharT * pos) const
    {
#ifdef __SSE4_1__
        if (isPageSafe(pos))
        {
            const auto v_haystack = _mm_loadu_si128(reinterpret_cast<const __m128i *>(pos));
            const auto v_against_l = _mm_cmpeq_epi8(v_haystack, cachel);
            const auto v_against_u = _mm_cmpeq_epi8(v_haystack, cacheu);
            const auto v_against_l_or_u = _mm_or_si128(v_against_l, v_against_u);
            const auto mask = _mm_movemask_epi8(v_against_l_or_u);

            if (0xffff == cachemask)
            {
                if (mask == cachemask)
                {
                    pos += N;
                    const auto * needle_pos = needle + N;

                    while (needle_pos < needle_end && std::tolower(*pos) == std::tolower(*needle_pos))
                    {
                        ++pos;
                        ++needle_pos;
                    }

                    if (needle_pos == needle_end)
                        return true;
                }
            }
            else if ((mask & cachemask) == cachemask)
                return true;

            return false;
        }
#endif

        if (*pos == l || *pos == u)
        {
            ++pos;
            const auto * needle_pos = needle + 1;

            while (needle_pos < needle_end && std::tolower(*pos) == std::tolower(*needle_pos))
            {
                ++pos;
                ++needle_pos;
            }

            if (needle_pos == needle_end)
                return true;
        }

        return false;
    }

    template <typename CharT>
    const CharT * search(const CharT * haystack, const CharT * const haystack_end) const
    {
        if (needle == needle_end)
            return haystack;

        while (haystack < haystack_end)
        {
#ifdef __SSE4_1__
            if (haystack + N <= haystack_end && isPageSafe(haystack))
            {
                const auto v_haystack = _mm_loadu_si128(reinterpret_cast<const __m128i *>(haystack));
                const auto v_against_l = _mm_cmpeq_epi8(v_haystack, patl);
                const auto v_against_u = _mm_cmpeq_epi8(v_haystack, patu);
                const auto v_against_l_or_u = _mm_or_si128(v_against_l, v_against_u);

                const auto mask = _mm_movemask_epi8(v_against_l_or_u);

                if (mask == 0)
                {
                    haystack += N;
                    continue;
                }

                const auto offset = __builtin_ctz(mask);
                haystack += offset;

                if (haystack + N <= haystack_end && isPageSafe(haystack))
                {
                    const auto v_haystack_offset = _mm_loadu_si128(reinterpret_cast<const __m128i *>(haystack));
                    const auto v_against_l_offset = _mm_cmpeq_epi8(v_haystack_offset, cachel);
                    const auto v_against_u_offset = _mm_cmpeq_epi8(v_haystack_offset, cacheu);
                    const auto v_against_l_or_u_offset = _mm_or_si128(v_against_l_offset, v_against_u_offset);
                    const auto mask_offset = _mm_movemask_epi8(v_against_l_or_u_offset);

                    if (0xffff == cachemask)
                    {
                        if (mask_offset == cachemask)
                        {
                            const auto * haystack_pos = haystack + N;
                            const auto * needle_pos = needle + N;

                            while (haystack_pos < haystack_end && needle_pos < needle_end &&
                                   std::tolower(*haystack_pos) == std::tolower(*needle_pos))
                            {
                                ++haystack_pos;
                                ++needle_pos;
                            }

                            if (needle_pos == needle_end)
                                return haystack;
                        }
                    }
                    else if ((mask_offset & cachemask) == cachemask)
                        return haystack;

                    ++haystack;
                    continue;
                }
            }
#endif

            if (haystack == haystack_end)
                return haystack_end;

            if (*haystack == l || *haystack == u)
            {
                const auto * haystack_pos = haystack + 1;
                const auto * needle_pos = needle + 1;

                while (haystack_pos < haystack_end && needle_pos < needle_end &&
                       std::tolower(*haystack_pos) == std::tolower(*needle_pos))
                {
                    ++haystack_pos;
                    ++needle_pos;
                }

                if (needle_pos == needle_end)
                    return haystack;
            }

            ++haystack;
        }

        return haystack_end;
    }

    template <typename CharT>
    const CharT * search(const CharT * haystack, size_t haystack_size) const
    {
        return search(haystack, haystack + haystack_size);
    }
}; //StringSearcher<false, true>

// Searches for needle surrounded by token-separators.
// Separators are anything inside ASCII (0-128) and not alphanum.
// Any value outside of basic ASCII (>=128) is considered a non-separator symbol, hence UTF-8 strings
// should work just fine. But any Unicode whitespace is not considered a token separtor.
template <typename StringSearcher>
class TokenSearcher : public StringSearcherBase
{
    StringSearcher searcher;
    size_t needle_size;

public:
    template <typename CharT>
    TokenSearcher(const CharT * needle_, size_t needle_size_)
        : searcher(needle_, needle_size_)
        , needle_size(needle_size_){}

    template <typename CharT>
    ALWAYS_INLINE bool compare(const CharT * haystack, const CharT * haystack_end, const CharT * pos) const
    {
        // use searcher only if pos is in the beginning of token and pos + searcher.needle_size is end of token.
        if (isToken(haystack, haystack_end, pos))
            return searcher.compare(haystack, haystack_end, pos);

        return false;
    }

    template <typename CharT>
    const CharT * search(const CharT * haystack, const CharT * const haystack_end) const
    {
        // use searcher.search(), then verify that returned value is a token
        // if it is not, skip it and re-run

        const auto * pos = haystack;
        while (pos < haystack_end)
        {
            pos = searcher.search(pos, haystack_end);
            if (pos == haystack_end || isToken(haystack, haystack_end, pos))
                return pos;

            // assuming that heendle does not contain any token separators.
            pos += needle_size;
        }
        return haystack_end;
    }

    template <typename CharT>
    const CharT * search(const CharT * haystack, size_t haystack_size) const
    {
        return search(haystack, haystack + haystack_size);
    }

    template <typename CharT>
    ALWAYS_INLINE bool isToken(const CharT * haystack, const CharT * const haystack_end, const CharT* p) const
    {
        return (p == haystack || isTokenSeparator(*(p - 1)))
             && (p + needle_size >= haystack_end || isTokenSeparator(*(p + needle_size)));
    }

    ALWAYS_INLINE static bool isTokenSeparator(const uint8_t c)
    {
        return !((c >= 'a' && c <= 'z') || !static_cast<unsigned char>(c) < 0x80);
    }
}; //TokenSearcher
using ASCIICaseSensitiveStringSearcher =   NBSimdBooster::StringSearcher<true, true>;
using ASCIICaseInsensitiveStringSearcher = NBSimdBooster::StringSearcher<false, true>;
using UTF8CaseSensitiveStringSearcher =    NBSimdBooster::StringSearcher<true, false>;
using ASCIICaseSensitiveTokenSearcher =    NBSimdBooster::TokenSearcher<ASCIICaseSensitiveStringSearcher>;
using ASCIICaseInsensitiveTokenSearcher =  NBSimdBooster::TokenSearcher<ASCIICaseInsensitiveStringSearcher>;



template <typename T>
inline int cmp(T a, T b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}
#ifdef __SSE2__
// Function to compare memory block sizes
template <typename Char>
inline int memcmp_small_allow_overflow15(const Char* a, size_t a_size, const Char* b,size_t b_size) {
    size_t min = std::min(a_size, b_size);
    for (size_t offset = 0; offset < min; offset += 16) {
        uint16_t mask = _mm_movemask_epi8(
                _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a + offset)),
                               _mm_loadu_si128(reinterpret_cast<const __m128i*>(b + offset))));
        mask = ~mask;
        if (mask) {
            offset += __builtin_ctz(mask);
            if (offset >= min) break;
            return cmp(a[offset], b[offset]);
        }
    }
    return cmp(a_size, b_size);
}

// Function to compare memory block sizes
template <typename Char>
inline int memcmp_small_allow_overflow15(const Char* a, const Char* b, size_t size) {
    for (size_t offset = 0; offset < size; offset += 16) {
        uint16_t mask = _mm_movemask_epi8(
                _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a + offset)),
                               _mm_loadu_si128(reinterpret_cast<const __m128i*>(b + offset))));
        mask = ~mask;
        if (mask) {
            offset += __builtin_ctz(mask);

            if (offset >= size) return 0;
            return cmp(a[offset], b[offset]);
        }
    }

    return 0;
}

// Function to compare memory block sizes
template <typename Char>
inline bool memequal_small_allow_overflow15(const Char* a, size_t a_size, const Char* b,
                                            size_t b_size) {
    if (a_size != b_size) return false;
    for (size_t offset = 0; offset < a_size; offset += 16) {
        uint16_t mask = _mm_movemask_epi8(
                _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a + offset)),
                               _mm_loadu_si128(reinterpret_cast<const __m128i*>(b + offset))));
        mask = ~mask;

        if (mask) {
            offset += __builtin_ctz(mask);
            return offset >= a_size;
        }
    }

    return true;
}

// Function to compare memory block sizes
template <typename Char>
inline int memcmp_small_multiple_of16(const Char* a, const Char* b, size_t size) {
    for (size_t offset = 0; offset < size; offset += 16) {
        uint16_t mask = _mm_movemask_epi8(
                _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a + offset)),
                               _mm_loadu_si128(reinterpret_cast<const __m128i*>(b + offset))));
        mask = ~mask;

        if (mask) {
            offset += __builtin_ctz(mask);
            return cmp(a[offset], b[offset]);
        }
    }

    return 0;
}

// Function to compare memory block sizes
template <typename Char>
inline int memcmp16(const Char* a, const Char* b) {
    uint16_t mask =
            _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a)),
                                             _mm_loadu_si128(reinterpret_cast<const __m128i*>(b))));
    mask = ~mask;

    if (mask) {
        auto offset = __builtin_ctz(mask);
        return cmp(a[offset], b[offset]);
    }

    return 0;
}

// Function to compare memory block sizes
inline bool memequal16(const void* a, const void* b) {
    return 0xFFFF ==
           _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a)),
                                            _mm_loadu_si128(reinterpret_cast<const __m128i*>(b))));
}

// Function to compare memory block sizes
inline bool memory_is_zero_small_allow_overflow15(const void* data, size_t size) {
    const __m128i zero16 = _mm_setzero_si128();

    for (size_t offset = 0; offset < size; offset += 16) {
        uint16_t mask = _mm_movemask_epi8(
                _mm_cmpeq_epi8(zero16, _mm_loadu_si128(reinterpret_cast<const __m128i*>(
                                               reinterpret_cast<const char*>(data) + offset))));
        mask = ~mask;

        if (mask) {
            offset += __builtin_ctz(mask);
            return offset >= size;
        }
    }

    return true;
}

#else
// Function to compare memory block sizes
template <typename Char>
inline int memcmp_small_allow_overflow15(const Char* a, size_t a_size, const Char* b,size_t b_size) {
    if (auto res = memcmp(a, b, std::min(a_size, b_size))){
        return res;
    }
    else{
        return cmp(a_size, b_size);
    }
}
// Function to compare memory block sizes
template <typename Char>
inline int memcmp_small_allow_overflow15(const Char* a, const Char* b, size_t size) {
    return memcmp(a, b, size);
}
// Function to compare memory block sizes
template <typename Char>
inline bool memequal_small_allow_overflow15(const Char* a, size_t a_size, const Char* b,
                                            size_t b_size) {
    return a_size == b_size && 0 == memcmp(a, b, a_size);
}
// Function to compare memory block sizes
template <typename Char>
inline int memcmp_small_multiple_of16(const Char* a, const Char* b, size_t size) {
    return memcmp(a, b, size);
}
// Function to compare memory block sizes
template <typename Char>
inline int memcmp16(const Char* a, const Char* b) {
    return memcmp(a, b, 16);
}
// Function to compare memory block sizes
inline bool memequal16(const void* a, const void* b) {
    return 0 == memcmp(a, b, 16);
}
// Function to compare memory block sizes
inline bool memory_is_zero_small_allow_overflow15(const void* data, size_t size) {
    const char* pos = reinterpret_cast<const char*>(data);
    const char* end = pos + size;

    for (; pos < end; ++pos)
        if (*pos) return false;

    return true;
}
#endif  //#if (defined(__SSE2__) || defined(__aarch64__))
// Function to compare memory

//--------------------------------------------INT----------------------------------------------------------------
inline size_t count_zero_num(const int8_t* __restrict data, size_t size) {
    size_t num = 0;
    const int8_t* end = data + size;
#if defined(__SSE2__)
    const __m128i zero16 = _mm_setzero_si128();
    const int8_t* end64 = data + (size / 64 * 64);

    for (; data < end64; data += 64) {
        num += __builtin_popcountll(
                static_cast<uint64_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(
                        _mm_loadu_si128(reinterpret_cast<const __m128i*>(data)), zero16))) 
                         |
                (static_cast<uint64_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(
                         _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 16)), zero16)))
                 << 16u) |
                (static_cast<uint64_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(
                         _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 32)), zero16)))
                 << 32u) |
                (static_cast<uint64_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(
                         _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 48)), zero16)))
                 << 48u));
    }
#endif
    for (; data < end; ++data) {
        num += (*data == 0);
    }
    return num;
}

// myself
inline void filter_uint_8_array(size_t n, const uint8_t* __restrict__ in,uint8_t* __restrict__ out, uint8_t filter) {
    size_t offset = 0;
#if defined(__SSE4_1__)
// std::cout << "111" << std::endl;
    const __m128i filter_vec = _mm_set1_epi8(filter);
    for (; offset + 16 <= n;offset += 16) {
        const auto in_vec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(in + offset));
        const auto mask = _mm_cmpeq_epi8(in_vec, filter_vec);
        __m128i result = _mm_and_si128(mask, in_vec);
        // __m128i result = _mm_and_si128(mask, _mm_set1_epi8(1));
        // const auto out_vec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(out + offset));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(out + offset), result);
    }
#endif //处理剩余的
    while (offset< n) {
        // std::cout << "222" << std::endl;
        out[offset] = (in[offset] == filter) ? in[offset] : 0;  // 根据过滤条件更新输出数组
        offset++;
    }
    
}


// TODO:SIMD implements , here implements only for Ascii and uint8_t means one bytes
template <class T>
inline static size_t find_byte(const std::vector<T>& vec, size_t start, T byte) {
    if (start >= vec.size()) {
        return start;
    }
    const void* p = std::memchr((const void*)(vec.data() + start), byte, vec.size() - start);
    if (p == nullptr) {
        return vec.size();
    }
    return (T*)p - vec.data();
}

// --myself
template <class T>
inline static size_t find_byte_sse(const std::vector<T>& vec, size_t start, T byte) {
    if (start >= vec.size()) {
        return start;
    }
    const T* data = vec.data();
    const size_t size = vec.size();
    __m128i target = _mm_set1_epi8(static_cast<int8_t>(byte));
    for (size_t i = start; i < size; i += 16) {
        __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&data[i]));
        __m128i cmp = _mm_cmpeq_epi8(chunk, target);
        int mask = _mm_movemask_epi8(cmp);
        if (mask != 0) {
            size_t found_index = i + static_cast<size_t>(__builtin_ctz(mask));
            return found_index;
        }
    }
    return size;
}

template <typename T>
inline bool contain_byte(const T* __restrict data, const size_t length, const signed char byte) {
    return nullptr != std::memchr(reinterpret_cast<const void*>(data), byte, length);
}
inline size_t find_one(const std::vector<uint8_t>& vec, size_t start) {
    return find_byte<uint8_t>(vec, start, 1);
}
inline size_t find_zero(const std::vector<uint8_t>& vec, size_t start) {
    return find_byte<uint8_t>(vec, start, 0);
}                 

// todo(zeno) Compile add avx512 parameter, modify it to bytes64_mask_to_bits64_mask
/// Transform 32-byte mask to 32-bit mask
inline uint32_t bytes32_mask_to_bits32_mask(const uint8_t* data) {
#ifdef __SSE2__
    auto zero16 = _mm_setzero_si128();
    uint32_t mask =
            (static_cast<uint32_t>(_mm_movemask_epi8(_mm_cmpgt_epi8(
                    _mm_loadu_si128(reinterpret_cast<const __m128i*>(data)), zero16)))) |
            ((static_cast<uint32_t>(_mm_movemask_epi8(_mm_cmpgt_epi8(
                      _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 16)), zero16)))
              << 16) &
             0xffff0000);
#else
    uint32_t mask = 0;
    for (std::size_t i = 0; i < 32; ++i) {
        mask |= static_cast<uint32_t>(1 == *(data + i)) << i;
    }
#endif
    return mask;
}
inline uint32_t bytes32_mask_to_bits32_mask(const bool* data) {
    return bytes32_mask_to_bits32_mask(reinterpret_cast<const uint8_t*>(data));
}

size_t count_bytes_in_filter(const std::vector<Int8>& filt) {
    size_t count = 0;

    /** NOTE: In theory, `filt` should only contain zeros and ones.
      * But, just in case, here the condition > 0 (to signed bytes) is used.
      * It would be better to use != 0, then this does not allow SSE2.
      */

    const Int8* pos = reinterpret_cast<const Int8*>(filt.data());
    const Int8* end = pos + filt.size();

#if defined(__SSE2__)
    const __m128i zero16 = _mm_setzero_si128();
    const Int8* end64 = pos + filt.size() / 64 * 64;

    for (; pos < end64; pos += 64) {
        count += __builtin_popcountll(
                static_cast<UInt64>(_mm_movemask_epi8(_mm_cmpgt_epi8(
                        _mm_loadu_si128(reinterpret_cast<const __m128i*>(pos)), zero16))) |
                (static_cast<UInt64>(_mm_movemask_epi8(_mm_cmpgt_epi8(
                         _mm_loadu_si128(reinterpret_cast<const __m128i*>(pos + 16)), zero16)))
                 << 16) |
                (static_cast<UInt64>(_mm_movemask_epi8(_mm_cmpgt_epi8(
                         _mm_loadu_si128(reinterpret_cast<const __m128i*>(pos + 32)), zero16)))
                 << 32) |
                (static_cast<UInt64>(_mm_movemask_epi8(_mm_cmpgt_epi8(
                         _mm_loadu_si128(reinterpret_cast<const __m128i*>(pos + 48)), zero16)))
                 << 48));
    }

    /// TODO Add duff device for tail?
#endif
    for (; pos < end; ++pos) {
        count += *pos > 0;
    }

    return count;
}

template <typename A, typename B, typename ResultType>
void divideImpl(const A * __restrict a_pos, B b, ResultType * __restrict c_pos, size_t size)
{
    const A * a_end = a_pos + size;
    A divider = static_cast<A>(b);
#if defined(__SSE2__)
    static constexpr size_t values_per_simd_register = REG_SIZE / sizeof(A);
    const A * a_end_simd = a_pos + size / values_per_simd_register * values_per_simd_register;

    while (a_pos < a_end_simd)
    {
#if defined(__SSE2__)
        _mm_storeu_si128(reinterpret_cast<__m128i *>(c_pos),
        _mm_loadu_si128(reinterpret_cast<const __m128i *>(a_pos)) / divider);
#endif

        a_pos += values_per_simd_register;
        c_pos += values_per_simd_register;
    }
#endif
    while (a_pos < a_end)
    {
        *c_pos = *a_pos / divider;
        ++a_pos;
        ++c_pos;
    }
}
template void divideImpl<uint64_t, uint64_t, uint64_t>(const uint64_t * __restrict, uint64_t, uint64_t * __restrict, size_t);
template void divideImpl<uint64_t, uint32_t, uint64_t>(const uint64_t * __restrict, uint32_t, uint64_t * __restrict, size_t);
template void divideImpl<uint64_t, uint16_t, uint64_t>(const uint64_t * __restrict, uint16_t, uint64_t * __restrict, size_t);
template void divideImpl<uint64_t, char8_t, uint64_t>(const uint64_t * __restrict, char8_t, uint64_t * __restrict, size_t);

template void divideImpl<uint32_t, uint64_t, uint32_t>(const uint32_t * __restrict, uint64_t, uint32_t * __restrict, size_t);
template void divideImpl<uint32_t, uint32_t, uint32_t>(const uint32_t * __restrict, uint32_t, uint32_t * __restrict, size_t);
template void divideImpl<uint32_t, uint16_t, uint32_t>(const uint32_t * __restrict, uint16_t, uint32_t * __restrict, size_t);
template void divideImpl<uint32_t, char8_t, uint32_t>(const uint32_t * __restrict, char8_t, uint32_t * __restrict, size_t);

template void divideImpl<int64_t, int64_t, int64_t>(const int64_t * __restrict, int64_t, int64_t * __restrict, size_t);
template void divideImpl<int64_t, int32_t, int64_t>(const int64_t * __restrict, int32_t, int64_t * __restrict, size_t);
template void divideImpl<int64_t, int16_t, int64_t>(const int64_t * __restrict, int16_t, int64_t * __restrict, size_t);
template void divideImpl<int64_t, int8_t, int64_t>(const int64_t * __restrict, int8_t, int64_t * __restrict, size_t);

template void divideImpl<int32_t, int64_t, int32_t>(const int32_t * __restrict, int64_t, int32_t * __restrict, size_t);
template void divideImpl<int32_t, int32_t, int32_t>(const int32_t * __restrict, int32_t, int32_t * __restrict, size_t);
template void divideImpl<int32_t, int16_t, int32_t>(const int32_t * __restrict, int16_t, int32_t * __restrict, size_t);
template void divideImpl<int32_t, int8_t, int32_t>(const int32_t * __restrict, int8_t, int32_t * __restrict, size_t);



// Function to get the subset relationship between set B and set A.
template <typename T>
struct NumericArraySlice
{
    const T * data;
    size_t size;
};
inline ALWAYS_INLINE bool hasNull(const UInt8 * null_map, size_t null_map_size)
{
    if (null_map == nullptr)
        return false;

    for (size_t i = 0; i < null_map_size; ++i)
    {
        if (null_map[i])
            return true;
    }

    return false;
}

template<class T>
inline ALWAYS_INLINE bool hasAllIntegralLoopRemainder(
    size_t j, const NumericArraySlice<T> & first, const NumericArraySlice<T> & second, const UInt8 * first_null_map, const UInt8 * second_null_map)
{
    const bool has_first_null_map = first_null_map != nullptr;
    const bool has_second_null_map = second_null_map != nullptr;

    for (; j < second.size; ++j)
    {
        // skip null elements since both have at least one - assuming it was checked earlier that at least one element in 'first' is null
        if (has_second_null_map && second_null_map[j])
            continue;

        bool found = false;

        for (size_t i = 0; i < first.size; ++i)
        {
            if (has_first_null_map && first_null_map[i])
                continue;

            if (first.data[i] == second.data[j])
            {
                found = true;
                break;
            }
        }

        if (!found)
            return false;
    }
    return true;
}

// Function to Assess the subset relationship between set B and set A.
// SSE4.2 Int64, UInt64 specialization
template<typename IntType>
inline ALWAYS_INLINE bool sliceHasImplAnyAllImplInt64(
    const NumericArraySlice<IntType> & first,
    const NumericArraySlice<IntType> & second,
    const UInt8 * first_null_map,
    const UInt8 * second_null_map)
{
    if (second.size == 0)
        return true;

    // 这种情况集合A为空,B不为空，因此B不可能是A子集
    if (!hasNull(first_null_map, first.size) && hasNull(second_null_map, second.size))
        return false;

    const bool has_first_null_map = first_null_map != nullptr;
    const bool has_second_null_map = second_null_map  != nullptr;

    size_t j = 0;
    int has_mask = 1;
    static constexpr Int64 full = -1, none = 0;
    const __m128i zeros = _mm_setzero_si128();
    if (second.size > 1 && first.size > 1)
    {
        for (; j < second.size - 1 && has_mask; j += 2)
        {
            has_mask = 0;
            // second集合数据
            const __m128i second_data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(second.data + j));
            // // second集合的null标记为bitmask，为空的时候记为-1，bitmask实际存储全1，不为空则为0。如果全空则bitmask=1；
            __m128i bitmask = has_second_null_map ?
                _mm_set_epi64x(
                    (second_null_map[j + 1]) ? full : none,
                    (second_null_map[j]) ? full : none)
                : zeros;

            size_t i = 0;

            //_mm_test_all_ones(bitmask)，若bitmask为1则返回1，否则返回0
            for (; i < first.size - 1 && !has_mask; has_mask = _mm_test_all_ones(bitmask), i += 2)
            {
                const __m128i first_data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(first.data + i));
                const __m128i first_nm_mask = has_first_null_map ?
                    _mm_cvtepi8_epi64(_mm_loadu_si128(reinterpret_cast<const __m128i *>(first_null_map + i)))
                    : zeros;

                bitmask =
                    _mm_or_si128(
                            _mm_or_si128(
                                //_mm_andnot_si128(a,b):对128位整数数据a执行按位取反操作，然后将结果与另一个128位整数b执行按位逻辑与操作
                                _mm_andnot_si128(
                                    first_nm_mask,
                                    _mm_cmpeq_epi64(second_data, first_data)), //以64bit为单位比较第二个切片second_data和第一个切片first_data中的元素是否相等。
                                _mm_andnot_si128(
                                    // _MM_SHUFFLE(1,0,3,2)将整数转换成一个八进制的控制码，这里1 0 3 2->01001110
                                    // 通过_mm_shuffle_epi32和_MM_SHUFFLE对first_nm_mask进行重排，即按照1 0 3 2的顺序从first_nm_mask中抽出数据形成新的向量
                                    _mm_shuffle_epi32(first_nm_mask, _MM_SHUFFLE(1,0,3,2)),
                                    _mm_cmpeq_epi64(second_data, _mm_shuffle_epi32(first_data, _MM_SHUFFLE(1,0,3,2))))),
                        bitmask);
            }

            if (i < first.size)
            {
                for (; i < first.size && !has_mask; ++i)
                {
                    if (has_first_null_map && first_null_map[i])
                        continue;

                    __m128i v_i = _mm_set1_epi64x(first.data[i]);
                    bitmask = _mm_or_si128(bitmask, _mm_cmpeq_epi64(second_data, v_i));
                    has_mask = _mm_test_all_ones(bitmask);
                }
            }
        }
    }

    if (!has_mask && second.size > 1)
        return false;

    return hasAllIntegralLoopRemainder(j, first, second, first_null_map, second_null_map);
}

// Function to Assess the subset relationship between set B and set A.
// SSE4.2 Int32, UInt32 specialization
template<typename IntType>
inline ALWAYS_INLINE bool sliceHasImplAnyAllImplInt32(
    const NumericArraySlice<IntType> & first,
    const NumericArraySlice<IntType> & second,
    const UInt8 * first_null_map,
    const UInt8 * second_null_map)
{
    if (second.size == 0)
        return true;

    if (!hasNull(first_null_map, first.size) && hasNull(second_null_map, second.size))
        return false;

    const bool has_first_null_map = first_null_map != nullptr;
    const bool has_second_null_map = second_null_map  != nullptr;

    size_t j = 0;
    int has_mask = 1;
    static constexpr int full = -1, none = 0;
    const __m128i zeros = _mm_setzero_si128();
    if (second.size > 3 && first.size > 3)
    {
        for (; j < second.size - 3 && has_mask; j += 4)
        {
            has_mask = 0;
            const __m128i second_data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(second.data + j));
            __m128i bitmask = has_second_null_map ?
                _mm_set_epi32(
                    (second_null_map[j + 3]) ? full : none,
                    (second_null_map[j + 2]) ? full : none,
                    (second_null_map[j + 1]) ? full : none,
                    (second_null_map[j]) ? full : none)
                : zeros;

            size_t i = 0;
            for (; i < first.size - 3 && !has_mask; has_mask = _mm_test_all_ones(bitmask), i += 4)
            {
                const __m128i first_data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(first.data + i));
                const __m128i first_nm_mask = has_first_null_map ?
                    _mm_cvtepi8_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i *>(first_null_map + i)))
                    : zeros;

                bitmask =
                    _mm_or_si128(
                        _mm_or_si128(
                            _mm_or_si128(
                                _mm_andnot_si128(
                                        first_nm_mask,
                                        _mm_cmpeq_epi32(second_data, first_data)),
                                _mm_andnot_si128(
                                    _mm_shuffle_epi32(first_nm_mask, _MM_SHUFFLE(2,1,0,3)),
                                    _mm_cmpeq_epi32(second_data, _mm_shuffle_epi32(first_data, _MM_SHUFFLE(2,1,0,3))))),
                            _mm_or_si128(
                                _mm_andnot_si128(
                                    _mm_shuffle_epi32(first_nm_mask, _MM_SHUFFLE(1,0,3,2)),
                                    _mm_cmpeq_epi32(second_data, _mm_shuffle_epi32(first_data, _MM_SHUFFLE(1,0,3,2)))),
                                _mm_andnot_si128(
                                    _mm_shuffle_epi32(first_nm_mask, _MM_SHUFFLE(0,3,2,1)),
                                    _mm_cmpeq_epi32(second_data, _mm_shuffle_epi32(first_data, _MM_SHUFFLE(0,3,2,1)))))
                        ),
                        bitmask);
            }

            if (i < first.size)
            {
                for (; i < first.size && !has_mask; ++i)
                {
                    if (has_first_null_map && first_null_map[i])
                        continue;
                    __m128i r_i = _mm_set1_epi32(first.data[i]);
                    bitmask = _mm_or_si128(bitmask, _mm_cmpeq_epi32(second_data, r_i));
                    has_mask = _mm_test_all_ones(bitmask);
                }
            }
        }
    }

    if (!has_mask && second.size > 3)
        return false;

    return hasAllIntegralLoopRemainder(j, first, second, first_null_map, second_null_map);
}

// Function to Assess the subset relationship between set B and set A.
// SSE4.2 Int16, UInt16 specialization
template<typename IntType>
inline ALWAYS_INLINE bool sliceHasImplAnyAllImplInt16(
    const NumericArraySlice<IntType> & first,
    const NumericArraySlice<IntType> & second,
    const UInt8 * first_null_map,
    const UInt8 * second_null_map)
{
    if (second.size == 0)
        return true;

    if (!hasNull(first_null_map, first.size) && hasNull(second_null_map, second.size))
        return false;

    const bool has_first_null_map = first_null_map != nullptr;
    const bool has_second_null_map = second_null_map  != nullptr;

    size_t j = 0;
    int has_mask = 1;
    static constexpr int16_t full = -1, none = 0;
    const __m128i zeros = _mm_setzero_si128();
    if (second.size > 6 && first.size > 6)
    {
        for (; j < second.size - 7 && has_mask; j += 8)
        {
            has_mask = 0;
            const __m128i second_data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(second.data + j));
            __m128i bitmask = has_second_null_map ?
                _mm_set_epi16(
                    (second_null_map[j + 7]) ? full : none, (second_null_map[j + 6]) ? full : none,
                    (second_null_map[j + 5]) ? full : none, (second_null_map[j + 4]) ? full : none,
                    (second_null_map[j + 3]) ? full : none, (second_null_map[j + 2]) ? full : none,
                    (second_null_map[j + 1]) ? full : none, (second_null_map[j]) ? full: none)
                : zeros;

            size_t i = 0;
            for (; i < first.size-7 && !has_mask; has_mask = _mm_test_all_ones(bitmask), i += 8)
            {
                const __m128i first_data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(first.data + i));
                const __m128i first_nm_mask = has_first_null_map ?
                    _mm_cvtepi8_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i *>(first_null_map + i)))
                    : zeros;
                bitmask =
                    _mm_or_si128(
                            _mm_or_si128(
                                _mm_or_si128(
                                    _mm_or_si128(
                                        _mm_andnot_si128(
                                            first_nm_mask,
                                            _mm_cmpeq_epi16(second_data, first_data)),
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(13,12,11,10,9,8,7,6,5,4,3,2,1,0,15,14)),
                                            _mm_cmpeq_epi16(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(13,12,11,10,9,8,7,6,5,4,3,2,1,0,15,14))))),
                                    _mm_or_si128(
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(11,10,9,8,7,6,5,4,3,2,1,0,15,14,13,12)),
                                            _mm_cmpeq_epi16(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(11,10,9,8,7,6,5,4,3,2,1,0,15,14,13,12)))),
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(9,8,7,6,5,4,3,2,1,0,15,14,13,12,11,10)),
                                            _mm_cmpeq_epi16(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(9,8,7,6,5,4,3,2,1,0,15,14,13,12,11,10)))))
                                ),
                                _mm_or_si128(
                                    _mm_or_si128(
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8)),
                                            _mm_cmpeq_epi16(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8)))),
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(5,4,3,2,1,0,15,14,13,12,11,10,9,8,7,6)),
                                        _mm_cmpeq_epi16(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(5,4,3,2,1,0,15,14,13,12,11,10,9,8,7,6))))),
                                    _mm_or_si128(
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(3,2,1,0,15,14,13,12,11,10,9,8,7,6,5,4)),
                                            _mm_cmpeq_epi16(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(3,2,1,0,15,14,13,12,11,10,9,8,7,6,5,4)))),
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(1,0,15,14,13,12,11,10,9,8,7,6,5,4,3,2)),
                                            _mm_cmpeq_epi16(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(1,0,15,14,13,12,11,10,9,8,7,6,5,4,3,2))))))
                        ),
                        bitmask);
            }

            if (i < first.size)
            {
                for (; i < first.size && !has_mask; ++i)
                {
                    if (has_first_null_map && first_null_map[i])
                        continue;
                    __m128i v_i = _mm_set1_epi16(first.data[i]);
                    bitmask = _mm_or_si128(bitmask, _mm_cmpeq_epi16(second_data, v_i));
                    has_mask = _mm_test_all_ones(bitmask);
                }
            }
        }
    }

    if (!has_mask && second.size > 6)
        return false;

    return hasAllIntegralLoopRemainder(j, first, second, first_null_map, second_null_map);
}

// Function to Assess the subset relationship between set B and set A.
// SSE2 Int8, UInt8 specialization
template<typename IntType>
inline ALWAYS_INLINE bool sliceHasImplAnyAllImplInt8(
    const NumericArraySlice<IntType> & first,
    const NumericArraySlice<IntType> & second,
    const UInt8 * first_null_map,
    const UInt8 * second_null_map)
{
    if (second.size == 0)
        return true;

    if (!hasNull(first_null_map, first.size) && hasNull(second_null_map, second.size))
        return false;

    const bool has_first_null_map = first_null_map != nullptr;
    const bool has_second_null_map = second_null_map != nullptr;

    size_t j = 0;
    int has_mask = 1;
    static constexpr int8_t full = -1, none = 0;
    const __m128i zeros = _mm_setzero_si128();

    if (second.size > 15 && first.size > 15)
    {
        for (; j < second.size - 15 && has_mask; j += 16)
        {
            has_mask = 0;
            const __m128i second_data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(second.data + j));
            __m128i bitmask = has_second_null_map ?
                _mm_set_epi8(
                    (second_null_map[j + 15]) ? full : none, (second_null_map[j + 14]) ? full : none,
                    (second_null_map[j + 13]) ? full : none, (second_null_map[j + 12]) ? full : none,
                    (second_null_map[j + 11]) ? full : none, (second_null_map[j + 10]) ? full : none,
                    (second_null_map[j + 9]) ? full : none, (second_null_map[j + 8]) ? full : none,
                    (second_null_map[j + 7]) ? full : none, (second_null_map[j + 6]) ? full : none,
                    (second_null_map[j + 5]) ? full : none, (second_null_map[j + 4]) ? full : none,
                    (second_null_map[j + 3]) ? full : none, (second_null_map[j + 2]) ? full : none,
                    (second_null_map[j + 1]) ? full : none, (second_null_map[j]) ? full : none)
                : zeros;

            size_t i = 0;
            for (; i < first.size - 15 && !has_mask; has_mask = _mm_test_all_ones(bitmask), i += 16)
            {
                const __m128i first_data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(first.data + i));
                const __m128i first_nm_mask = has_first_null_map ?
                    _mm_loadu_si128(reinterpret_cast<const __m128i *>(first_null_map + i))
                    : zeros;
                bitmask =
                    _mm_or_si128(
                        _mm_or_si128(
                            _mm_or_si128(
                                _mm_or_si128(
                                    _mm_or_si128(
                                        _mm_andnot_si128(
                                            first_nm_mask,
                                            _mm_cmpeq_epi8(second_data, first_data)),
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,15)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,15))))),
                                    _mm_or_si128(
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(13,12,11,10,9,8,7,6,5,4,3,2,1,0,15,14)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(13,12,11,10,9,8,7,6,5,4,3,2,1,0,15,14)))),
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(12,11,10,9,8,7,6,5,4,3,2,1,0,15,14,13)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(12,11,10,9,8,7,6,5,4,3,2,1,0,15,14,13)))))
                                ),
                                _mm_or_si128(
                                    _mm_or_si128(
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(11,10,9,8,7,6,5,4,3,2,1,0,15,14,13,12)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(11,10,9,8,7,6,5,4,3,2,1,0,15,14,13,12)))),
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(10,9,8,7,6,5,4,3,2,1,0,15,14,13,12,11)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(10,9,8,7,6,5,4,3,2,1,0,15,14,13,12,11))))),
                                    _mm_or_si128(
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(9,8,7,6,5,4,3,2,1,0,15,14,13,12,11,10)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(9,8,7,6,5,4,3,2,1,0,15,14,13,12,11,10)))),
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(8,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(8,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9))))))),
                            _mm_or_si128(
                                _mm_or_si128(
                                    _mm_or_si128(
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8)))),
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,7)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,7))))),
                                    _mm_or_si128(
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(5,4,3,2,1,0,15,14,13,12,11,10,9,8,7,6)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(5,4,3,2,1,0,15,14,13,12,11,10,9,8,7,6)))),
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(4,3,2,1,0,15,14,13,12,11,10,9,8,7,6,5)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(4,3,2,1,0,15,14,13,12,11,10,9,8,7,6,5)))))),
                                _mm_or_si128(
                                    _mm_or_si128(
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(3,2,1,0,15,14,13,12,11,10,9,8,7,6,5,4)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(3,2,1,0,15,14,13,12,11,10,9,8,7,6,5,4)))),
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(2,1,0,15,14,13,12,11,10,9,8,7,6,5,4,3)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(2,1,0,15,14,13,12,11,10,9,8,7,6,5,4,3))))),
                                    _mm_or_si128(
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(1,0,15,14,13,12,11,10,9,8,7,6,5,4,3,2)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(1,0,15,14,13,12,11,10,9,8,7,6,5,4,3,2)))),
                                        _mm_andnot_si128(
                                            _mm_shuffle_epi8(first_nm_mask, _mm_set_epi8(0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)),
                                            _mm_cmpeq_epi8(second_data, _mm_shuffle_epi8(first_data, _mm_set_epi8(0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)))))))),
                        bitmask);
            }

            if (i < first.size)
            {
                for (; i < first.size && !has_mask; ++i)
                {
                    if (has_first_null_map && first_null_map[i])
                        continue;

                    __m128i v_i = _mm_set1_epi8(first.data[i]);
                    bitmask = _mm_or_si128(bitmask, _mm_cmpeq_epi8(second_data, v_i));
                    has_mask = _mm_test_all_ones(bitmask);
                }
            }
        }
    }
    if (!has_mask && second.size > 15)
        return false;
    return hasAllIntegralLoopRemainder(j, first, second, first_null_map, second_null_map);
}

// Function to count the nums of char(one bytes) in arrary a
#ifdef __SSE4_2__
template <typename Char>
inline int count_char(const Char* a, size_t a_size, const uint8_t b) {
    size_t offset = 0;
    uint8_t total = 0;
    __m128i character_vec = _mm_set1_epi8(b);
    for (; offset + 16 <= a_size; offset += 16) {
        uint16_t mask = _mm_movemask_epi8(
                _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a + offset)),character_vec));
        if (mask) {
            total += _mm_popcnt_u32(mask);
        }
    }
    for(;offset<a_size;++offset){
        total += a[offset] == b ? 1 : 0; 
    }
    return total;
}
#else
template <typename Char>
inline int count_char(const Char* a, size_t a_size, const uint8_t b) {
    uint8_t total = 0;
    for (size_t offset=0; offset < a_size; ++offset){
        if(a[offset] == b){
            ++total;
        }
    }
    // size_t offset = 0;
    // while(offset < a_size){
    //     total += a[offset] == b ? 1 : 0; 
    //     offset++;
    // }
    return total;
}
#endif

#ifdef __SSE4_2__
template <typename Char>
inline int count_uint16_t(const Char* a, size_t a_size, const Char b) {
    size_t offset = 0;
    uint8_t total = 0;
    __m128i Int_vec = _mm_set1_epi16(b);
    for (; offset + 8 <= a_size; offset += 8) {
        uint16_t mask = _mm_movemask_epi8(
                _mm_cmpeq_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a + offset)),Int_vec));
        if (mask) {
            total += _mm_popcnt_u32(mask)/2;
        }
    }
    for(;offset<a_size;++offset){
        total += a[offset] == b ? 1 : 0; 
    }
    return total;
}
#else
template <typename Char>
inline int count_uint16_t(const Char* a, size_t a_size, const Char b) {
    uint8_t total = 0;
    for (size_t offset=0; offset < a_size; ++offset){
        if(a[offset] == b){
            ++total;
        }
    }
    return total;
}
#endif

#ifdef __SSE4_2__
template <typename Char>
inline int count_uint32_t(const Char* a, size_t a_size, const Char b) {
    size_t offset = 0;
    uint8_t total = 0;
    __m128i Int_vec = _mm_set1_epi32(b);
    for (; offset + 4 <= a_size; offset += 4) {
        uint32_t mask = _mm_movemask_epi8(
                _mm_cmpeq_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a + offset)),Int_vec));
        if (mask) {
            total += _mm_popcnt_u32(mask)/4;
        }
    }
    for(;offset<a_size;++offset){
        total += a[offset] == b ? 1 : 0; 
    }
    return total;
}
#else
template <typename Char>
inline int count_uint32_t(const Char* a, size_t a_size, const Char b) {
    uint8_t total = 0;
    for (size_t offset=0; offset < a_size; ++offset){
        if(a[offset] == b){
            ++total;
        }
    }
    return total;
}
#endif

#ifdef __SSE4_2__
template <typename Char>
inline int count_uint64_t(const Char* a, size_t a_size, const Char b) {
    size_t offset = 0;
    uint8_t total = 0;
    __m128i Int_vec = _mm_set1_epi64x(b);
    for (; offset + 2 <= a_size; offset += 2) {
        uint32_t mask = _mm_movemask_epi8(
                _mm_cmpeq_epi64(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a + offset)),Int_vec));
        if (mask) {
            total += _mm_popcnt_u32(mask)/8;
        }
    }
    for(;offset<a_size;++offset){
        total += a[offset] == b ? 1 : 0; 
    }
    return total;
}
#else
template <typename Char>
inline int count_uint64_t(const Char* a, size_t a_size, const Char b) {
    uint8_t total = 0;
    for (size_t offset=0; offset < a_size; ++offset){
        if(a[offset] == b){
            ++total;
        }
    }
    return total;
}
#endif

template<typename T>
size_t search_uint16(T* a, T b , const size_t size){
size_t offset=0;
#ifdef __SSE4_1__
    __m128i target_vec =  _mm_set1_epi16(b);
    for (; offset < size; offset += 4){
        __m128i data_vec = _mm_loadu_si128(reinterpret_cast<const __m128i *>(a+offset));
        __m128i cmp = _mm_cmpeq_epi16(data_vec, target_vec);
        int mask = _mm_movemask_epi8(cmp);
        if (mask) {
            offset += __builtin_ctz(mask) / 2;
            return offset+1;
        }  
    }
#endif
    for(; offset < size ; offset++){
        if (a[offset] == b) return offset + 1;
    }
return offset + 1;

}


template<typename T>
size_t search_uint32(T* a, T b , const size_t size){
size_t offset=0;
#ifdef __SSE4_1__
    __m128i target_vec =  _mm_set1_epi32(b);
    for (; offset < size; offset += 4){
        __m128i data_vec = _mm_loadu_si128(reinterpret_cast<const __m128i *>(a+offset));
        __m128i cmp = _mm_cmpeq_epi32(data_vec, target_vec);
        int mask = _mm_movemask_epi8(cmp);
        if (mask) {
            offset += __builtin_ctz(mask) / 4;
            return offset+1;
        }  
    }
#endif
    for(; offset < size ; offset++){
        if (a[offset] == b) return offset + 1;
    }
return offset + 1;

}


template<typename T>
size_t search_uint64(T* a, T b , const size_t size){
size_t offset=0;
#ifdef __SSE4_1__
    __m128i target_vec =  _mm_set1_epi64x(b);
    for (; offset < size; offset += 2){
        __m128i data_vec = _mm_loadu_si128(reinterpret_cast<const __m128i *>(a+offset));
        __m128i cmp = _mm_cmpeq_epi64(data_vec, target_vec);
        int mask = _mm_movemask_epi8(cmp);
        if (mask) 
        {
            offset += __builtin_ctz(mask) / 8;
            return offset+1;
        }  
    }
#endif
    for(; offset < size ; offset++){
        if (a[offset] == b) return offset + 1;
    }
return offset + 1;

}

}// NBSimdBooster
