// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <string>
#include <string_view>

#include "main/loquat.h"

#include "debug/logger.h"

namespace loquat
{
    struct FileLoc
    {
        FileLoc() = default;
        FileLoc(std::string_view filename)
            : filename(filename)
        {}

        std::string to_string() const;

        std::string_view filename;
        int line = 1;
        int column = 0;
    };

    void suppress_error_messages();

    int last_error();
    std::string error_string(int errorId = last_error());
}