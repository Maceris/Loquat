// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "pbr/util/tagged_pointer.h"

namespace loquat
{
	class HomogeneousMedium;
	class GridMedium;
	class RGBGridMedium;
	class CloudMedium;
	class NanoVDBMedium;

	class Medium : public TaggedPointer<HomogeneousMedium, GridMedium, 
		RGBGridMedium, CloudMedium, NanoVDBMedium>
	{
	public:
		using TaggedPointer::TaggedPointer;

		std::string to_string() const;
		bool is_emissive() const;
	};

	class MediumInterface
	{
	public:
		Medium inside;
		Medium outside;
	};
}