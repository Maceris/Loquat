// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <string>
#include <vector>

#include "main/loquat.h"

#include "pbr/util/pstd.h"

namespace loquat
{
	std::string read_file_contents(std::string filename);
	std::string read_decompressed_file_contents(std::string filename);
	bool write_file_contents(std::string filename, const std::string& contents);

	std::vector<Float> read_float_file(std::string filename);

	bool file_exists(std::string filename);
	bool remove_file(std::string filename);

	std::string resolve_filename(std::string filename);
	void set_search_directory(std::string filename);

	bool has_extension(std::string filename, std::string ext);
	std::string remove_extension(std::string filename);

	std::vector<std::string> matching_filenames(std::string filename);

	FILE* fopen_read(std::string filename);
	FILE* fopen_write(std::string filename);
}
