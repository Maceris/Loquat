// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/util/tagged_pointer.h"

namespace loquat
{
	
	struct PhaseFunctionSample
	{
		Float probability;
		Vec3f incoming;
		Float pdf;
	};

	class HGPhaseFunction;

	class PhaseFunction : TaggedPointer<HGPhaseFunction>
	{
	public:
		using TaggedPointer::TaggedPointer;

		[[nodiscard]]
		std::string to_string() const noexcept;

		[[nodiscard]]
		inline Float phase(Vec3f outgoing, Vec3f incoming) const noexcept;

		[[nodiscard]]
		inline std::optional<PhaseFunctionSample> sample_phase(Vec3f outgoing,
			Point2f sample_2D) const noexcept;

		[[nodiscard]]
		inline Float PDF(Vec3f outgoing, Vec3f incoming) const noexcept;
	};


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
		[[nodiscard]]
		std::string to_string() const noexcept;

		MediumInterface() = default;

		MediumInterface(Medium medium)
			: inside{ medium }
			, outside{ medium }
		{}

		MediumInterface(Medium inside, Medium outside)
			: inside{ inside }
			, outside{ outside }
		{}

		[[nodiscard]]
		bool is_medium_transition() const noexcept
		{
			return inside != outside;
		}

		Medium inside;
		Medium outside;
	};
}