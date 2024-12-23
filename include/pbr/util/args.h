// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <cctype>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include "main/loquat.h"

#include "pbr/util/pstd.h"
#include "pbr/util/string.h"

namespace loquat
{

    // Downcase the string and remove any '-' or '_' characters; thus we can be
    // a little flexible in what we match for argument names.
    inline std::string normalize_arg(const std::string& str)
    {
        std::string result;
        for (unsigned char c : str)
        {
            if (c != '_' && c != '-')
            {
                result += std::tolower(c);
            }
        }
        return result;
    }

    inline bool init_arg(const std::string& str, int* ptr)
    {
        if (str.empty() || (!std::isdigit(str[0]) && str[0] != '-'))
        {
            return false;
        }
        try
        {
            *ptr = std::stoi(str);
        }
        catch (const std::invalid_argument&)
        {
            return false;
        }
        catch (const std::out_of_range&)
        {
            return false;
        }
        return true;
    }

    inline bool init_arg(const std::string& str, float* ptr)
    {
        if (str.empty())
        {
            return false;
        }
        try
        {
            *ptr = std::stof(str);
        }
        catch (const std::invalid_argument&)
        {
            return false;
        }
        catch (const std::out_of_range&)
        {
            return false;
        }
        return true;
    }

    inline bool init_arg(const std::string& str, double* ptr)
    {
        if (str.empty())
        {
            return false;
        }
        try
        {
            *ptr = std::stod(str);
        }
        catch (const std::invalid_argument&)
        {
            return false;
        }
        catch (const std::out_of_range&)
        {
            return false;
        }
        return true;
    }

    inline bool init_arg(const std::string& str, pstd::span<float> out)
    {
        std::vector<Float> v = split_string_to_floats(str, ',');
        if (v.size() != out.size())
        {
            return false;
        }
        std::copy(v.begin(), v.end(), out.begin());
        return true;
    }

    inline bool init_arg(const std::string& str, pstd::span<double> out)
    {
        std::vector<double> v = split_string_to_doubles(str, ',');
        if (v.size() != out.size())
        {
            return false;
        }
        std::copy(v.begin(), v.end(), out.begin());
        return true;
    }

    inline bool init_arg(const std::string& str, pstd::span<int> out)
    {
        std::vector<int> v = split_string_to_ints(str, ',');
        if (v.size() != out.size())
        {
            return false;
        }
        std::copy(v.begin(), v.end(), out.begin());
        return true;
    }

    inline bool init_arg(const std::string& str, char** ptr)
    {
        if (str.empty())
        {
            return false;
        }
        *ptr = new char[str.size() + 1];
        std::strcpy(*ptr, str.c_str());
        return true;
    }

    inline bool init_arg(const std::string& str, std::string* ptr)
    {
        if (str.empty())
        {
            return false;
        }
        *ptr = str;
        return true;
    }

    inline bool init_arg(const std::string& str, bool* ptr)
    {
        if (normalize_arg(str) == "false")
        {
            *ptr = false;
            return true;
        }
        else if (normalize_arg(str) == "true")
        {
            *ptr = true;
            return true;
        }
        return false;
    }

    template <typename T>
    bool init_arg(const std::string& str, pstd::optional<T>* ptr)
    {
        T value;
        if (init_arg(str, &value))
        {
            *ptr = value;
            return true;
        }
        return false;
    }

    inline bool match_prefix(const std::string& str, const std::string& prefix)
    {
        if (prefix.size() > str.size())
        {
            return false;
        }
        for (size_t i = 0; i < prefix.size(); ++i)
        {
            if (prefix[i] != str[i])
            {
                return false;
            }
        }
        return true;
    }

    template <typename T>
    bool enable(T ptr)
    {
        return false;
    }

    inline bool enable(bool* ptr)
    {
        *ptr = true;
        return true;
    }

    // T basically needs to be a pointer type or a Span.
    template <typename Iter, typename T>
    bool parse_arg(Iter* iter, Iter end, const std::string& name, T out,
        std::function<void(std::string)> onError)
    {
        std::string arg = **iter;

        // Strip either one or two leading dashes.
        if (arg[1] == '-')
        {
            arg = arg.substr(2);
        }
        else
        {
            arg = arg.substr(1);
        }

        if (match_prefix(normalize_arg(arg), normalize_arg(name + '=')))
        {
            // --arg=value
            std::string value = arg.substr(name.size() + 1);
            if (!init_arg(value, out))
            {
                onError(StringPrintf("invalid value \"%s\" for --%s argument",
                    value, name));
                return false;
            }
            return true;
        }
        else if (normalize_arg(arg) == normalize_arg(name))
        {
            // --arg <value>, except for bool arguments, which are set to true
            // without expecting another argument.
            if (enable(out))
                return true;

            ++(*iter);
            if (*iter == end)
            {
                onError(StringPrintf("missing value after --%s argument", arg));
                return false;
            }
            if (!init_arg(**iter, out))
            {
                onError(StringPrintf("invalid value \"%s\" for --%s argument",
                    **iter, name));
                return false;
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    std::vector<std::string> get_command_line_arguments(char* argv[]);

}