// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <ctype.h>
#include <string>
#include <string_view>
#include <vector>

namespace loquat
{

    bool atoi(std::string_view str, int*);
    bool atoi(std::string_view str, int64_t*);
    bool atof(std::string_view str, float*);
    bool atof(std::string_view str, double*);

    std::vector<std::string> split_strings_from_whitespace(std::string_view str);

    std::vector<std::string> split_string(std::string_view str, char ch);
    std::vector<int> split_string_to_ints(std::string_view str, char ch);
    std::vector<int64_t> split_string_to_int64s(std::string_view str, char ch);
    std::vector<Float> split_string_to_floats(std::string_view str, char ch);
    std::vector<double> split_string_to_doubles(std::string_view str, char ch);

    std::string UTF8_from_UTF16(std::u16string str);
    std::u16string UTF16_from_UTF8(std::string str);

#ifdef LOQUAT_IS_WINDOWS
    std::wstring WString_from_UTF8(std::string str);
    std::string UTF8_from_WString(std::wstring str);
#endif

    std::string normalize_UTF8(std::string str);

    class InternedString
    {
    public:
        InternedString() = default;
        InternedString(const std::string* str)
            : str(str)
        {}
        operator const std::string& () const
        {
            return *str;
        }

        bool operator==(const char* s) const
        {
            return *str == s;
        }

        bool operator==(const std::string& s) const
        {
            return *str == s;
        }

        bool operator!=(const char* s) const
        {
            return *str != s;
        }

        bool operator!=(const std::string& s) const
        {
            return *str != s;
        }

        bool operator<(const char* s) const
        {
            return *str < s;
        }

        bool operator<(const std::string& s) const
        {
            return *str < s;
        }

        [[nodiscard]]
        std::string to_string() const
        {
            return *str;
        }

    private:
        const std::string* str = nullptr;
    };

    struct InternedStringHash
    {
        size_t operator()(const InternedString& s) const
        {
            return std::hash<std::string>()(s);
        }
    };

}